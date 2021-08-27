#pragma once

#include "MooseEnum.h"

MooseEnum getNekOrderEnum();
MooseEnum getNekFieldEnum();
MooseEnum getOperationEnum();
MooseEnum getTallyTypeEnum();
MooseEnum getEigenvalueEnum();

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
  /// Enumeration of possible fields to read from nekRS
  enum NekFieldEnum
  {
    velocity_x,
    velocity_y,
    velocity_z,
    velocity,
    temperature,
    pressure,
    unity
  };

  /// Enumeration of possible fields to write in nekRS
  enum NekWriteEnum
  {
    flux,
    heat_source,
    x_displacement,
    y_displacement,
    z_displacement
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

namespace eigenvalue
{
  /// Type of OpenMC k-eigenvalue global tally
  enum EigenvalueEnum
  {
    collision,
    absorption,
    tracklength,
    combined
  };
}
