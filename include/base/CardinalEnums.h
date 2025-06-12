/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#pragma once

#include "MooseEnum.h"
#include "MultiMooseEnum.h"

MooseEnum getSynchronizationEnum();
MooseEnum getNekOrderEnum();
MooseEnum getBinnedVelocityComponentEnum();
MooseEnum getNekFieldEnum();
MooseEnum getOperationEnum();
MooseEnum getTallyTypeEnum();
MooseEnum getTallyEstimatorEnum();
MooseEnum getEigenvalueEnum();
MooseEnum getRelaxationEnum();
MooseEnum getTallyTriggerEnum();
MooseEnum getInitialPropertiesEnum();
MultiMooseEnum getTallyScoreEnum();
MultiMooseEnum getParticleFilterEnums();
MooseEnum getSingleTallyScoreEnum();
MooseEnum getSingleParticleFilterEnum();
MooseEnum getEnergyGroupsEnum();
MooseEnum getNekMeshEnum();
MooseEnum getFilterTypeEnum();
MooseEnum getStatsOutputEnum();

namespace nek_mesh
{

/**
 * Enumeration of possible meshes to operate upon within NekRS
 */
enum NekMeshEnum
{
  fluid,
  solid,
  all
};
} // namespace nek_mesh

namespace synchronization
{
/**
 * Enumeration of possible points at which to synchronize the external app's solution
 * to the mesh mirror
 */
enum SynchronizationEnum
{
  constant,
  parent_app
};
} //namespace synchronization

namespace order
{
/// Enumeration of possible surface order reconstructions for nekRS solution transfer
enum NekOrderEnum
{
  first,
  second
};
} // namespace order

namespace component
{
/// Directions in which to evaluate velocity
enum BinnedVelocityComponentEnum
{
  normal,
  user
};
} // namespace component

namespace field
{
/// Enumeration of possible fields to read from nekRS
enum NekFieldEnum
{
  velocity_x,
  velocity_y,
  velocity_z,
  velocity,
  velocity_component,
  velocity_x_squared,
  velocity_y_squared,
  velocity_z_squared,
  temperature,
  pressure,
  scalar01,
  scalar02,
  scalar03,
  unity,
  usrwrk00,
  usrwrk01,
  usrwrk02
};

/// Enumeration of possible fields to write in nekRS
enum NekWriteEnum
{
  flux,
  heat_source,
  x_displacement,
  y_displacement,
  z_displacement,
  mesh_velocity_x,
  mesh_velocity_y,
  mesh_velocity_z
};
} // namespace field

namespace output
{
/// Enumeration of possible fields to read from NekRS
enum NekOutputEnum
{
  velocity_x,
  velocity_y,
  velocity_z,
  temperature,
  pressure,
  scalar01,
  scalar02,
  scalar03
};
} // namespace output

namespace operation
{
/// Enumeration of possible operations to perform in global postprocessors
enum OperationEnum
{
  max,
  min,
  average
};
} // namespace operation

namespace tally
{
/// Type of tally to construct for the OpenMC model
enum TallyTypeEnum
{
  cell,
  mesh
};

/// Type of estimator to use for OpenMC tallies
enum TallyEstimatorEnum
{
  collision,
  tracklength,
  analog
};

enum TallyScoreEnum
{
  heating,
  heating_local,
  kappa_fission,
  fission_q_prompt,
  fission_q_recoverable,
  damage_energy,
  flux,
  H3_production
};
} // namespace tally

namespace trigger
{
/// Type of trigger to apply
enum TallyTriggerTypeEnum
{
  variance,
  std_dev,
  rel_err,
  none
};
} // namespace trigger

namespace energyfilter
{
/// Type of energy group structure to use
enum GroupStructureEnum
{
  CASMO_2,
  CASMO_4,
  CASMO_8,
  CASMO_16,
  CASMO_25,
  CASMO_40,
  VITAMINJ_42,
  SCALE_44,
  MPACT_51,
  MPACT_60,
  MPACT_69,
  CASMO_70,
  XMAS_172,
  VITAMINJ_175,
  SCALE_252,
  TRIPOLI_315,
  SHEM_361,
  CCFE_709,
  UKAEA_1102,
  ECCO_1968
};
} // namespace energyfilter

namespace coupling
{
/// Type of feedback in Monte Carlo simulation
enum CouplingFields
{
  temperature,
  density,
  density_and_temperature,
  none
};

/// Where to get the initial temperature and density settings for OpenMC
enum OpenMCInitialCondition
{
  hdf5,
  moose,
  xml
};
} // namespace coupling

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
} // namespace eigenvalue

namespace statistics
{
/// The type of output for both the k-eigenvalue and kinetics parameters.
enum OutputEnum
{
  Mean,
  StDev,
  RelError
};
} // namespace statistics

namespace relaxation
{
/// Type of relaxation
enum RelaxationEnum
{
  constant,
  robbins_monro,
  dufek_gudowski,
  none
};
} // namespace relaxation

enum OpenMCFilterType
{
  cell,
  material,
  universe,
  mesh,
  none
};
