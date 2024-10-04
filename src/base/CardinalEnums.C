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
      "velocity_component velocity_x velocity_y velocity_z velocity temperature pressure scalar01 scalar02 scalar03 unity");
}

MooseEnum
getOperationEnum()
{
  return MooseEnum("max min average", "max");
}

MooseEnum
getTallyTypeEnum()
{
  return MooseEnum("cell mesh none");
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
  return MultiMooseEnum("heating heating_local kappa_fission fission_q_prompt fission_q_recoverable damage_energy flux H3_production total absorption scatter fission");
}

MooseEnum
getEnergyGroupsEnum()
{
  return MooseEnum(
      "CASMO_2 CASMO_4 CASMO_8 CASMO_16 CASMO_25 CASMO_40 VITAMIN_J_42 SCALE_44 MPACT_51 MPACT_60 "
      "MPACT_69 CASMO_70 XMAS_172 VITAMIN_J_175 SCALE_252 TRIPOLI_315 SHEM_361 CCFE_709 UKAEA_1102 "
      "ECCO_1968");
}
