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
MooseEnum getNekMeshEnum();

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
  velocity_component,
  velocity_x,
  velocity_y,
  velocity_z,
  velocity,
  temperature,
  pressure,
  scalar01,
  scalar02,
  scalar03,
  wall_shear,
  traction,
//  traction_x,
//  traction_y,
//  traction_z,
  ros_s11,
  ros_s22,
  ros_s33,
  ros_s12,
  ros_s23,
  ros_s13,
  unity
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

namespace operation
{
/// Enumeration of possible operations to perform in global postprocessors
enum OperationEnum
{
  max,
  min
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

/// Type of trigger to apply
enum TallyTriggerTypeEnum
{
  variance,
  std_dev,
  rel_err,
  none
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

namespace coupling
{
/// Type of feedback in Monte Carlo simulation
enum CouplingFields
{
  temperature,
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

namespace channel_type
{
/// Type of subchannel
enum ChannelTypeEnum
{
  interior,
  edge,
  corner
};
} // namespace channel_type

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
