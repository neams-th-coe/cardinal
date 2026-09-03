# cmake -P script: installs Cardinal -- the app (cardinal-opt, cardinal-dbg,
# or whichever METHOD this build was configured for -- see CARDINAL_APP
# below), the MOOSE framework/module libraries it links, and nekRS/OpenMC/
# MOAB/Embree/double-down/DAGMC/nuclear_data -- into CARDINAL_FINAL_INSTALL_
# DIR as a self-contained, relocatable install: one that keeps working after
# CARDINAL_STAGE_DIR (the CMake build tree, which includes
# CARDINAL_INSTALL_DIR, the *staging* copy of those last six dependencies
# built up as a side effect of the ordinary build) is deleted.
#
# Two things happen here, in order:
#
# 1. CARDINAL_INSTALL_DIR's own staged tree (nekRS/OpenMC/MOAB/Embree/
#    double-down/DAGMC/nuclear_data, already a complete install of those
#    six from their own ExternalProject_Add install steps, all merged
#    together into one shared prefix -- CONTRIB_INSTALL_DIR ?= $(CARDINAL_
#    DIR)/install in Cardinal's own Makefile.cardinal, used identically by
#    every one of the six, is that same merge, so this isn't a superbuild
#    quirk) is copied wholesale into CARDINAL_FINAL_INSTALL_DIR/install
#    (a no-op only in the uncommon case where that and CARDINAL_INSTALL_
#    DIR already happen to be the same path -- CMAKE_INSTALL_PREFIX's own
#    default, see CMakeLists.txt, deliberately avoids that coinciding, so
#    this real copy is the normal case, not the exception). Kept in its
#    own subdirectory, not the final prefix's root, precisely because
#    it's a merge of six independent projects' install trees (one of
#    them, nekRS, scatters a fair amount of loose top-level content of
#    its own -- LICENSE, examples/, gslib/, modulefiles/, ... -- found
#    live) -- mixed in with the app's own bin/lib/share below, that
#    would bury cardinal-opt itself in a pile of unrelated files. Every
#    ELF file in the copy (executables, libraries) gets its RPATH
#    rewritten the same way as step 2 below, since these were built
#    assuming they'd live at CARDINAL_INSTALL_DIR specifically (e.g.
#    MOAB's own CMAKE_INSTALL_RPATH is set to that exact absolute path --
#    see CMakeLists.txt) and would otherwise reach back into
#    CARDINAL_STAGE_DIR once copied elsewhere.
# 2. cardinal-opt and the MOOSE framework/module libraries it needs --
#    which, unlike the six above, have no install step of their own at
#    all -- go into CARDINAL_FINAL_INSTALL_DIR's own bin/lib/share
#    (*not* nested under install/ alongside step 1 -- MOOSE's own
#    installed-data-file lookup, see below, hardcodes an <exe-dir>/../
#    share/<name>/data convention that requires the app to sit directly
#    under the real prefix, not an extra level down).
#
# Why this needs more than a plain copy: cardinal-opt's own RUNPATH (and,
# critically, the RUNPATH baked into *each* of the MOOSE framework/module
# libraries it links -- confirmed directly with readelf -d) is a list of
# absolute paths into CARDINAL_STAGE_DIR (contrib/moose/framework,
# contrib/moose/modules/*/lib, lib/, test/lib/, ...), alongside genuinely
# external ones (/opt/petsc/lib, /opt/openmpi/lib, ...). DT_RUNPATH is
# *not* transitive -- glibc's loader only consults the RUNPATH of the
# object doing the lookup, not the top-level executable's -- which is
# exactly why each of these libraries carries its own RUNPATH pointing at
# its sibling libraries in the first place. So merely copying the files
# without also rewriting each one's own RUNPATH would still reach back
# into the (now-deleted) build tree the moment the loader resolves a
# second-level dependency.
#
# Step 2's approach: build an index of every *.so* under Cardinal's own
# compiled output plus contrib/moose (the framework/modules, and -- if
# built from source -- PETSc/libMesh/WASP too, all of which land
# somewhere under there), then walk each DT_NEEDED name via
# `patchelf --print-needed` (repeated on every newly-found library, for
# the full transitive closure) and copy+relink whatever matches that
# index; a name that doesn't match is left alone (a genuine system/
# container/module lib, or one of the six dependencies step 1 already
# handled).
#
# Deliberately *not* implemented via `ldd`, which resolves each NEEDED
# name through the object's actual RUNPATH: found live, that approach
# broke on the *second* install onward, because by then CARDINAL_APP's
# own RUNPATH already contains CARDINAL_INSTALL_DIR/lib (Cardinal's own
# Makefile bakes CONTRIB_INSTALL_DIR in there, ahead of the build-tree
# paths -- confirmed with readelf -d) -- which, after a first install,
# already holds copies of these same libraries. `ldd` would resolve
# straight to that earlier copy instead of the authoritative
# CARDINAL_STAGE_DIR original, silently poisoning every install after the
# first into never picking up a rebuilt library again. Consulting a
# pre-built index instead of asking the loader to resolve anything
# sidesteps that class of bug entirely.
#
# Relinking in step 2 isn't purely "flatten into lib/", either:
# cardinal-opt's own RUNPATH carries a *separate* entry for
# CARDINAL_INSTALL_DIR/lib (found live -- initially collapsed into the
# same flat lib/-relative target as its MOOSE entries, which broke it
# outright once step 1 started nesting the staged dependencies under
# install/ instead of sharing CARDINAL_FINAL_INSTALL_DIR's own lib/ with
# them). See cardinal_install_relink's own comment below for how the two
# kinds of entry are told apart and remapped differently.
#
# Expected -D arguments:
#   CARDINAL_APP               - path to the built cardinal-<method> executable
#   CARDINAL_STAGE_DIR         - the build tree root (CMAKE_BINARY_DIR)
#   CARDINAL_INSTALL_DIR       - nekRS/OpenMC/MOAB/Embree/double-down/DAGMC/
#                                 nuclear_data's fixed staging location
#                                 (always CARDINAL_STAGE_DIR/install)
#   CARDINAL_FINAL_INSTALL_DIR - the real, user-facing install prefix
#                                 (CMAKE_INSTALL_PREFIX): the app's own
#                                 bin/lib/share go directly here, the
#                                 staged dependencies under this/install
#   CARDINAL_SOURCE_DIR        - the Cardinal checkout CMake was
#                                 configured from (CMAKE_SOURCE_DIR); only
#                                 used to decide whether to add the
#                                 cardinal-opt root symlink below
cmake_minimum_required(VERSION 3.21)

foreach(_required CARDINAL_APP CARDINAL_STAGE_DIR CARDINAL_INSTALL_DIR CARDINAL_FINAL_INSTALL_DIR CARDINAL_SOURCE_DIR)
  if(NOT ${_required})
    message(FATAL_ERROR "CardinalInstallApp.cmake: ${_required} is required")
  endif()
endforeach()
if(NOT EXISTS "${CARDINAL_APP}")
  message(FATAL_ERROR "CardinalInstallApp.cmake: '${CARDINAL_APP}' does not exist -- build it first")
endif()

find_program(_patchelf patchelf)
if(NOT _patchelf)
  message(FATAL_ERROR
    "CardinalInstallApp.cmake: 'patchelf' is required to produce a "
    "relocatable install (RPATH rewriting) but wasn't found on PATH")
endif()
find_program(_rsync rsync)
if(NOT _rsync)
  message(FATAL_ERROR "CardinalInstallApp.cmake: 'rsync' is required but wasn't found on PATH")
endif()

# Normalized (trailing-slash-free) forms used for prefix matching below.
get_filename_component(_stage_dir "${CARDINAL_STAGE_DIR}" ABSOLUTE)
get_filename_component(_staging_install_dir "${CARDINAL_INSTALL_DIR}" ABSOLUTE)
get_filename_component(_final_install_dir "${CARDINAL_FINAL_INSTALL_DIR}" ABSOLUTE)
# Where step 1 (below) lands -- deliberately CARDINAL_FINAL_INSTALL_DIR's
# own install/ subdirectory, not its root; see the file header.
set(_final_staged_dir "${_final_install_dir}/install")

file(MAKE_DIRECTORY "${_final_install_dir}/bin")
file(MAKE_DIRECTORY "${_final_install_dir}/lib")

# Computes the $ORIGIN-relative replacement for one old RPATH entry
# already known to be under CARDINAL_INSTALL_DIR -- i.e. one of step 1's
# six staged dependencies, now living under CARDINAL_FINAL_INSTALL_DIR/
# install instead. Shared by both relink functions below: cardinal-opt
# itself needs exactly this, not just its *own* MOOSE libraries -- its
# RUNPATH carries a separate entry for CARDINAL_INSTALL_DIR/lib (found
# live: initially missed, since it was collapsed into the same flat
# lib/-relative target as the MOOSE entries -- broke cardinal-opt with
# "libocca.so: cannot open shared object file" once step 1 started
# nesting the staged dependencies under install/ instead of sharing
# CARDINAL_FINAL_INSTALL_DIR's own lib/ with them).
function(cardinal_install_relink_staged_entry entry_abs dest_dir out_var)
  file(RELATIVE_PATH _rel_to_staging "${_staging_install_dir}" "${entry_abs}")
  set(_new_target "${_final_staged_dir}/${_rel_to_staging}")
  file(RELATIVE_PATH _rel_from_dest "${dest_dir}" "${_new_target}")
  string(REGEX REPLACE "/$" "" _rel_from_dest "${_rel_from_dest}")
  if(_rel_from_dest STREQUAL "" OR _rel_from_dest STREQUAL ".")
    set(${out_var} "\$ORIGIN" PARENT_SCOPE)
  else()
    set(${out_var} "\$ORIGIN/${_rel_from_dest}" PARENT_SCOPE)
  endif()
endfunction()

# Rewrites patchelf's own multi-entry ":"-joined path list. Applied to the
# *destination* (already-copied) file, using the *source* file's original
# rpath as input (before it was moved, so the build-tree-vs-external
# distinction is unambiguous). Each old entry becomes one of:
#   - unchanged, if already $ORIGIN-relative or genuinely external;
#   - individually remapped under CARDINAL_FINAL_INSTALL_DIR/install, if
#     it was one of step 1's six staged dependencies (see
#     cardinal_install_relink_staged_entry above);
#   - collapsed to the single shared $ORIGIN<origin_suffix> target, if it
#     was some *other* CARDINAL_STAGE_DIR path (Cardinal's own compiled
#     output, or a MOOSE framework/module library) -- this is the one
#     genuinely flattening step, correct here because the caller already
#     knows dest itself was copied into that same flat target.
function(cardinal_install_relink source_for_rpath dest origin_suffix)
  execute_process(
    COMMAND "${_patchelf}" --print-rpath "${source_for_rpath}"
    OUTPUT_VARIABLE _old_rpath
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "CardinalInstallApp.cmake: 'patchelf --print-rpath ${source_for_rpath}' failed")
  endif()
  get_filename_component(_dest_dir "${dest}" DIRECTORY)
  set(_new_entries "")
  set(_added_flat_target FALSE)
  if(_old_rpath)
    string(REPLACE ":" ";" _old_rpath_list "${_old_rpath}")
    foreach(_entry IN LISTS _old_rpath_list)
      if(_entry MATCHES "^\\$ORIGIN")
        # Already self-relative -- unaffected by relocation, and not a
        # real filesystem path to resolve (get_filename_component(...
        # ABSOLUTE) would otherwise mangle the literal "$ORIGIN" token
        # into a bogus path).
        list(APPEND _new_entries "${_entry}")
        continue()
      endif()
      get_filename_component(_entry_abs "${_entry}" ABSOLUTE)
      string(FIND "${_entry_abs}/" "${_staging_install_dir}/" _staged_pos)
      string(FIND "${_entry_abs}/" "${_stage_dir}/" _stage_pos)
      if(_staged_pos EQUAL 0)
        cardinal_install_relink_staged_entry("${_entry_abs}" "${_dest_dir}" _remapped)
        list(APPEND _new_entries "${_remapped}")
      elseif(_stage_pos EQUAL 0)
        if(NOT _added_flat_target)
          list(APPEND _new_entries "\$ORIGIN${origin_suffix}")
          set(_added_flat_target TRUE)
        endif()
      else()
        list(APPEND _new_entries "${_entry}")  # genuinely external
      endif()
    endforeach()
  endif()
  list(REMOVE_DUPLICATES _new_entries)
  list(JOIN _new_entries ":" _new_rpath)
  execute_process(
    COMMAND "${_patchelf}" --set-rpath "${_new_rpath}" "${dest}"
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "CardinalInstallApp.cmake: 'patchelf --set-rpath ... ${dest}' failed")
  endif()
endfunction()

# For the bulk staging-tree copy below: this copy is a structure-
# preserving *mirror* -- rsync -a reproduces CARDINAL_INSTALL_DIR's own
# internal layout exactly under CARDINAL_FINAL_INSTALL_DIR/install, one
# absolute-path move of the whole tree -- unlike cardinal_install_relink
# above, which additionally *flattens* whatever it copies that ISN'T one
# of the six staged dependencies (Cardinal's own compiled output, MOOSE
# framework/module libraries) into one destination directory, so
# collapsing those specific entries into one fixed $ORIGIN<suffix> is
# correct there. Every entry this function touches instead gets its own,
# individually-computed $ORIGIN-relative replacement (via
# cardinal_install_relink_staged_entry above), not one shared value:
# collapsing would both point at the wrong place (an install like
# nekRS's own -- confirmed with readelf -- has *two different*
# build-tree rpath entries on the same binary, one for lib/ and a
# separate one for occa/lib/, not interchangeable) and silently drop
# every entry after the first.
#
# An old entry under CARDINAL_STAGE_DIR but *not* under
# CARDINAL_INSTALL_DIR specifically (e.g. a from-source PETSc/libMesh/
# WASP lib under contrib/moose, which this copy doesn't touch at all) has
# no corresponding new location to compute -- left as its original
# absolute path (a real, narrower limitation: that specific dependency
# combination isn't fully relocatable yet), with a warning so it's
# visible rather than silently wrong.
#
# Also silently skips anything patchelf doesn't recognize as ELF at all
# (most files in a large third-party install -- headers, scripts, *.cmake
# package configs, docs, ...) rather than treating that as an error.
function(cardinal_install_relink_mirrored source dest)
  execute_process(
    COMMAND "${_patchelf}" --print-rpath "${source}"
    OUTPUT_VARIABLE _old_rpath
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    return()  # not an ELF file (or no dynamic section) -- nothing to do
  endif()
  if(NOT _old_rpath)
    return()  # no RPATH at all -- nothing to rewrite
  endif()

  get_filename_component(_dest_dir "${dest}" DIRECTORY)
  set(_new_entries "")
  string(REPLACE ":" ";" _old_rpath_list "${_old_rpath}")
  foreach(_entry IN LISTS _old_rpath_list)
    if(_entry MATCHES "^\\$ORIGIN")
      # Already self-relative (e.g. Embree's own CMake install already
      # uses $ORIGIN) -- unaffected by relocation, and not a real
      # filesystem path to resolve (get_filename_component(... ABSOLUTE)
      # would otherwise mangle the literal "$ORIGIN" token into a bogus
      # path that could wrongly look like it's inside the build tree).
      list(APPEND _new_entries "${_entry}")
      continue()
    endif()
    get_filename_component(_entry_abs "${_entry}" ABSOLUTE)
    string(FIND "${_entry_abs}/" "${_staging_install_dir}/" _staged_pos)
    string(FIND "${_entry_abs}/" "${_stage_dir}/" _stage_pos)
    if(_staged_pos EQUAL 0)
      cardinal_install_relink_staged_entry("${_entry_abs}" "${_dest_dir}" _remapped)
      list(APPEND _new_entries "${_remapped}")
    elseif(_stage_pos EQUAL 0)
      message(WARNING "CardinalInstallApp.cmake: ${source}: RPATH entry '${_entry}' is inside "
        "the build tree but outside the staged install (likely a from-source PETSc/libMesh/"
        "WASP) -- left as an absolute path; this install won't be relocatable for it.")
      list(APPEND _new_entries "${_entry}")
    else()
      list(APPEND _new_entries "${_entry}")  # genuinely external
    endif()
  endforeach()
  list(REMOVE_DUPLICATES _new_entries)
  list(JOIN _new_entries ":" _new_rpath)
  execute_process(
    COMMAND "${_patchelf}" --set-rpath "${_new_rpath}" "${dest}"
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "CardinalInstallApp.cmake: 'patchelf --set-rpath ... ${dest}' failed")
  endif()
endfunction()

# --- Step 1: bring the staged nekRS/OpenMC/MOAB/Embree/double-down/DAGMC/
# nuclear_data tree along to the real install prefix, unless it's already
# there. -a preserves the symlink chains libtool-style shared libraries
# rely on (libfoo.so -> libfoo.so.1 -> libfoo.so.1.2.3).
if(NOT _staging_install_dir STREQUAL _final_staged_dir)
  file(MAKE_DIRECTORY "${_final_staged_dir}")
  execute_process(
    COMMAND "${_rsync}" -a "${_staging_install_dir}/" "${_final_staged_dir}/"
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "CardinalInstallApp.cmake: rsync of ${_staging_install_dir} to ${_final_staged_dir} failed")
  endif()

  # The whole tree, not just bin/ and lib/ -- a third-party install like
  # nekRS's own scatters ELF content in other places too (occa/lib/,
  # gslib/, nek5000/, tools/, ...); cheaper to probe every file (most
  # calls are a fast, immediate failure below) than to hand-enumerate
  # every location that happens to matter today.
  file(GLOB_RECURSE _copied_files LIST_DIRECTORIES false "${_final_staged_dir}/*")
  foreach(_dest IN LISTS _copied_files)
    if(IS_SYMLINK "${_dest}")
      continue()  # relink the real file; the symlink alongside it already points at it by name
    endif()
    file(RELATIVE_PATH _rel "${_final_staged_dir}" "${_dest}")
    set(_source "${_staging_install_dir}/${_rel}")
    if(EXISTS "${_source}")
      cardinal_install_relink_mirrored("${_source}" "${_dest}")
    endif()
  endforeach()
  message(STATUS "Cardinal: copied the staged nekRS/OpenMC/MOAB/Embree/double-down/DAGMC/nuclear_data install from ${_staging_install_dir} to ${_final_staged_dir}")
endif()

# --- Step 2: the app itself, plus whatever MOOSE framework/module
# libraries it needs -- these have no install step of their own at all.

# The index: basename -> realpath, for every *.so* under Cardinal's own
# compiled output and contrib/moose (see rationale above). First match
# wins on a basename collision (not expected in practice).
file(GLOB_RECURSE _stage_so_files LIST_DIRECTORIES false
  "${_stage_dir}/lib/*.so*"
  "${_stage_dir}/test/lib/*.so*"
  "${_stage_dir}/contrib/moose/*.so*")
set(_index_names "")
set(_index_paths "")
foreach(_f IN LISTS _stage_so_files)
  get_filename_component(_f_name "${_f}" NAME)
  list(FIND _index_names "${_f_name}" _idx)
  if(_idx EQUAL -1)
    get_filename_component(_f_real "${_f}" REALPATH)
    list(APPEND _index_names "${_f_name}")
    list(APPEND _index_paths "${_f_real}")
  endif()
endforeach()

# Returns (in out_var) the literal DT_NEEDED strings of `target` -- no
# path resolution at all, just the raw list `patchelf` reads out of the
# ELF dynamic section.
function(cardinal_install_needed_names target out_var)
  execute_process(
    COMMAND "${_patchelf}" --print-needed "${target}"
    OUTPUT_VARIABLE _out
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "CardinalInstallApp.cmake: 'patchelf --print-needed ${target}' failed")
  endif()
  string(STRIP "${_out}" _out)
  string(REPLACE "\n" ";" _names "${_out}")
  list(REMOVE_ITEM _names "")
  set(${out_var} "${_names}" PARENT_SCOPE)
endfunction()

# BFS over the transitive closure of DT_NEEDED names, starting from the
# app itself: `patchelf --print-needed` only reports *direct* deps, so
# every newly-copied library's own needed names get pushed back onto the
# worklist too (replicating what `ldd` gives for free, but through the
# index above instead of the loader's own path resolution).
cardinal_install_needed_names("${CARDINAL_APP}" _worklist)
set(_seen "")            # NEEDED names already processed (skip repeats)
set(_installed_libs "")  # install/lib basenames copied so far, for the summary
while(_worklist)
  list(POP_FRONT _worklist _needed_name)
  list(FIND _seen "${_needed_name}" _already)
  if(NOT _already EQUAL -1)
    continue()
  endif()
  list(APPEND _seen "${_needed_name}")

  list(FIND _index_names "${_needed_name}" _idx)
  if(_idx EQUAL -1)
    continue()  # not ours -- system/container lib, or one of step 1's dependencies
  endif()
  list(GET _index_paths ${_idx} _real)

  # Always (re-)copy and relink, even if a same-named file is already
  # present from an earlier install -- an "install" should always reflect
  # the current build, same as a plain `make install` never skips a file
  # just because it already exists; skipping here would let an
  # already-installed copy quietly go stale across incremental rebuilds.
  get_filename_component(_real_name "${_real}" NAME)
  set(_dest "${_final_install_dir}/lib/${_real_name}")
  file(COPY_FILE "${_real}" "${_dest}")
  file(CHMOD "${_dest}" PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE)
  cardinal_install_relink("${_real}" "${_dest}" "")
  list(APPEND _installed_libs "${_real_name}")

  # Recreate the DT_NEEDED-visible name as a symlink alongside it when it
  # differs from the real file's own basename (the common libtool case:
  # DT_NEEDED carries the SONAME, e.g. libfoo.so.1, symlinked to the fully
  # versioned real file, e.g. libfoo.so.1.2.3).
  if(NOT _needed_name STREQUAL _real_name)
    set(_link "${_final_install_dir}/lib/${_needed_name}")
    file(REMOVE "${_link}")
    file(CREATE_LINK "${_real_name}" "${_link}" SYMBOLIC)
  endif()

  cardinal_install_needed_names("${_real}" _more_needed)
  list(APPEND _worklist ${_more_needed})
endwhile()
list(REMOVE_DUPLICATES _installed_libs)

get_filename_component(_app_name "${CARDINAL_APP}" NAME)

# The METHOD this build was actually configured for (opt, dbg, devel, prof,
# oprof, ...) -- not re-derived independently here, just read back off
# CARDINAL_APP's own name (cardinal-<method>), so this script automatically
# tracks whatever CMakeLists.txt's own _cardinal_method resolved to without
# needing that value passed in separately, and without hardcoding which
# METHOD values exist. Used below for the module data-dir check, matching
# MOOSE's own library naming convention (lib<module>-<method>.so).
#
# FATAL_ERROR, not a warning, if this can't be determined: it's not a
# cosmetic gap. Registry::determineDataFilePath (framework/src/base/
# Registry.C) mooseErrors immediately -- not lazily, not only if that
# module's functionality is actually used -- the moment a registered data
# path can't be found, and that registration runs via a static initializer
# at process load, before main() does anything (confirmed directly: a
# missing MOOSE-framework data dir made cardinal-opt mooseError "on
# startup", every single invocation). solid_mechanics (SolidMechanicsApp.C)
# registers one and is always linked into Cardinal's own module set today,
# so an install missing its data/ directory isn't degraded -- it's a binary
# that refuses to start at all, while `cardinal-install` reports success.
if(_app_name MATCHES "^cardinal-(.+)$")
  set(_app_method "${CMAKE_MATCH_1}")
else()
  message(FATAL_ERROR "CardinalInstallApp.cmake: '${_app_name}' doesn't "
    "match cardinal-<method> -- can't determine which METHOD's module "
    "data directories (e.g. solid_mechanics/data) to install, and skipping "
    "them would silently produce a binary that mooseErrors on startup")
endif()

set(_app_dest "${_final_install_dir}/bin/${_app_name}")
file(COPY_FILE "${CARDINAL_APP}" "${_app_dest}")
file(CHMOD "${_app_dest}" PERMISSIONS
  OWNER_READ OWNER_WRITE OWNER_EXECUTE
  GROUP_READ GROUP_EXECUTE
  WORLD_READ WORLD_EXECUTE)
cardinal_install_relink("${CARDINAL_APP}" "${_app_dest}" "/../lib")

# Convenience symlink at the prefix root, alongside install/ -- so the
# app is reachable as CARDINAL_FINAL_INSTALL_DIR/cardinal-opt too,
# matching where the native Makefile build puts it (the checkout root,
# no bin/ of its own). Safe wherever it's added: MOOSE's own
# Moose::getExec() (framework/src/utils/ExecutablePath.C) resolves the
# running executable's path via readlink("/proc/<pid>/exe"), which the
# kernel always resolves to the real underlying file regardless of which
# symlink (if any) was used to invoke it -- so running the app through
# this symlink still correctly finds CARDINAL_FINAL_INSTALL_DIR/share/
# <name>/data, the same as running bin/cardinal-opt directly. Only added
# when CARDINAL_FINAL_INSTALL_DIR *is* the Cardinal checkout itself
# (e.g. --install-prefix $(pwd), see with_cmake.md) -- that's the one
# case the "matches the native Makefile build" comparison is actually
# about; for an arbitrary/unrelated install prefix there's no checkout
# for the symlink to be matching the layout of, so it'd just be clutter.
get_filename_component(_source_dir_abs "${CARDINAL_SOURCE_DIR}" ABSOLUTE)
if(_final_install_dir STREQUAL _source_dir_abs)
  set(_app_root_link "${_final_install_dir}/${_app_name}")
  file(REMOVE "${_app_root_link}")
  file(CREATE_LINK "bin/${_app_name}" "${_app_root_link}" SYMBOLIC)
endif()

# Runtime library loading isn't the only thing "whatever it needs to run"
# covers -- MOOSE's own Registry::determineDataFilePath (framework/src/
# base/Registry.C) looks for each registered app/module's "data" directory
# at a documented, already-relocatable convention:
#   <exe-dir>/../share/<name>/data   (installed)
#   <in-tree path relative to the registering *App.C>   (fallback)
# Found live: cardinal-opt (built here, then run with CARDINAL_STAGE_DIR
# moved out of the way) immediately mooseError'd on startup --
# "Failed to determine data file path for 'moose'" -- since neither
# location existed. Every MOOSE app/module that registers one (grep for
# registerAppDataFilePath) requires its own literally-named "data"
# directory alongside its own source (Registry.C enforces this), so rather
# than hardcode which of Cardinal's linked modules happen to register one
# today, just check: the framework's own (registered as "moose", always
# needed) plus, for every module library actually copied above, whether
# that module has its own data/ dir (module directory name == registered
# name in every case observed, e.g. modules/solid_mechanics/data ->
# "solid_mechanics") -- a plain filesystem check, so it stays correct
# regardless of which modules a given ENABLE_* configuration links in.
#
# The METHOD-matching regex below uses _app_method (read off CARDINAL_APP's
# own name above), not a hardcoded opt/dbg alternation -- MOOSE module
# libraries are named lib<module>-<method>.so for every METHOD (devel,
# prof, oprof included, not just opt/dbg), so matching literally whatever
# method the app itself was built for is both simpler and correct for all
# five, rather than needing this list kept in sync with libmesh_method.m4's.
function(cardinal_install_data_dir name data_dir)
  if(IS_DIRECTORY "${data_dir}")
    file(MAKE_DIRECTORY "${_final_install_dir}/share/${name}")
    file(COPY "${data_dir}" DESTINATION "${_final_install_dir}/share/${name}")
    message(STATUS "Cardinal: installed '${name}' data files into ${_final_install_dir}/share/${name}/data")
  endif()
endfunction()

cardinal_install_data_dir(moose "${CARDINAL_STAGE_DIR}/contrib/moose/framework/data")
foreach(_lib_name IN LISTS _installed_libs)
  if(_lib_name MATCHES "^lib(.+)-${_app_method}\\.so")
    cardinal_install_data_dir("${CMAKE_MATCH_1}" "${CARDINAL_STAGE_DIR}/contrib/moose/modules/${CMAKE_MATCH_1}/data")
  endif()
endforeach()

list(LENGTH _installed_libs _n_libs)
message(STATUS
  "Cardinal: installed ${_app_name} into ${_final_install_dir}/bin, "
  "relinked against ${_n_libs} build-tree librar(y/ies) copied into "
  "${_final_install_dir}/lib (external dependencies left in place)")
