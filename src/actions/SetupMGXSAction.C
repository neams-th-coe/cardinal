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
#include "OpenMCCellAverageProblem.h"

registerMooseAction("CardinalApp", SetupMGXSAction, "add_tallies");
registerMooseAction("CardinalApp", SetupMGXSAction, "add_filters");
registerMooseAction("CardinalApp", SetupMGXSAction, "add_aux_variable");
registerMooseAction("CardinalApp", SetupMGXSAction, "add_aux_kernel");

InputParameters
SetupMGXSAction::validParams()
{
  auto params = CardinalAction::validParams();
  params.addClassDescription("A class which sets up multi-group cross section generation using Cardinal's mapped tallies.");
  params += EnergyBinBase::validParams();
  // MGXS' are always reversed.
  params.suppressParameter<bool>("reverse_bins");
  params.set<bool>("reverse_bins") = true;

  params.addRequiredParam<MooseEnum>(
      "tally_type",
      getTallyTypeEnum(),
      "The type of spatial tally to use. Options are a distributed cell tally (cell) or an unstructured mesh tally (mesh).");
  params.addRequiredParam<MooseEnum>(
      "particle",
      getSingleParticleFilterEnum(),
      "The particle to filter for. At present cross sections can only be generated for neutrons or photons, if 'electron' or "
      "'positron' are selected an error will be thrown.");
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
    EnergyBinBase(this, parameters),
    _t_type(getParam<MooseEnum>("tally_type").getEnum<tally::TallyTypeEnum>()),
    _particle(getParam<MooseEnum>("particle")),
    _l_order(getParam<unsigned int>("legendre_order")),
    _add_scattering(getParam<bool>("add_scattering")),
    _add_fission(getParam<bool>("add_fission")),
    _add_kappa_fission(getParam<bool>("add_fission_heating")),
    _add_inv_vel(getParam<bool>("add_inverse_velocity"))
{
  if (_particle == "electron" || _particle == "positron")
    paramError("particle", "At present, multi-group cross sections can only be generated for neutrons or photons.");

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

OpenMCCellAverageProblem *
SetupMGXSAction::openmcProblem()
{
  auto p = dynamic_cast<OpenMCCellAverageProblem *>(_problem.get());
  if (!p)
    mooseError("MGXS can only be used with problems of the type 'OpenMCCellAverageProblem'! Please ensure you've added one in the [Problem] block.");
  return p;
}

void
SetupMGXSAction::addFilters()
{
  // Need an EnergyFilter for all cross sections.
  {
    auto params = _factory.getValidParams("EnergyFilter");
    params.set<std::vector<Real>>("energy_boundaries") = _energy_bnds;
    params.set<bool>("reverse_bins") = true;

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    openmcProblem()->addFilter("EnergyFilter", "MGXS_EnergyFilter", params);
  }

  // Need a ParticleFilter for all cross sections.
  {
    auto params = _factory.getValidParams("ParticleFilter");
    params.set<MultiMooseEnum>("particles") = MultiMooseEnum(getParticleFilterEnums().getRawNames(), _particle, false);

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    openmcProblem()->addFilter("ParticleFilter", "MGXS_ParticleFilter", params);
  }

  // Need an EnergyOutFilter for scattering / neutron production cross sections.
  if (_add_scattering || _add_fission)
  {
    auto params = _factory.getValidParams("EnergyOutFilter");
    params.set<std::vector<Real>>("energy_boundaries") = _energy_bnds;
    params.set<bool>("reverse_bins") = true;

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    openmcProblem()->addFilter("EnergyOutFilter", "MGXS_EnergyOutFilter", params);
  }

  // Need an AngularLegendreFilter for scattering cross sections with a Legendre order greater than 0.
  if (_add_scattering && _l_order > 0)
  {
    auto params = _factory.getValidParams("AngularLegendreFilter");
    params.set<unsigned int>("order") = _l_order;

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    openmcProblem()->addFilter("AngularLegendreFilter", "MGXS_AngularLegendreFilter", params);
  }
}

void
SetupMGXSAction::addTallies()
{
  switch (_t_type)
  {
    case tally::TallyTypeEnum::cell:
      // Total and flux tally.
      {
        auto params = _factory.getValidParams("CellTally");
        params.set<MultiMooseEnum>("score") = MultiMooseEnum(getTallyScoreEnum().getRawNames(), "total flux", false);
        params.set<MooseEnum>("estimator") = MooseEnum(getTallyEstimatorEnum().getRawNames(), "analog", false);
        params.set<std::vector<std::string>>("name") = { std::string("mgxs_total"), std::string("mgxs_flux") };
        params.set<std::vector<std::string>>("filters") = { std::string("MGXS_EnergyFilter"), std::string("MGXS_ParticleFilter") };
        setObjectBlocks(params, _blocks);

        params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
        openmcProblem()->addTally("CellTally", "MGXS_CellTally_Total_Flux", params);
      }
      // Scattering tally.
      if (_add_scattering)
      {
        auto params = _factory.getValidParams("CellTally");
        params.set<MultiMooseEnum>("score") = MultiMooseEnum(getTallyScoreEnum().getRawNames(), "scatter", false);
        params.set<MooseEnum>("estimator") = MooseEnum(getTallyEstimatorEnum().getRawNames(), "analog", false);
        params.set<std::vector<std::string>>("name") = { std::string("mgxs_scatter") };
        params.set<std::vector<std::string>>("filters") =
          { std::string("MGXS_EnergyFilter"), std::string("MGXS_EnergyOutFilter"), std::string("MGXS_ParticleFilter") };
        if (_l_order > 0)
          params.set<std::vector<std::string>>("filters").emplace_back("MGXS_AngularLegendreFilter");
        setObjectBlocks(params, _blocks);

        params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
        openmcProblem()->addTally("CellTally", "MGXS_CellTally_Scatter", params);
      }
      // Fission tally.
      if (_add_fission)
      {
        auto params = _factory.getValidParams("CellTally");
        params.set<MultiMooseEnum>("score") = MultiMooseEnum(getTallyScoreEnum().getRawNames(), "fission", false);
        params.set<MooseEnum>("estimator") = MooseEnum(getTallyEstimatorEnum().getRawNames(), "analog", false);
        params.set<std::vector<std::string>>("name") = { std::string("mgxs_fission") };
        params.set<std::vector<std::string>>("filters") =
          { std::string("MGXS_EnergyFilter"), std::string("MGXS_EnergyOutFilter"), std::string("MGXS_ParticleFilter") };
        setObjectBlocks(params, _blocks);

        params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
        openmcProblem()->addTally("CellTally", "MGXS_CellTally_Fission", params);
      }
      // Kappa-fission tally. Multi-group kappa fissions segfault when used for normalization??????????
      // TODO: figure out what's going on and fix it.
      if (_add_kappa_fission)
      {
        auto params = _factory.getValidParams("CellTally");
        params.set<MultiMooseEnum>("score") = MultiMooseEnum(getTallyScoreEnum().getRawNames(), "kappa_fission", false);
        params.set<MooseEnum>("estimator") = MooseEnum(getTallyEstimatorEnum().getRawNames(), "analog", false);
        params.set<std::vector<std::string>>("name") = { std::string("mgxs_kappa_fission") };
        params.set<std::vector<std::string>>("filters") = { std::string("MGXS_EnergyFilter"), std::string("MGXS_ParticleFilter") };
        setObjectBlocks(params, _blocks);

        params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
        openmcProblem()->addTally("CellTally", "MGXS_CellTally_Kappa_Fission", params);
      }
      // Inverse velocity tally.
      if (_add_inv_vel)
      {
        auto params = _factory.getValidParams("CellTally");
        params.set<MultiMooseEnum>("score") = MultiMooseEnum(getTallyScoreEnum().getRawNames(), "inverse_velocity", false);
        params.set<MooseEnum>("estimator") = MooseEnum(getTallyEstimatorEnum().getRawNames(), "analog", false);
        params.set<std::vector<std::string>>("name") = { std::string("mgxs_inverse_velocity") };
        params.set<std::vector<std::string>>("filters") = { std::string("MGXS_EnergyFilter"), std::string("MGXS_ParticleFilter") };
        setObjectBlocks(params, _blocks);

        params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
        openmcProblem()->addTally("CellTally", "MGXS_CellTally_Inverse_velocity", params);
      }
      break;

    case tally::TallyTypeEnum::mesh:
      // Total and flux tally.
      {
        auto params = _factory.getValidParams("MeshTally");
        params.set<MultiMooseEnum>("score") = MultiMooseEnum(getTallyScoreEnum().getRawNames(), "total flux", false);
        params.set<MooseEnum>("estimator") = MooseEnum(getTallyEstimatorEnum().getRawNames(), "analog", false);
        params.set<std::vector<std::string>>("name") = { std::string("mgxs_total"), std::string("mgxs_flux") };
        params.set<std::vector<std::string>>("filters") = { std::string("MGXS_EnergyFilter"), std::string("MGXS_ParticleFilter") };
        setObjectBlocks(params, _blocks);

        params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
        openmcProblem()->addTally("MeshTally", "MGXS_MeshTally_Total_Flux", params);
      }
      // Scattering tally.
      if (_add_scattering)
      {
        auto params = _factory.getValidParams("MeshTally");
        params.set<MultiMooseEnum>("score") = MultiMooseEnum(getTallyScoreEnum().getRawNames(), "scatter", false);
        params.set<MooseEnum>("estimator") = MooseEnum(getTallyEstimatorEnum().getRawNames(), "analog", false);
        params.set<std::vector<std::string>>("name") = { std::string("mgxs_scatter") };
        params.set<std::vector<std::string>>("filters") =
          { std::string("MGXS_EnergyFilter"), std::string("MGXS_EnergyOutFilter"), std::string("MGXS_ParticleFilter") };
        if (_l_order > 0)
          params.set<std::vector<std::string>>("filters").emplace_back("MGXS_AngularLegendreFilter");
        setObjectBlocks(params, _blocks);

        params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
        openmcProblem()->addTally("MeshTally", "MGXS_MeshTally_Scatter", params);
      }
      // Fission tally.
      if (_add_fission)
      {
        auto params = _factory.getValidParams("MeshTally");
        params.set<MultiMooseEnum>("score") = MultiMooseEnum(getTallyScoreEnum().getRawNames(), "fission", false);
        params.set<MooseEnum>("estimator") = MooseEnum(getTallyEstimatorEnum().getRawNames(), "analog", false);
        params.set<std::vector<std::string>>("name") = { std::string("mgxs_fission") };
        params.set<std::vector<std::string>>("filters") =
          { std::string("MGXS_EnergyFilter"), std::string("MGXS_EnergyOutFilter"), std::string("MGXS_ParticleFilter") };
        setObjectBlocks(params, _blocks);

        params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
        openmcProblem()->addTally("MeshTally", "MGXS_MeshTally_Fission", params);
      }
      // Kappa-fission tally. Multi-group kappa fissions segfault when used for normalization??????????
      // TODO: figure out what's going on and fix it.
      if (_add_kappa_fission)
      {
        auto params = _factory.getValidParams("MeshTally");
        params.set<MultiMooseEnum>("score") = MultiMooseEnum(getTallyScoreEnum().getRawNames(), "kappa_fission", false);
        params.set<MooseEnum>("estimator") = MooseEnum(getTallyEstimatorEnum().getRawNames(), "analog", false);
        params.set<std::vector<std::string>>("name") = { std::string("mgxs_kappa_fission") };
        params.set<std::vector<std::string>>("filters") = { std::string("MGXS_EnergyFilter"), std::string("MGXS_ParticleFilter") };
        setObjectBlocks(params, _blocks);

        params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
        openmcProblem()->addTally("MeshTally", "MGXS_MeshTally_Kappa_Fission", params);
      }
      // Inverse velocity tally.
      if (_add_inv_vel)
      {
        auto params = _factory.getValidParams("MeshTally");
        params.set<MultiMooseEnum>("score") = MultiMooseEnum(getTallyScoreEnum().getRawNames(), "inverse_velocity", false);
        params.set<MooseEnum>("estimator") = MooseEnum(getTallyEstimatorEnum().getRawNames(), "analog", false);
        params.set<std::vector<std::string>>("name") = { std::string("mgxs_inverse_velocity") };
        params.set<std::vector<std::string>>("filters") = { std::string("MGXS_EnergyFilter"), std::string("MGXS_ParticleFilter") };
        setObjectBlocks(params, _blocks);

        params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
        openmcProblem()->addTally("MeshTally", "MGXS_MeshTally_Inverse_velocity", params);
      }
      break;

    default:
      mooseError("Internal error: Unhandled enum in 'tally::TallyTypeEnum'.");
      break;
  }
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
