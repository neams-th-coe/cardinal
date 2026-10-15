# cmake -P script: build-time (not configure-time) safety check that the
# ambient shell's METHOD environment variable, if set, still matches the
# METHOD this build was actually configured for.
#
# METHOD is frozen into the final "build cardinal" custom target's generated
# recipe at *configure* time (see the CMAKE_BUILD_TYPE/METHOD block near the
# top of CMakeLists.txt, and the METHOD=${_cardinal_method} note on that
# target) -- exactly like NEKRS_HOME/PETSC_DIR/etc, `cmake`'s own generated
# Makefiles never re-read $ENV{METHOD} on a later `cmake --build`/`make`,
# only on an actual reconfigure. Confirmed directly by testing: with a build
# already configured for METHOD=dbg, `METHOD=opt cmake --build build` (no
# reconfigure) silently keeps building cardinal-dbg -- the new METHOD is
# simply never consulted anywhere. That's surprising enough to be worth an
# explicit error instead of a silent no-op, so this script re-checks METHOD
# at real build time, as the first step of the `cardinal` custom target,
# using whatever the *current* invocation's environment actually holds
# (unlike the rest of that target's recipe, which runs under a
# `${CMAKE_COMMAND} -E env METHOD=${_cardinal_method} ...` override and so
# can never see this).
#
# Expected -D argument: CARDINAL_METHOD (the METHOD this build was
# configured for -- CMakeLists.txt's own derived _cardinal_method).
cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED CARDINAL_METHOD OR CARDINAL_METHOD STREQUAL "")
  message(FATAL_ERROR "CardinalCheckMethod.cmake: CARDINAL_METHOD is required")
endif()

if(DEFINED ENV{METHOD} AND NOT "$ENV{METHOD}" STREQUAL "" AND
   NOT "$ENV{METHOD}" STREQUAL "${CARDINAL_METHOD}")
  message(FATAL_ERROR
    "Cardinal: METHOD=$ENV{METHOD} in the environment does not match "
    "METHOD=${CARDINAL_METHOD}, which this build was configured for. "
    "METHOD is fixed at 'cmake' *configure* time (see CMakeLists.txt), not "
    "at 'cmake --build'/'make' time, so changing it here alone has no "
    "effect on the build that's about to run. Re-run 'cmake <build-dir>' "
    "(with the new METHOD in the environment, or a corresponding "
    "-DCMAKE_BUILD_TYPE=...) to actually change it, or unset METHOD to "
    "build ${CARDINAL_METHOD} as configured.")
endif()
