# CMake Superbuild: Design and Implementation Notes

This page documents the design of Cardinal's [experimental CMake build](with_cmake.md): why it's
built the way it is, and the non-obvious problems its implementation had to work around. It's aimed
at anyone maintaining or extending `CMakeLists.txt`/`cmake/*.cmake`, not at someone just trying to
build Cardinal (see [with_cmake.md](with_cmake.md) for that).

## Goal and non-goals

The goal is to support the standard out-of-source CMake workflow
(`cmake -S . -B build && cmake --build build`, `ccmake build`) as an alternative to the Makefile
workflow in [without_conda.md](without_conda.md), both against a pre-built dependency container and
building PETSc/libMesh/WASP from source on a bare HPC host.

Explicit non-goals:

- +Not replacing or depending on MOOSE's own (separately in-progress) CMake port.+
- +Not replacing Cardinal's own Make-based app compile+ (`framework/build.mk`, `app.mk`, etc) --
  that stays exactly as it is today, including its own `config/*.mk` files. The CMake project only
  ever invokes MOOSE's/the dependencies' own existing build scripts unmodified, and finishes by
  handing off to Cardinal's own `Makefile`, unpatched.

## Scope

Per `Makefile` and `config/check_deps.mk`, the dependencies that run their own CMake sub-build today
(independent of MOOSE's framework compile) are NekRS, OpenMC, and -- DAGMC's CAD/mesh-geometry
stack -- MOAB, Embree, Double-Down, and DAGMC itself. MOOSE, `nuclear_data`, and
`test/tests/nek_ci` have no CMake sub-build of their own but are still required (unconditionally, or
gated on `ENABLE_NEK`) -- resolved the same way as the others but with a `NO_BUILD` mode: mirrored or
cloned like any other managed dependency, but never itself configured/built (Cardinal's own Makefile
compiles MOOSE directly; `nuclear_data`/`nek_ci` are just data/test-fixture trees).

SAM/BISON/Sockeye/Sodium/IAPWS95/Griffin remain entirely out of scope: none of them run their own
CMake sub-build, several require INL/ANL-internal credentials just to clone, and they're simply
excluded from the source mirror (below) like any other unmanaged `contrib/` path.

## Why a superbuild, and why the build directory doubles as the app checkout

`find_package` resolves at configure time; `ExternalProject_Add` only builds at build time -- a
single CMake configure pass can't build a dependency and then consume it in the same pass. The
standard fix is a *superbuild*: a top-level `CMakeLists.txt` whose only job is to resolve/build
dependencies via `ExternalProject_Add`, with the actual project handled separately.

Rather than staging dependencies in some separate area and then pointing the *existing* Make-based
app build at them (which would require patching `config/check_deps.mk`'s hard requirement that
`contrib/<dep>` be populated), this superbuild mirrors the +whole+ Cardinal source tree directly
into `${CMAKE_BINARY_DIR}` and lays out each dependency at exactly the path the Makefile already
expects, relative to that directory. The build directory *is* the equivalent of a Cardinal checkout
-- the result of a CMake build looks like the result of an in-tree Makefile build (`cardinal-opt`,
`lib/`, `src/`, `contrib/`, ... all directly at the top), not nested inside an extra subdirectory.

| What | Path (relative to the one build dir) | Matches |
|---|---|---|
| Whole app source | `${CMAKE_BINARY_DIR}/` (root) | a manual `rsync -a` into a scratch build tree |
| NekRS checkout | `${CMAKE_BINARY_DIR}/contrib/nekRS` | `NEKRS_DIR` |
| NekRS's own CMake build | `${CMAKE_BINARY_DIR}/build/nekrs` | `NEKRS_BUILDDIR` |
| OpenMC checkout | `${CMAKE_BINARY_DIR}/contrib/openmc` | `OPENMC_DIR` |
| OpenMC's own CMake build | `${CMAKE_BINARY_DIR}/build/openmc` | `OPENMC_BUILDDIR` |
| MOAB/Embree/double-down/DAGMC checkouts | `${CMAKE_BINARY_DIR}/contrib/{moab,embree,double-down,DAGMC}` | `{MOAB,EMBREE,DOUBLEDOWN,DAGMC}_DIR` |
| Their own CMake builds | `${CMAKE_BINARY_DIR}/build/{moab,embree,double-down,DAGMC}` | `{...}_BUILDDIR` |
| Shared install prefix | `${CMAKE_BINARY_DIR}/install` | `CONTRIB_INSTALL_DIR` |

+The one real collision:+ Cardinal's own hand-written top-level `Makefile` against CMake's own
generated `Makefile` of the same name at the build directory root. Resolved by having the source
mirror (below) exclude `/Makefile` from its normal copy and instead copy it under a different name
(`Makefile.cardinal`); the final `cardinal` target invokes that explicitly
(`$(MAKE) -f Makefile.cardinal ...`). CMake's own `CMakeCache.txt`/`CMakeFiles/`/
`cmake_install.cmake` have no other name collisions with anything in Cardinal's tree.

Because everything else lines up exactly, once `ExternalProject_Add` has configured, built, and
installed a dependency, the Makefile's own `build_<dep>` rule sees its sub-Makefile already exists
and `check_deps.mk`'s gate sees the submodule populated -- +no patch to the Makefile/`config/*.mk`
is needed.+ `make` runs unmodified as the final build step.

## `mirror_source`: what gets copied, and how

```cmake
add_custom_target(mirror_source ALL
  COMMAND rsync -rlpgo --update
          --exclude=/contrib
          --exclude=/.git
          --exclude=/Makefile
          --exclude=/build
          --exclude=/install
          --exclude=/CMakeLists.txt
          --exclude=/cmake
          ${CMAKE_SOURCE_DIR}/ ${CARDINAL_STAGE_DIR}/
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_SOURCE_DIR}/Makefile ${CARDINAL_STAGE_DIR}/Makefile.cardinal
  ...)
```

Reruns every build (`add_custom_target` has no tracked `OUTPUT`, so it's always considered out of
date).

+Why `-rlpgo --update`, not `-a`:+ `-a` is `-rlptgoD`. The `-t` (preserve source mtime) is wrong
here for two reasons:

1. Cardinal's Makefile relies on ordinary mtime comparisons to decide what needs recompiling.
   Blindly preserving a source file's own mtime (rather than stamping the destination with the
   actual copy time) means a concurrent edit to the source tree can produce a misleading timestamp.
2. Without `-t`, the destination's mtime becomes "whenever it was last copied" -- so `--update`
   (skip a file when the destination is already newer than the source, comparing timestamps only)
   is *required* alongside dropping `-t`, not optional: without it, rsync's default quick-check
   would see every file as changed on every build and re-copy the entire tree every time, forcing a
   full recompile+relink regardless of whether anything actually changed.

+Why no `--delete`:+ Cardinal builds in-place -- object files, libraries, `.d` dependency files,
and generated `*Revision.h` headers all land inline next to their sources (e.g.
`src/base/CardinalApp.o`), not confined to `/build`. None of that exists in the pristine source
tree, so `--delete` would remove it all on every single `mirror_source` run, as soon as it was
produced, forcing a full rebuild+relink every time. Rather than comprehensively enumerate every
in-tree generated pattern Cardinal's Makefile happens to produce, `mirror_source` just drops
`--delete` outright: the cost is a stale copy lingering if a source file is later removed/renamed
upstream, far cheaper than silently rebuilding the world every time.

+Why "copy everything" rather than filtering by `.gitignore`:+ `.gitignore` only hides
*untracked* files from git -- a file that's explicitly tracked despite matching a broad ignore
pattern (e.g. a fixture under `test/tests/` tracked despite the repo's blanket `*.xml` pattern,
which exists to keep *generated* XML out, not checked-in fixtures) stays tracked and visible to git
regardless, but rsync's `--filter` has no such "tracked overrides ignore" concept and would exclude
it unconditionally. Mirroring exactly what `git ls-files` tracks instead would dodge that specific
problem but would silently miss any file a user has created in the source tree but not yet `git
add`ed. Simplest and most robust: don't try to be clever about what belongs in the mirror --
just copy everything except what's excluded for concrete, functional reasons below.

- +`--exclude=/contrib`+ covers every submodule under it in one line -- the ones this build
  manages (moose/nekRS/openmc/nuclear_data/moab/embree/double-down/DAGMC), each resolved
  independently, plus the rest (SAM/bison/iapws95/potassium/sockeye/sodium) that shouldn't be
  mirrored at all even if a user happens to have them checked out locally.
- +`--exclude=/build --exclude=/install`:+ Cardinal's own native Makefile defaults both to exactly
  these names directly under the source checkout. Since the CMake build directory *is* the app
  checkout, a bare rsync of the whole source tree into it would, for anyone who has ever run (or
  runs concurrently) a native build in that same checkout, copy that leftover/live build/install
  tree wholesale on top of this build's own `ExternalProject_Add` trees living at those same
  relative paths -- silently corrupting them. Excluded unconditionally, not just when such a
  directory happens to exist.
- +`--exclude=/CMakeLists.txt --exclude=/cmake`:+ this build's own files. Cardinal's native
  Makefile has no use for either, so mirroring them would just leave a redundant, easily stale copy
  sitting next to CMake's own `CMakeCache.txt`/`CMakeFiles` for no purpose.
- +`test/tests/nek_ci`+ is a submodule *outside* `contrib/` (NekRS's own CI test fixtures),
  resolved independently the same way, but only when `ENABLE_NEK` is on -- excluded from the
  generic mirror in that case too, so the two don't fight over the same destination.

## Per-submodule three-tier resolution (`cardinal_add_submodule_dependency`)

For each managed dependency, in priority order:

1. +Pre-built install already known+ (WASP/libMesh/PETSc only -- see below; not handled by this
   function at all).
2. +Submodule already initialized in the source tree.+ Treated as the authoritative source,
   mirrored into the build tree on every build via `ExternalProject_Add`'s
   `DOWNLOAD_COMMAND`/`UPDATE_COMMAND`, delegated to `CardinalMirrorSubmodule.cmake` (plain
   `rsync -rlpgo --update`, matching `mirror_source`'s own reasoning, plus a git-metadata repair --
   see below).
3. +Not initialized anywhere.+ Cloned straight into the build tree, delegated to
   `CardinalGitFetchBySha.cmake` rather than `ExternalProject_Add`'s own built-in
   `GIT_REPOSITORY`/`GIT_TAG` support (see below). The URL comes from `.gitmodules`; the pinned
   commit comes from `git ls-tree HEAD -- <path>` on the superproject at configure time, so the pin
   has one source of truth instead of being hand-copied into CMake.

In all cases the heavy submodule content only ever lands under `${CMAKE_BINARY_DIR}`, never written
into the pristine `-S` source tree -- this is what keeps `git status`/`git diff` on the reference
checkout fast regardless of which tier fired.

`MARKER_FILE` (what decides tier 2 vs. tier 3) is `.git` for MOOSE/`nuclear_data`/`nek_ci` (all
`NO_BUILD`) rather than a specific filename -- more robust against any one file being
renamed/removed upstream, and simple to verify.

### A pinned commit isn't always reachable by `git clone`

`ExternalProject_Add`'s own `GIT_REPOSITORY`/`GIT_TAG` mechanism does a plain `git clone` --
fetching only objects reachable from an advertised branch/tag -- then `git checkout <sha>`. This
fails with "fatal: reference is not a tree" whenever the pinned commit isn't at the tip of any
upstream branch, which happens for real: `contrib/moab`'s pin is exactly such a commit on the
`fathomteam/moab` Bitbucket repo. `git submodule update --init` succeeds on that same commit because
git instead fetches the exact SHA directly (`git fetch <url> <sha>`), which the server honors even
though the commit is unreachable from any ref. `CardinalGitFetchBySha.cmake` does the same, with one
added subtlety: it sets up a real `origin` remote first (`git remote add origin <url>` then
`git fetch origin <sha>`), not a bare-URL fetch -- a bare-URL fetch leaves no `remote.origin.url`
recorded at all, which breaks *relative* submodule URLs in whatever gets cloned this way (MOOSE's
own `.gitmodules` points at libmesh with `url = ../../libMesh/libmesh`, resolved relative to the
parent repo's own `origin` URL; with no origin configured, git instead falls back to resolving it
against the local filesystem path of the current working directory).

### A submodule's `.git` is a gitlink, not a real repo -- mirroring breaks it

A submodule's `.git` is a gitlink file (`gitdir: <relative path>`) pointing at the real git
directory nested under the superproject's own `.git/modules/<path>`, entirely outside the checked
out submodule directory itself. Copying it verbatim into the build tree either:

- points at a path that doesn't exist there at all (the source mirror deliberately excludes
  `/.git`), breaking any git command later run from *inside* the mirrored copy -- e.g. MOOSE's own
  on-demand libmesh/petsc/wasp submodule fetch, which `cd`s into `contrib/moose` before running
  `git submodule update --init --recursive libmesh`; or
- if instead repointed at the *original* gitdir's real absolute path, would silently operate on the
  *original* source tree's working directory rather than the mirror -- that gitdir's own config
  sets `core.worktree` to a relative path back to the original checkout, which git honors
  regardless of where the gitlink pointing at it lives.

`CardinalMirrorSubmodule.cmake` fixes this by copying the *real* git directory's contents into
`<dest>/.git` (a real directory, not a gitlink) and stripping `core.worktree` from its config --
using `git config --file <path> --unset core.worktree` rather than `git -C <dest> config --unset`,
since the latter goes through git's own repo/worktree discovery, which is exactly what's broken at
that point. The result is a genuinely self-contained repository whose worktree is itself. This is
also what lets OpenMC's own `git submodule update --init --recursive` (for its vendored
dependencies, run unconditionally by OpenMC's own `CMakeLists.txt` whenever `.git` exists) succeed
from inside its mirrored copy.

## WASP/libMesh/PETSc: three independent toggles

Each of `WASP_DIR`/`LIBMESH_DIR`/`PETSC_DIR` is independently either a pre-built install (a
`CACHE PATH`, pre-filled from the environment) or built from source via MOOSE's own
`contrib/moose/scripts/update_and_rebuild_{petsc,libmesh,wasp}.sh` scripts, matching the
non-container Makefile workflow. They're independent except for one real constraint:

- +WASP+ has no build or link dependency on libMesh or PETSc at all -- a standalone CMake+Ninja
  tool -- so it's fully independent of the other two.
- +PETSc+ is independent too, and by far the slowest and most painful of the three to build from
  source -- exactly the one most worth being able to skip via a system/module-provided install.
- +libMesh+ has one real constraint: a pre-built `LIBMESH_DIR` was compiled against some specific
  PETSc, and there's no way to discover which one from `LIBMESH_DIR` alone -- so `LIBMESH_DIR` set
  without `PETSC_DIR` is a configure-time error. Otherwise independent.

`CARDINAL_BUILD_PETSC`/`CARDINAL_BUILD_LIBMESH`/`CARDINAL_BUILD_WASP` (each `NOT <DIR>_set`) gate
their own `cardinal_add_moose_prereq` call, `DEPENDS` entries, and whether the final `cardinal`
target depends on that dependency's own CMake target at all.

+Compiler discovery+ is a single top-level decision, independent of what else needs building:

- +If libMesh is pre-built:+ ask it directly via `${LIBMESH_DIR}/bin/libmesh-config --cc/--cxx/
  --fc` -- most authoritative, and applies to whatever else needs building too, for toolchain
  consistency. Cardinal's own `config/*.mk` files normally get this from `$(libmesh_CC)` etc,
  computed deep inside `framework/build.mk` which only runs once `make` starts -- too late for
  `ExternalProject_Add` configure steps, which need it up front.
- +Otherwise+ (building libMesh ourselves): no chicken-and-egg trick available (libMesh doesn't
  exist yet), so pick `CC`/`CXX`/`FC` directly from the environment if set, else
  `find_program(mpicc/mpicxx/mpif90)` -- matching what the Makefile workflow already relies on.

Deliberately does *not* try to derive compilers from an externally provided `PETSC_DIR`'s own build
info (e.g. reading its `petscvariables` for the exact compiler PETSc was built with), even though a
compiler/ABI mismatch between what libMesh's `configure` uses and what PETSc was actually built with
is a real risk. The user is expected to have a consistent `CC`/`CXX`/`FC` for whatever `PETSC_DIR`
they hand us, rather than this build system reverse-engineering it.

+Compiler list conversion:+ Cardinal's own Makefile (`Makefile:322-324`) converts `libmesh_CC`/
`libmesh_CXX`/`libmesh_F90` into a semicolon-separated CMake list (`LIBMESH_CC_LIST`, etc) before
passing them to any dependency's own CMake configure as `CMAKE_C_COMPILER`/`CMAKE_CXX_COMPILER`/
`CMAKE_Fortran_COMPILER`. CMake accepts a semicolon-list value for these variables, treating entries
after the first as required compiler arguments; a compound value with embedded spaces (a compiler
wrapper reported as `"ccache mpicc"`, for instance) would otherwise be misread as one invalid
executable path. This build computes the same `LIBMESH_CC_LIST`/`CXX_LIST`/`FC_LIST` and uses them
identically for every dependency's `CMAKE_ARGS` -- but, matching the Makefile exactly, *not* for
`OCCA_CXX` or the plain `CC=`/`CXX=`/`FC=` exports to the PETSc/libMesh/WASP scripts, which both
still take the raw value.

## `HDF5_ROOT`/`PETSC_DIR`/`LIBMESH_DIR`/`WASP_DIR`: a value computed but never forwarded

Two related bugs, both found by testing with every container-provided `/opt/*` dependency
environment variable unset and paths given purely via `-D` on the `cmake` command line instead of
relying on the ambient container environment:

1. The `HDF5_ROOT`-derived-from-`PETSC_DIR` computation originally lived *inside* the
   `if(CARDINAL_BUILD_PETSC OR CARDINAL_BUILD_LIBMESH OR CARDINAL_BUILD_WASP)` block, so it silently
   never ran at all -- `HDF5_ROOT` stayed permanently empty in the cache -- whenever PETSc/libMesh/
   WASP were all pre-built (the common container case, where that whole block is skipped). Moved
   outside/after that block so it always runs.
2. Even with `HDF5_ROOT` correctly computed, nothing forwarded that value, or `PETSC_DIR`/
   `LIBMESH_DIR`/`WASP_DIR`, to the final "build cardinal" step's environment. Cardinal's own native
   Makefile step then fell back to whatever each happened to already be in the *ambient* shell
   environment -- which, in a container, "happens to work" because the container sets `/opt/*`
   variables anyway, masking the fact that nothing was actually being forwarded. Confirmed with
   every `/opt/*` variable unset: the build failed outright
   (`make: +* No rule to make target '.../contrib/moose/petsc/lib/petsc/conf/petscvariables'`)
   despite `-DPETSC_DIR=/opt/petsc` having been given at configure time.

Fixed by explicitly forwarding `NEKRS_HOME`/`CONTRIB_INSTALL_DIR`/`HDF5_ROOT`/`PETSC_DIR`/
`LIBMESH_DIR`/`WASP_DIR` through the final step's `${CMAKE_COMMAND} -E env` wrapper, using whichever
value is in effect at that point (user-given pre-built path, or this build's own from-source install
path) regardless of the ambient environment. Separately, MOAB's own configure needs `HDF5_ROOT`
too, for a different reason: it calls plain `find_package(HDF5 REQUIRED COMPONENTS C HL)`, resolved
by CMake's own bundled `FindHDF5.cmake`, which consults `HDF5_ROOT` but not `HDF5_DIR`
(`config/moab.mk`'s own flag) at all -- Cardinal's native `moab.mk` gets this for free because the
Makefile itself `export HDF5_ROOT`s before that `cmake` invocation ever runs; this build's
independent `ExternalProject_Add` sub-build for MOAB has no such ambient export to inherit, so
`-DHDF5_ROOT=${HDF5_ROOT}` is passed explicitly in its own `CMAKE_ARGS`.

`OPENMC_CROSS_SECTIONS` is deliberately +not+ given the same forced treatment: unlike the six
computed values above, it must come from whatever the user has exported at *run* time, not get
frozen in as of CMake *configure* time.

`PETSC_DIR`/`LIBMESH_DIR`/`WASP_DIR` themselves deliberately do *not* get `FORCE`-written into the
cache the way `HDF5_ROOT` does: their empty-vs-set state in the cache is exactly what the
`CARDINAL_BUILD_*` branches above key off of. Force-writing a computed from-source path would make
that dependency look pre-built on the *next* reconfigure, permanently (and incorrectly) flipping it.

## The final "build cardinal" step

```cmake
add_custom_target(cardinal ALL
  COMMAND ${CMAKE_COMMAND} -E env
          NEKRS_HOME=${CARDINAL_INSTALL_DIR}
          CONTRIB_INSTALL_DIR=${CARDINAL_INSTALL_DIR}
          HDF5_ROOT=${HDF5_ROOT}
          PETSC_DIR=${PETSC_DIR}
          LIBMESH_DIR=${LIBMESH_DIR}
          WASP_DIR=${WASP_DIR}
          ENABLE_NEK=$<IF:$<BOOL:${ENABLE_NEK}>,yes,no>
          ENABLE_OPENMC=$<IF:$<BOOL:${ENABLE_OPENMC}>,yes,no>
          ENABLE_DAGMC=$<IF:$<BOOL:${ENABLE_DAGMC}>,yes,no>
          ENABLE_DOUBLE_DOWN=$<IF:$<BOOL:${ENABLE_DOUBLE_DOWN}>,yes,no>
          --
          $(MAKE) -f Makefile.cardinal
  WORKING_DIRECTORY ${CARDINAL_STAGE_DIR}
  DEPENDS ${_cardinal_depends}
  USES_TERMINAL VERBATIM)
```

Two details matter here, both found by actually running the build rather than by inspection:

+`$(MAKE)`, not a literal `make`/`make -jN`:+ CMake's Makefiles generator substitutes this with a
recursive make invocation that shares the invoking make's jobserver, so this step's parallelism is
always exactly whatever `-j` the user passed to `cmake --build`/`make` -- never a number this
project invents itself. (An earlier version defaulted it via `ProcessorCount()`, which reported the
full core count of a shared login node and forced that regardless of what the user actually asked
for.) This requires the Makefiles generator, which is checked for at configure time.

+`ENABLE_*` go through `-E env` as environment variables, not as trailing `make VAR=value`
command-line arguments.+ GNU Make gives a variable set on the command line unconditional priority
over any plain `VAR := value` assignment the makefile itself makes afterwards -- only
`override VAR := value` can beat it, which Cardinal's Makefile doesn't use anywhere. Cardinal's own
Makefile relies on exactly that freedom: `config/check_deps.mk` and the Makefile itself canonicalize
`ENABLE_DAGMC`/`ENABLE_DOUBLE_DOWN` from `yes`/`no` into `ON`/`OFF` via plain `:=`, and
`build_embree`/`build_doubledown` only get their real-or-stub definitions based on those
canonicalized `ON`/`OFF` values. Passing `ENABLE_DAGMC=yes ENABLE_DOUBLE_DOWN=no` as trailing `make`
command-line arguments silently blocks that canonicalization, so `ENABLE_DOUBLE_DOWN` never becomes
`OFF`, the `ifeq ($(ENABLE_DOUBLE_DOWN), OFF)` stub-definition block never fires, and
`build_embree`/`build_doubledown` are never defined at all -- surfacing as
`make: +* No rule to make target 'build_embree'` even with `ENABLE_DOUBLE_DOWN` correctly left at
its default off. An environment variable doesn't have this problem: it's the *lowest*-precedence
source of a variable's value, so the makefile's own reassignments apply normally, exactly like a
native user's `export ENABLE_DAGMC=yes; make` would.

Note the `ON`/`OFF` &rarr; `yes`/`no` translation via `$<IF:$<BOOL:...>,yes,no>`: CMake `option()`
cache variables are boolean, the Makefile's are `yes`/`no` strings.

## Interactive configuration (`ccmake`)

Every relevant `?=`-overridable Makefile variable is a CMake cache variable: `option()` for the
`ENABLE_*` switches (Cardinal's own NEK/OPENMC/DAGMC/DOUBLE_DOWN, plus NekRS/OCCA's own device
backends), `set(... CACHE PATH ...)` pre-filled from the environment for
`WASP_DIR`/`LIBMESH_DIR`/`PETSC_DIR`/`HDF5_ROOT`/`EIGEN3_DIR`, plus `OPENBLAS_DYNAMIC_ARCH` and
`MOOSE_BUILD_PARALLELISM` (see below). `ccmake <build-dir>` &rarr; toggle &rarr; `c` &rarr; `g`
&rarr; `cmake --build <build-dir>` works with no special-casing.

## Building PETSc/libMesh/WASP from source

+What the three scripts actually do+, read directly from the cloned MOOSE tree:

- `update_and_rebuild_petsc.sh` builds `contrib/moose/petsc` via autotools + `make`. With no
  `PETSC_PREFIX` override, PETSc installs *in place* at `$PETSC_DIR/$PETSC_ARCH` (default
  `arch-moose`) -- exactly the Makefile's own default. Also builds HDF5 itself
  (`--download-hdf5=1`) unless one's found on typical system paths.
- `update_and_rebuild_libmesh.sh` builds `contrib/moose/libmesh`, also autotools + `make`. Default
  install `${LIBMESH_SRC_DIR}/installed`. Depends on PETSc already being built (if building it too):
  falls back to `$SCRIPT_DIR/../petsc` + `PETSC_ARCH=arch-moose` if `$PETSC_DIR` isn't set.
- `update_and_rebuild_wasp.sh` builds `contrib/moose/framework/contrib/wasp` via CMake (not
  autotools), Ninja if available. Default install `${WASP_SRC_DIR}/install`. Independent of the
  other two.
- All three: the non-`--fast` invocation wipes and reconfigures+rebuilds from scratch; `--fast`
  skips configure and requires a pre-existing build dir (build+install only, no wipe). All three
  self-manage their own MOOSE-nested submodule fetch inside their non-`--fast` run.
- All three also accept and forward arbitrary extra arguments straight through to their underlying
  configure (PETSc's own `configure`, libMesh's `configure`, WASP's `cmake`), after stripping the
  flags they interpret themselves (`--fast`, `--skip-submodule-update`, etc) -- confirmed directly
  in each script's own argument-parsing loop.

+`PETSC_SCRIPT_ARGS`/`LIBMESH_SCRIPT_ARGS`/`WASP_SCRIPT_ARGS`:+ user-facing escape hatches (plain
`CACHE STRING`s, whitespace-split via `separate_arguments`) appended, in that order, after our own
hard-coded args in each `cardinal_add_moose_prereq(... SCRIPT_ARGS ...)` call -- covers anything not
already surfaced as its own CMake option (an extra PETSc `--download-` package, say). Since
PETSc's/libMesh's configure treat repeated `--key=value` options as last-one-wins, these can also
override one of our own hard-coded args (e.g. `PETSC_SCRIPT_ARGS=--with-cuda=1`) rather than just
add to them.

+CONFIGURE/BUILD split, not "always run the full script":+ `ExternalProject_Add` tracks
configure/build as independent steps, so:

- `CONFIGURE_COMMAND` is the full non-`--fast` script (the slow one).
- `BUILD_COMMAND` is the `--fast` invocation, with `BUILD_ALWAYS TRUE` -- reruns every
  `cmake --build`, cheap when nothing changed, but picks up someone hand-editing source in place.
- `INSTALL_COMMAND` is a no-op: both of the above already install internally.
- `DOWNLOAD_COMMAND` is a no-op: the script handles its own submodule fetch.

+libMesh's XDR requirement (HPC hosts without `libtirpc-devel`):+ HPC login nodes commonly lack
`libtirpc-devel` (no `rpc/xdr.h`) -- unlike the container image, which bundles it -- so libMesh's
`configure` hard-fails since `update_and_rebuild_libmesh.sh` always passes `--enable-xdr-required`
by default. Fixed per [the documented workaround](hpc_build_run_tips.md): forward
`--disable-xdr-required --disable-xdr` (Cardinal normally uses Exodus output, not libMesh's legacy
XDR format, so this costs nothing in practice).

+`petsc`/`wasp` configure-step git-lock race:+ both scripts' full (non-`--fast`) configure run
`git submodule update --init --recursive <path>` against the same shared `MOOSE_DIR` checkout. With
no ordering between them, `ExternalProject` could start both configure steps at once when building
both from source -- and they race on `MOOSE_DIR/.git/config`
(`error: could not lock config file .git/config: File exists`). Fixed with
`add_dependencies(wasp-configure petsc-configure)` -- serializes just the configure steps (not the
whole build, so PETSc's slow build can still overlap with WASP's much faster one), and only when
building both.

+Jobserver/parallelism can't be shared here+, unlike the `$(MAKE)` trick used for the final
"build cardinal" step:

- These scripts call `make -jN` with an explicit numeric `N` internally, not `$(MAKE)` -- an
  explicit `-jN` on a recursive make always starts its own independent job pool rather than joining
  an inherited jobserver.
- Worse, even the *inherited* jobserver reference is unreliable here: `CONFIGURE_COMMAND`/
  `BUILD_COMMAND` run as a plain custom command (`bash ${DEP_SCRIPT}`, not the literal `$(MAKE)`
  token), so they never get CMake's jobserver-fd-preserving recipe treatment -- the inherited
  `MAKEFLAGS` *text* (e.g. `-j16 --jobserver-auth=3,4`) still leaks into their environment, but fds
  3/4 no longer point at the real jobserver pipe by the time it reaches them. PETSc's own build
  rules see that stale `-j` in `MAKEFLAGS`, assume a real jobserver is already handing out tokens,
  and deliberately add none of their own -- silently building one file at a time. Fixed by
  overriding `MAKEFLAGS` to a clean `-jN` (no jobserver-auth reference) for these three scripts'
  environment specifically, so GNU Make bootstraps its own fresh, working jobserver instead of
  trusting the broken inherited fds.
- `MOOSE_BUILD_PARALLELISM` defaults to `min(available logical cores, 16)` (with a warning, not a
  hard failure) when neither it nor the standard `CMAKE_BUILD_PARALLEL_LEVEL` environment variable
  is set at configure time -- it's a plain `CACHE STRING`, so `-DMOOSE_BUILD_PARALLELISM=<N>` (or
  editing it in `ccmake`) overrides the default like any other cache variable.
- PETSc's own downloaded OpenBLAS additionally defaults to `DYNAMIC_ARCH=1` (kernels for every x86
  microarchitecture it knows about, auto-selected at runtime) -- safe on a cluster where the build
  host and the nodes you'll actually run on might differ, but much slower to build.
  `OPENBLAS_DYNAMIC_ARCH` (default `ON`) lets this be turned off (host-only OpenBLAS) when the
  build host's CPU is known to be representative of every node the result will run on.
- PETSc's own `--download-kokkos`/`--download-kokkos-kernels` (and NekRS's own OCCA backends)
  auto-detect whatever CUDA/HIP/OpenCL/SYCL toolkit happens to be on the build host and silently
  build GPU-enabled code regardless of what was asked for -- harmless in a container with none of
  these toolkits, but not on a bare HPC host that does. Both are tied to the same
  `ENABLE_CUDA`/`ENABLE_HIP`/`ENABLE_OPENCL`/`ENABLE_DPCPP`/`ENABLE_METAL` toggles (default `OFF`).

## Phase 2: DAGMC/MOAB/Embree/double-down

CAD/mesh-based geometry support, consumed by OpenMC via `-DDAGMC_DIR`. Same pattern as NekRS/OpenMC:
each dependency gets its own independent CMake sub-build, with `CMAKE_ARGS` matching
`config/moab.mk`/`embree.mk`/`double_down.mk`/`dagmc.mk`'s own `cmake` invocations.

+Build order+ mirrors the `*.mk` files' own prerequisite chain: MOAB and Embree are independent of
each other; double-down needs both; DAGMC needs MOAB always, and additionally double-down/Embree
only when `ENABLE_DOUBLE_DOWN` is on (`config/check_deps.mk` only requires double-down/Embree in
that case -- DAGMC can build without ray-tracing acceleration). OpenMC depends on DAGMC (when
`ENABLE_DAGMC`) and on libMesh (when building it from source).

+Configure-time validation+, matching `config/check_deps.mk`'s own logic, enforced earlier and
more clearly by CMake instead: `check_deps.mk` silently downgrades `ENABLE_DAGMC` to `no` when
`ENABLE_OPENMC` is off (DAGMC only matters as an OpenMC geometry backend) -- this is a configure-time
error here instead, so a misconfiguration fails loudly rather than silently building nothing.
`ENABLE_DAGMC` without `ENABLE_DOUBLE_DOWN` is a warning (not an error), matching
`check_deps.mk`'s own informational note about the ~100x speedup left on the table for mesh
geometries without double-down.

### A real bug in MOAB's own exported CMake config

Built the way `config/moab.mk` builds it (every optional package library -- BLAS/LAPACK/NetCDF/
METIS/etc -- off), MOAB's own generated `MOABConfig.cmake` sets:

```cmake
set(MOAB_LIBRARIES "-L<prefix>/lib ${MOAB_LIBS} ${MOAB_PACKAGE_LIBS}")
```

which, with `MOAB_PACKAGE_LIBS` empty, evaluates at configure time (in whatever project consumes
this file) to `-L<prefix>/lib -lMOAB ` with a literal trailing space -- the space between the two
`${}` references survives even though the second one expands to nothing. Both DAGMC's and
double-down's own `CMakeLists.txt` do `target_link_libraries(<tgt> ${MOAB_LIBRARIES})`, which trips
CMake policy `CMP0004` ("Libraries linked may not have leading or trailing whitespace") -- a hard
error on every CMake new enough to matter. This is a real, reproducible bug in MOAB's own CMake
export, independent of how MOAB is built: Cardinal's own Makefile-driven build (an identical
`cmake -L` invocation) hits it identically.

Rather than patch the MOAB/DAGMC/double-down submodule sources (out of scope here),
`CardinalFixupMoabConfig.cmake` patches the *generated* config file in place: it inserts a
`string(STRIP "${MOAB_LIBRARIES}" MOAB_LIBRARIES)` line right after MOAB's own `set(...)` line, so
the fix applies at evaluation time regardless of what the embedded
`${MOAB_LIBS}`/`${MOAB_PACKAGE_LIBS}` references expand to -- a plain text-substitution fix matching
the raw string literally doesn't work, since that literal text has no leading/trailing whitespace of
its own; only the *evaluated* value does.

Applied via a `moab_fixup` custom target run after `moab`'s own install step, and -- this part
matters -- patches +two+ copies: the build-tree template CMake generates during MOAB's *configure*
step, and the installed copy. Patching only the installed copy isn't enough: the final "build
cardinal" step hands off to Cardinal's own native Makefile, which -- entirely independently of
everything else -- always redoes its own `build_moab` (`make -C $(MOAB_BUILDDIR) install`) on every
invocation (`build_moab` is a `.PHONY` target with no staleness check of its own). That `make
install` blindly re-copies MOAB's own *un-patched* build-tree template over the patched installed
copy, silently reverting the fix. Patching the build-tree template too means every later
re-install -- this build's own, or Cardinal's native one -- keeps re-copying an already-fixed file
instead of reverting it: self-healing rather than a one-time patch.

## GPU/CUDA support

### `ENABLE_HYPRE_GPU`

NekRS's own bundled HYPRE builds a CUDA/HIP device variant whenever `OCCA_CUDA_ENABLED`/
`OCCA_HIP_ENABLED`, controlled by `ENABLE_HYPRE_GPU` (`cmake/hypre.cmake` inside the NekRS
submodule), defaulting `ON` regardless of caller. This can fail to build against a given GPU
toolchain independent of anything Cardinal or this build controls. Cardinal's own
[hpc.md](hpc.md) already documents needing this off on Frontier (HIP), historically by hand-editing
`config/nekrs.mk`'s CMake flags -- not applicable to this build, which never invokes that file, so
`ENABLE_HYPRE_GPU` is exposed as its own top-level option (default `ON`, matching NekRS's own
default) instead, forwarded to NekRS's `CMAKE_ARGS`.

### A genuine upstream incompatibility: HYPRE 2.32.0 vs. CUDA &ge;13.3

Building with `-DENABLE_CUDA=ON` against a CUDA 13.3 toolchain fails inside NekRS's bundled HYPRE
2.32.0: its device code (`_hypre_utilities.hpp`) uses the legacy `thrust::tuple`/`thrust::get` API,
which CUDA 13.3's bundled Thrust/CCCL (&ge;3.3.0) no longer transitively includes
(`thrust/tuple.h` must now be included explicitly) -- a genuine upstream incompatibility between
HYPRE 2.32.0 and CUDA 13.3, unrelated to anything this build does. This is confirmed to also affect
+Cardinal's native (non-CMake) build+: `config/nekrs.mk` forwards only `-DENABLE_CUDA`/
`-DENABLE_OPENCL`/`-DENABLE_HIP`/`-DENABLE_AMGX` to NekRS's CMake configure -- never
`-DENABLE_HYPRE_GPU` -- so it hits the identical failure whenever building against CUDA 13.3.

The upstream fix landed well after HYPRE 2.32.0, in HYPRE's own CUDA 13 support work:
[hypre-space/hypre#1547](https://github.com/hypre-space/hypre/pull/1547) ("Fix thrust header
inclusion") explains the exact issue above, part of the broader
[#1353](https://github.com/hypre-space/hypre/pull/1353) ("CUDA 13 support", merged into the
`hypre-3.0` branch), with further CUDA-13.2/13.3-specific fixes in
[#1487](https://github.com/hypre-space/hypre/pull/1487) and
[#1600](https://github.com/hypre-space/hypre/pull/1600). The durable fix is for NekRS to bump its
vendored HYPRE past this point -- out of scope for this build (per the standing rule against
modifying vendored/submodule source), but a concrete, actionable report-upstream item for
NekRS/Cardinal maintainers.

+Workaround:+ `-DENABLE_HYPRE_GPU=OFF` skips `HYPRE_BUILD_DEVICE` entirely; NekRS/OCCA's CUDA
backend then builds and links completely. This also uncovered a second, unrelated,
driver-less-build-environment nuance on a host with no NVIDIA driver at all: `libocca.so` records
`NEEDED libcuda.so.1`, and the only real `libcuda.so.1` available in that case is the driver
forward-compatibility shim under `/usr/local/cuda-<ver>/compat/`, which isn't on the default
`LD_LIBRARY_PATH`. A host with a real NVIDIA driver (or `--nv`/`nvidia-container-toolkit` populating
`/usr/local/nvidia/lib64`) doesn't hit this.

### Validated end-to-end on real GPU hardware

Beyond a driver-less build-only check, this was validated on an actual GPU node (NCSA Delta, A100),
confirming the workaround produces a fully functional binary, not just one that compiles:

- `cardinal-opt` runs correctly with a real driver present (`--nv` is required on
  `apptainer exec`/`shell` even on an actual GPU node -- it's what binds the host driver into the
  container at all; without it, the failure mode is identical to a driver-less host).
- A real NekRS-coupled case (`test/tests/nek_standalone/conj_ht`, run with
  `--nekrs-backend CUDA`) exercises NekRS's on-the-fly per-case compilation
  (`src/core/udf/udfMake.hpp`, which runs a fresh `cmake`+`make` at the start of every run to
  compile the case's `.udf`/`.oudf` into `libudf.so`) followed by OCCA's CUDA kernel JIT
  compilation. Both succeed, and the run executes real timesteps and converges.
- Notably, the case's multigrid coarse solver location is `CPU` (BoomerAMG/HYPRE on the host), while
  velocity/pressure/scalar solves run GPU-resident via CUDA -- i.e. `ENABLE_HYPRE_GPU=OFF` isn't
  just a way to get the build to compile; the resulting binary is fully functional, correctly
  falling back to HYPRE's CPU path for the coarse solve while everything else runs on the GPU as
  intended.

(A job-script comment in `scripts/job_frontier`, `export ENABLE_HYPRE_GPU=OFF`, might suggest a
*runtime* environment variable of the same name also matters. It doesn't: `ENABLE_HYPRE_GPU` is only
ever a CMake cache variable and a compile-time `#ifdef` guard in NekRS
(`hypreWrapperDevice.cpp`) -- there is no `getenv("ENABLE_HYPRE_GPU")` anywhere in NekRS's source.
That comment doesn't affect anything this build controls.)

## Known non-blocking rough edge

A `premake.py`/`versioner.py` `AttributeError: 'NoneType' object has no attribute 'name'` traceback
appears partway through the Cardinal compile step in every build observed so far
(`Versioner.get_app_info()` returns `None`). It does not block the build -- `make` continues and
`cardinal-opt` still links successfully -- but it's suspected to trace back to `mirror_source`
deliberately excluding `/.git` from the staged tree, leaving MOOSE's own versioning script unable to
determine Cardinal's app git info from the mirrored copy. Not yet investigated further since it
isn't blocking anything.

## Status

Phases 1-3 (container build against pre-built dependencies; Phase 2's DAGMC/MOAB/Embree/
double-down scope; building PETSc/libMesh/WASP from source on a bare host) are complete and
validated end-to-end, including with every optional feature enabled at once and with every
container-provided `/opt/*` environment variable deliberately unset. The CUDA path is validated
both at the build level (against a CUDA 13.3 toolchain, no GPU present) and, with
`-DENABLE_HYPRE_GPU=OFF`, end-to-end on real GPU hardware as described above. This build remains
labeled experimental primarily because it is new and has not yet seen broad use, not because of any
specific known-broken configuration.
