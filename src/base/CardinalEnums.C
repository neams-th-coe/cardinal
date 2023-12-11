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

#include "CardinalEnums.h"

MooseEnum
getNekMeshEnum()
{
  return MooseEnum("fluid solid all", "all");
}

MooseEnum
getSynchronizationEnum()
{
  return MooseEnum("constant parent_app", "constant");
}

MooseEnum
getNekOrderEnum()
{
  return MooseEnum("first second", "first");
}

MooseEnum
getBinnedVelocityComponentEnum()
{
  return MooseEnum("normal user");
}

MooseEnum
getNekFieldEnum()
{
  return MooseEnum(
      "velocity_component velocity_x velocity_y velocity_z velocity temperature"
      " pressure scalar01 scalar02 scalar03 wall_shear traction traction_x traction_y traction_z"
      " ros_s11 ros_s22 ros_s33 ros_s12 ros_s23 ros_s13 unity");
}

MooseEnum
getOperationEnum()
{
  return MooseEnum("max min", "max");
}

MooseEnum
getTallyTypeEnum()
{
  return MooseEnum("cell mesh");
}

MooseEnum
getTallyEstimatorEnum()
{
  return MooseEnum("collision tracklength analog");
}

MooseEnum
getEigenvalueEnum()
{
  return MooseEnum("collision absorption tracklength combined", "combined");
}

MooseEnum
getRelaxationEnum()
{
  return MooseEnum("constant robbins_monro dufek_gudowski none", "none");
}

MooseEnum
getTallyTriggerEnum()
{
  return MooseEnum("variance std_dev rel_err none", "none");
}

MooseEnum
getInitialPropertiesEnum()
{
  return MooseEnum("hdf5 moose xml", "moose");
}

MultiMooseEnum
getTallyScoreEnum()
{
  return MultiMooseEnum("heating heating_local kappa_fission fission_q_prompt fission_q_recoverable damage_energy flux H3_production");
}
