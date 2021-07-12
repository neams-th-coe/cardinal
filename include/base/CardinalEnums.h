#pragma once

#include "MooseEnum.h"

MooseEnum getNekOrderEnum();
MooseEnum getNekFieldEnum();
MooseEnum getOperationEnum();
MooseEnum getTallyTypeEnum();
MooseEnum getTallyCellFilterEnum();

namespace order
{
  /// Enumeration of possible surface order reconstructions for nekRS solution transfer
  enum NekOrderEnum
  {
    first,
    second
  };
}

namespace field
{
  /// Enumeration of possible fields to integrate in nekRS
  enum NekFieldEnum
  {
    x_velocity,
    y_velocity,
    z_velocity,
    velocity,
    temperature,
    pressure,
    unity
  };
}

namespace operation
{
  /// Enumeration of possible operations to perform in global postprocessors
  enum OperationEnum
  {
    max,
    min
  };
}

namespace tally
{
  /// Type of tally to construct for the OpenMC model
  enum TallyTypeEnum
  {
    cell,
    mesh
  };
}

namespace filter
{
  /// Type of filter to use for cell-based coupling
  enum CellFilterEnum
  {
    cell,
    cell_instance
  };
}

namespace coupling
{
  /// Type of feedback in Monte Carlo simulation
  enum CouplingFields
  {
    temperature,
    density_and_temperature,
    none
  };
}
