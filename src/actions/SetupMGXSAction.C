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

#ifdef ENABLE_OPENMC_COUPLING

#include "SetupMGXSAction.h"

#include "CardinalEnums.h"

registerMooseAction("CardinalApp", SetupMGXSAction, "add_tallies");
registerMooseAction("CardinalApp", SetupMGXSAction, "add_filters");
registerMooseAction("CardinalApp", SetupMGXSAction, "add_aux_variable");
registerMooseAction("CardinalApp", SetupMGXSAction, "add_aux_kernel");

InputParameters
SetupMGXSAction::validParams()
{
  auto params = CardinalAction::validParams();
  params.addClassDescription("A class which sets up multi-group cross section generation using Cardinal's mapped tallies.");
  params += OpenMCBase::validParams();
  params += EnergyBinBase::validParams();
  // MGXS' are always reversed.
  params.suppressParameter<bool>("reverse_bins");
  params.set<bool>("reverse_bins") = true;

  params.addRequiredParam<MooseEnum>(
      "tally_type",
      getTallyTypeEnum(),
      "The type of spatial tally to use. Options are a distributed cell tally (cell) or an unstructured mesh tally (mesh).");
  params.addRequiredParam<MooseEnum>(
      "particle", getSingleParticleFilterEnum(), "The particle to filter for.");
  params.addParam<unsigned int>(
      "legendre_order",
      0,
      "The order of the Legendre expansion in scattering angle to use for generating scattering cross sections. Defaults to 0.");

  // Options for MGXS generation. At a minimum, we generate multi-group total cross sections.
  params.addParam<bool>(
      "add_scattering",
      true,
      "Whether or not the multi-group scattering cross section matrix should be generated.");
  params.addParam<bool>(
      "add_fission",
      false,
      "Whether or not fission cross sections (neutron production and the discrete chi spectrum) should be generated.");
  params.addParam<bool>(
      "add_fission_heating",
      false,
      "Whether or not per-group fission heating (kappa-fission) values should be generated.");
  params.addParam<bool>(
      "add_inverse_velocity",
      false,
      "Whether or not per-group inverse velocities (kappa-fission) should be generated.");

  return params;
}

SetupMGXSAction::SetupMGXSAction(const InputParameters & parameters)
  : CardinalAction(parameters),
    OpenMCBase(this, parameters),
    EnergyBinBase(this, parameters)
{

}

void
SetupMGXSAction::act()
{
  if (_current_task == "add_filters")
    addFilters();

  if (_current_task == "add_tallies")
    addTallies();

  if (_current_task == "add_aux_variable")
    addAuxVars();

  if (_current_task == "add_aux_kernel")
    addAuxKernels();
}

void
SetupMGXSAction::addFilters()
{

}

void
SetupMGXSAction::addTallies()
{

}

void
SetupMGXSAction::addAuxVars()
{

}

void
SetupMGXSAction::addAuxKernels()
{

}

#endif
