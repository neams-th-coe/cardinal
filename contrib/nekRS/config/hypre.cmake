# If user-specified a HYPRE installation, create an imported library target
if (NOT "${HYPRE_DIR}" STREQUAL "")
  if (NOT IS_DIRECTORY ${HYPRE_DIR})
    message(FATAL_ERROR "User specified HYPRE_DIR=${HYPRE_DIR}, but the directory doesn't exist")
  endif()
  message(STATUS "Using HYPRE installation in ${HYPRE_DIR}")
  find_library(HYPRE_FOUND libHYPRE.so PATHS ${HYPRE_DIR}/lib REQUIRED NO_DEFAULT_PATH)
  find_file(HYPRE_parcsr_ls.h PATHS ${HYPRE_DIR}/include REQUIRED NO_DEFAULT_PATH)
  find_file(HYPRE.h PATHS ${HYPRE_DIR}/include REQUIRED NO_DEFAULT_PATH)

  add_library(HYPRE SHARED IMPORTED)
  set_target_properties(HYPRE PROPERTIES 
    IMPORTED_LOCATION ${HYPRE_DIR}/lib/libHYPRE.so
    IMPORTED_NO_SONAME FALSE)
  target_include_directories(HYPRE INTERFACE ${HYPRE_DIR}/include)

# Otherwise, build HYPRE
else()
  # These two variables are significant to HYPRE's CMakeLists, not our own.
  # HYPRE's CMakeLists leaks some variables into parent project, and this is a workaround.
  set(HYPRE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "" FORCE)
  set(HYPRE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "" FORCE)

  set(HYPRE_ENABLE_SINGLE OFF CACHE BOOL "" FORCE)
  set(HYPRE_ENABLE_MIXEDINT ON CACHE BOOL "" FORCE)

  # First try using the source that's already in the 3rd_party dir.
  set(HYPRE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/hypre)
  if (IS_DIRECTORY ${HYPRE_SOURCE_DIR})
    message(STATUS "Using HYPRE source in ${HYPRE_SOURCE_DIR}")
    add_subdirectory(${HYPRE_SOURCE_DIR}/src)
    get_property(HYPRE_BINARY_DIR TARGET HYPRE PROPERTY BINARY_DIR)
  # Otherwise, download the source
  else()
    FetchContent_Declare(
      hypre_content
      URL https://github.com/hypre-space/hypre/archive/v${HYPRE_VER}.tar.gz )
    FetchContent_GetProperties(hypre_content)
    if (NOT hypre_content_POPULATED)
      FetchContent_Populate(hypre_content)
    endif()
    set(HYPRE_SOURCE_DIR ${hypre_content_SOURCE_DIR})
    set(HYPRE_BINARY_DIR ${hypre_content_BINARY_DIR})
    # * Exclude from all since HYPRE CMakeLists adds a bunch of targets we don't need
    #   libHYPRE will be build just fine, since we've explicitly declared it as a dependency
    add_subdirectory(${HYPRE_SOURCE_DIR}/src ${HYPRE_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
  # This conflicts with the stdlib "version" header...
  file(REMOVE ${HYPRE_SOURCE_DIR}/src/utilities/version)
endif()

