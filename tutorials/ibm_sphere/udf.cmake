#
# IBM dependencies for NekRS UDF
#

message(STATUS "====================================================")
message(STATUS "Configuring IBM dependencies")
message(STATUS "====================================================")


# ============================================================
# REQUIRED ENVIRONMENT VARIABLES
# ============================================================

foreach(var
    VTK_ROOT
    )

  if(NOT DEFINED ENV{${var}})
    message(FATAL_ERROR "${var} is not defined")
  endif()

endforeach()


# ============================================================
# VTK 9.3
# ============================================================

find_package(VTK 9.3 CONFIG REQUIRED
  COMPONENTS
    CommonCore
    CommonDataModel
    CommonExecutionModel
    FiltersCore
    IOCore
    IOXML
    IOExodus
    IOGeometry
    IOLegacy

  PATHS
    "$ENV{VTK_ROOT}/install/lib/cmake/vtk-9.3"

  NO_DEFAULT_PATH
)

message(STATUS "VTK version: ${VTK_VERSION}")


## ============================================================
## EMBREE 4
## ============================================================
#
#find_package(embree 4 CONFIG REQUIRED
#  PATHS
#    "$ENV{EMBREE_ROOT}/lib64/cmake/embree-4.4.0"
#
#  NO_DEFAULT_PATH
#)
#
#
## Embree target name can differ between installations
#if(TARGET embree)
#  set(IBM_EMBREE_TARGET embree)
#
#elseif(TARGET embree::embree)
#  set(IBM_EMBREE_TARGET embree::embree)
#
#else()
#  message(FATAL_ERROR "Embree package found but no Embree target was exported")
#endif()
#
#message(STATUS "Embree target: ${IBM_EMBREE_TARGET}")
#
#
## ============================================================
## GMP / MPFR
## ============================================================
#
#find_library(IBM_GMP_LIBRARY
#  NAMES gmp
#  PATHS "$ENV{GCC_AUX_ROOT}/lib"
#  NO_DEFAULT_PATH
#  REQUIRED
#)
#
#find_library(IBM_MPFR_LIBRARY
#  NAMES mpfr
#  PATHS "$ENV{GCC_AUX_ROOT}/lib"
#  NO_DEFAULT_PATH
#  REQUIRED
#)
#
#message(STATUS "GMP library:  ${IBM_GMP_LIBRARY}")
#message(STATUS "MPFR library: ${IBM_MPFR_LIBRARY}")
#
#
## ============================================================
## HEADER-ONLY / HEADER-BASED DEPENDENCIES
## ============================================================
##
## CGAL 5.6
## Boost 1.84
## libigl
## Eigen 3.4
##
## VTK and Embree include paths are propagated by their
## imported CMake targets.
##
#
target_include_directories(udf PRIVATE)
#target_include_directories(udf PRIVATE
#
#  # CGAL
#  "$ENV{CGAL_ROOT}/include"
#
#  # Boost
#  "$ENV{BOOST_ROOT}"
#
#  # libigl
#  "$ENV{LIBIGL_ROOT}/include"
#
#  # Eigen
#  "$ENV{EIGEN_ROOT}/include/eigen3"
#
#  # GMP / MPFR headers
#  "$ENV{GCC_AUX_ROOT}/include"
#)
#
#
## ============================================================
## PREPROCESSOR DEFINITIONS
## ============================================================

target_compile_definitions(udf PRIVATE
  ENABLE_VTK
)


# ============================================================
# LINK LIBRARIES
# ============================================================

target_link_libraries(udf PRIVATE

  # ---------------- VTK ----------------
  VTK::CommonCore
  VTK::CommonDataModel
  VTK::CommonExecutionModel
  VTK::FiltersCore
  VTK::IOCore
  VTK::IOXML
  VTK::IOExodus
  VTK::IOGeometry
  VTK::IOLegacy

  # ---------------- Embree -------------
  ${IBM_EMBREE_TARGET}

  # ---------------- CGAL dependencies --
  ${IBM_GMP_LIBRARY}
  ${IBM_MPFR_LIBRARY}

  # ---------------- Standard libs -------
  m
  dl
  pthread
)


# ============================================================
# RPATH
# ============================================================

set_property(TARGET udf APPEND PROPERTY BUILD_RPATH

  "$ENV{VTK_ROOT}/install/lib"

  "$ENV{EMBREE_ROOT}/lib64"

  "$ENV{GCC_AUX_ROOT}/lib"
)


# ============================================================
# VTK MODULE INITIALIZATION
# ============================================================

if(COMMAND vtk_module_autoinit)

  vtk_module_autoinit(
    TARGETS udf

    MODULES
      VTK::CommonCore
      VTK::CommonDataModel
      VTK::CommonExecutionModel
      VTK::FiltersCore
      VTK::IOCore
      VTK::IOXML
      VTK::IOExodus
      VTK::IOGeometry
      VTK::IOLegacy
  )

endif()


message(STATUS "====================================================")
message(STATUS "IBM dependencies configured successfully")
message(STATUS "====================================================")
