#
# IBM dependencies for NekRS UDF
#

message(STATUS "====================================================")
message(STATUS "Configuring IBM dependencies")
message(STATUS "====================================================")

# ============================================================
# REQUIRED ENVIRONMENT VARIABLES
# ============================================================

foreach(var VTK_ROOT)

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

target_include_directories(udf PRIVATE)

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
