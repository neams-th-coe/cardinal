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

#include "AddOutputAction.h"

#include "CardinalEnums.h"
#include "OpenMCCellAverageProblem.h"

registerMooseAction("CardinalApp", SetupMGXSAction, "add_tallies");
registerMooseAction("CardinalApp", SetupMGXSAction, "add_filters");
registerMooseAction("CardinalApp", SetupMGXSAction, "add_aux_variable");
registerMooseAction("CardinalApp", SetupMGXSAction, "add_aux_kernel");
registerMooseAction("CardinalApp", SetupMGXSAction, "modify_outputs");

InputParameters
SetupMGXSAction::validParams()
{
  auto params = CardinalAction::validParams();
  params.addClassDescription("A class which sets up multi-group cross section generation using "
                             "Cardinal's mapped tallies.");
  params += EnergyBinBase::validParams();
  // MGXS' are always reversed.
  params.suppressParameter<bool>("reverse_bins");
  params.set<bool>("reverse_bins") = true;

  params.addRequiredParam<MooseEnum>(
      "tally_type", getTallyTypeEnum(), "The type of spatial tally to use");
  params.addRequiredParam<MooseEnum>("particle",
                                     getSingleParticleFilterEnum(),
                                     "The particle to filter for. At present cross sections can "
                                     "only be generated for neutrons or photons, if 'electron' or "
                                     "'positron' are selected an error will be thrown.");
  auto estimator_enum = getTallyEstimatorEnum();
  estimator_enum.assign("tracklength");
  params.addParam<MooseEnum>(
      "estimator",
      estimator_enum,
      "The type of estimator to use with the tallies added for MGXS generation. This is not "
      "applied to scattering / fission"
      " scores as the filters applied to those scores only support analog estimators.");
  params.addParam<bool>("check_tally_sum",
                        "Whether to check consistency between the local tallies "
                        "with a global tally sum");
  params.addParam<bool>(
      "normalize_by_global_tally",
      true,
      "Whether to normalize local tallies by a global tally (true) or else by the sum "
      "of the local tally (false)");

  // Options for MGXS generation. At a minimum, we generate multi-group total cross sections.
  params.addParam<bool>(
      "add_scattering",
      true,
      "Whether or not the scattering multi-group cross section matrix should be generated.");
  params.addParam<unsigned int>("legendre_order",
                                0,
                                "The order of the Legendre expansion in scattering angle to use "
                                "for generating scattering cross sections. Defaults to 0.");
  params.addParam<bool>("transport_correction",
                        true,
                        "Whether the in-group scattering cross section should include a P0 "
                        "transport correction or not.");

  params.addParam<bool>("add_fission",
                        false,
                        "Whether or not fission multi-group cross sections (neutron production and "
                        "the discrete chi spectrum) should be generated.");

  params.addParam<bool>(
      "add_fission_heating",
      false,
      "Whether or not per-group fission heating (kappa-fission) values should be generated.");

  params.addParam<bool>("add_inverse_velocity",
                        false,
                        "Whether or not per-group inverse velocities should be generated.");

  params.addParam<bool>(
      "add_diffusion_coefficient",
      false,
      "Whether or not per-group particle diffusion coefficients should be generated.");
  params.addParam<Real>("void_diffusion_coefficient",
                        1e3,
                        "The value the diffusion coefficient should take in a void region.");

  params.addParam<bool>(
      "add_absorption",
      false,
      "Whether or not absorption multi-group cross sections should be generated.");

  params.addParam<bool>("hide_tally_vars",
                        true,
                        "Whether or not tally variables used to compute multi-group cross sections "
                        "are hidden in exodus output.");

  return params;
}

SetupMGXSAction::SetupMGXSAction(const InputParameters & parameters)
  : CardinalAction(parameters),
    EnergyBinBase(this, parameters),
    _t_type(getParam<MooseEnum>("tally_type").getEnum<tally::TallyTypeEnum>()),
    _particle(getParam<MooseEnum>("particle")),
    _estimator(getParam<MooseEnum>("estimator")),
    _add_scattering(getParam<bool>("add_scattering")),
    _l_order(getParam<unsigned int>("legendre_order")),
    _transport_correction(getParam<bool>("transport_correction")),
    _add_fission(getParam<bool>("add_fission")),
    _add_kappa_fission(getParam<bool>("add_fission_heating")),
    _add_inv_vel(getParam<bool>("add_inverse_velocity")),
    _add_diffusion(getParam<bool>("add_diffusion_coefficient")),
    _void_diff(getParam<Real>("void_diffusion_coefficient")),
    _add_absorption(getParam<bool>("add_absorption")),
    _hide_tally_vars(getParam<bool>("hide_tally_vars")),
    _need_p1_scatter(false)
{
  if (_particle == "electron" || _particle == "positron")
    paramError("particle",
               "Multi-group cross sections can only be generated for neutrons or photons.");

  if (_add_fission && _particle == "photon")
    paramError("add_fission",
               "Multi-group fission cross sections / chi spectra cannot be "
               "generated when selecting particle = 'photon'! Please set 'add_fission = false' to "
               "continue.");

  if (_add_kappa_fission && _particle == "photon")
    paramError("add_fission_heating",
               "Multi-group fission heating values cannot be "
               "generated when selecting particle = 'photon'! Please set add_fission_heating = "
               "false to continue.");

  if (_t_type == tally::TallyTypeEnum::mesh && _estimator == "tracklength")
  {
    if (isParamSetByUser("estimator"))
      mooseWarning(
          "You've selected an unstructured mesh tally discretization for your multi-group cross "
          "sections and requested a tracklength estimator. Unstructured mesh tallies don't "
          "support, tracklength estimators, the estimator will be set to 'collision' instead.");

    _estimator.assign("collision");
  }

  if (_estimator != "analog" && (_add_fission || _add_scattering || _add_diffusion))
  {
    mooseWarning("You've requested the generation of scattering / fission / diffusion group "
                 "constants with a "
                 "non-analog estimator. At present these MGXS don't support 'collision' / "
                 "'tracklength' estimators, the estimator will be set to 'analog' instead.");

    _estimator.assign("analog");
  }

  if (_l_order > 0 && _transport_correction)
  {
    mooseWarning(
        "Transport-corrected scattering cross sections can only be used with isotropic scattering "
        "(l = 0). You've set l = " +
        Moose::stringify(_l_order) + ", the transport correction will not be applied.");
    _transport_correction = false;
  }

  if ((_transport_correction || _add_diffusion) && _l_order == 0)
    _need_p1_scatter = true;
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

  if (_current_task == "modify_outputs" && _hide_tally_vars)
    modifyOutputs();
}

OpenMCCellAverageProblem *
SetupMGXSAction::openmcProblem()
{
  auto p = dynamic_cast<OpenMCCellAverageProblem *>(_problem.get());
  if (!p)
    mooseError("MGXS can only be used with problems of the type 'OpenMCCellAverageProblem'! Please "
               "ensure you've added one in the [Problem] block.");
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
    params.set<MultiMooseEnum>("particles") =
        MultiMooseEnum(getParticleFilterEnums().getRawNames(), _particle, false);

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    openmcProblem()->addFilter("ParticleFilter", "MGXS_ParticleFilter", params);
  }

  // Need an EnergyOutFilter for scattering / neutron production cross sections.
  if (_add_scattering || _add_fission || _add_diffusion)
  {
    auto params = _factory.getValidParams("EnergyOutFilter");
    params.set<std::vector<Real>>("energy_boundaries") = _energy_bnds;
    params.set<bool>("reverse_bins") = true;

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    openmcProblem()->addFilter("EnergyOutFilter", "MGXS_EnergyOutFilter", params);
  }

  // Need an AngularLegendreFilter for scattering cross sections.
  if (_add_scattering || _add_diffusion)
  {
    auto params = _factory.getValidParams("AngularLegendreFilter");
    params.set<unsigned int>("order") = _need_p1_scatter ? 1 : _l_order;

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    openmcProblem()->addFilter("AngularLegendreFilter", "MGXS_AngularLegendreFilter", params);
  }
}

void
SetupMGXSAction::addTallies()
{
  // Determine the string form of the tally type.
  std::string tally_type;
  switch (_t_type)
  {
    case tally::TallyTypeEnum::cell:
      tally_type = "CellTally";
      break;

    case tally::TallyTypeEnum::mesh:
      tally_type = "MeshTally";
      break;

    default:
      mooseError("Internal error: Unhandled enum in 'tally::TallyTypeEnum'.");
      break;
  }

  // Total and flux tally.
  {
    auto params = _factory.getValidParams(tally_type);
    if (isParamValid("check_tally_sum"))
      params.set<bool>("check_tally_sum") = getParam<bool>("check_tally_sum");
    params.set<bool>("normalize_by_global_tally") = getParam<bool>("normalize_by_global_tally");
    params.set<MultiMooseEnum>("score") =
        MultiMooseEnum(getTallyScoreEnum().getRawNames(), "total flux", false);
    params.set<MooseEnum>("estimator") = _estimator;
    params.set<std::vector<std::string>>("name") = {std::string("mgxs_total"),
                                                    std::string("mgxs_flux")};
    params.set<std::vector<std::string>>("filters") = {std::string("MGXS_EnergyFilter"),
                                                       std::string("MGXS_ParticleFilter")};
    setObjectBlocks(params, _blocks);

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    _mgxs_tallies.push_back(
        openmcProblem()->addTally(tally_type, "MGXS_" + tally_type + "_Total_Flux", params).get());
  }

  // Scattering tally.
  if (_add_scattering || _add_diffusion)
  {
    auto params = _factory.getValidParams(tally_type);
    if (isParamValid("check_tally_sum"))
      params.set<bool>("check_tally_sum") = getParam<bool>("check_tally_sum");
    params.set<bool>("normalize_by_global_tally") = getParam<bool>("normalize_by_global_tally");
    params.set<MultiMooseEnum>("score") =
        MultiMooseEnum(getTallyScoreEnum().getRawNames(), "nu_scatter", false);
    params.set<MooseEnum>("estimator") = "analog";
    params.set<std::vector<std::string>>("name") = {std::string("mgxs_scatter")};
    params.set<std::vector<std::string>>("filters") = {std::string("MGXS_EnergyFilter"),
                                                       std::string("MGXS_EnergyOutFilter"),
                                                       std::string("MGXS_AngularLegendreFilter"),
                                                       std::string("MGXS_ParticleFilter")};
    setObjectBlocks(params, _blocks);

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    _mgxs_tallies.push_back(
        openmcProblem()->addTally(tally_type, "MGXS_" + tally_type + "_Scatter", params).get());
  }

  // Fission tally.
  if (_add_fission)
  {
    auto params = _factory.getValidParams(tally_type);
    if (isParamValid("check_tally_sum"))
      params.set<bool>("check_tally_sum") = getParam<bool>("check_tally_sum");
    params.set<bool>("normalize_by_global_tally") = getParam<bool>("normalize_by_global_tally");
    params.set<MultiMooseEnum>("score") =
        MultiMooseEnum(getTallyScoreEnum().getRawNames(), "nu_fission", false);
    params.set<MooseEnum>("estimator") = "analog";
    params.set<std::vector<std::string>>("name") = {std::string("mgxs_fission")};
    params.set<std::vector<std::string>>("filters") = {std::string("MGXS_EnergyFilter"),
                                                       std::string("MGXS_EnergyOutFilter"),
                                                       std::string("MGXS_ParticleFilter")};
    setObjectBlocks(params, _blocks);

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    _mgxs_tallies.push_back(
        openmcProblem()->addTally(tally_type, "MGXS_" + tally_type + "_Fission", params).get());
  }

  // Kappa-fission tally.
  if (_add_kappa_fission)
  {
    auto params = _factory.getValidParams(tally_type);
    if (isParamValid("check_tally_sum"))
      params.set<bool>("check_tally_sum") = getParam<bool>("check_tally_sum");
    params.set<bool>("normalize_by_global_tally") = getParam<bool>("normalize_by_global_tally");
    params.set<MultiMooseEnum>("score") =
        MultiMooseEnum(getTallyScoreEnum().getRawNames(), "kappa_fission", false);
    params.set<MooseEnum>("estimator") = _estimator;
    params.set<std::vector<std::string>>("name") = {std::string("mgxs_kappa_fission")};
    params.set<std::vector<std::string>>("filters") = {std::string("MGXS_EnergyFilter"),
                                                       std::string("MGXS_ParticleFilter")};
    setObjectBlocks(params, _blocks);

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    _mgxs_tallies.push_back(
        openmcProblem()
            ->addTally(tally_type, "MGXS_" + tally_type + "_Kappa_Fission", params)
            .get());
  }

  // Inverse velocity tally.
  if (_add_inv_vel)
  {
    auto params = _factory.getValidParams(tally_type);
    if (isParamValid("check_tally_sum"))
      params.set<bool>("check_tally_sum") = getParam<bool>("check_tally_sum");
    params.set<bool>("normalize_by_global_tally") = getParam<bool>("normalize_by_global_tally");
    params.set<MultiMooseEnum>("score") =
        MultiMooseEnum(getTallyScoreEnum().getRawNames(), "inverse_velocity", false);
    params.set<MooseEnum>("estimator") = _estimator;
    params.set<std::vector<std::string>>("name") = {std::string("mgxs_inverse_velocity")};
    params.set<std::vector<std::string>>("filters") = {std::string("MGXS_EnergyFilter"),
                                                       std::string("MGXS_ParticleFilter")};
    setObjectBlocks(params, _blocks);

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    _mgxs_tallies.push_back(
        openmcProblem()
            ->addTally(tally_type, "MGXS_" + tally_type + "_Inverse_velocity", params)
            .get());
  }

  // Absorption tally.
  if (_add_absorption)
  {
    auto params = _factory.getValidParams(tally_type);
    if (isParamValid("check_tally_sum"))
      params.set<bool>("check_tally_sum") = getParam<bool>("check_tally_sum");
    params.set<bool>("normalize_by_global_tally") = getParam<bool>("normalize_by_global_tally");
    params.set<MultiMooseEnum>("score") =
        MultiMooseEnum(getTallyScoreEnum().getRawNames(), "absorption", false);
    params.set<MooseEnum>("estimator") = _estimator;
    params.set<std::vector<std::string>>("name") = {std::string("mgxs_absorption")};
    params.set<std::vector<std::string>>("filters") = {std::string("MGXS_EnergyFilter"),
                                                       std::string("MGXS_ParticleFilter")};
    setObjectBlocks(params, _blocks);

    params.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmcProblem();
    _mgxs_tallies.push_back(
        openmcProblem()->addTally(tally_type, "MGXS_" + tally_type + "_Absorption", params).get());
  }
}

void
SetupMGXSAction::addAuxVars()
{
  // Total MGXE variables.
  for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
  {
    const std::string name = "total_xs_g" + Moose::stringify(g + 1);
    auto params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = "MONOMIAL";
    params.set<MooseEnum>("order") = "CONSTANT";
    setObjectBlocks(params, _blocks);

    openmcProblem()->checkDuplicateVariableName(name);
    _problem->addAuxVariable("MooseVariable", name, params);
  }

  // Scattering matrix MGXS variables.
  if (_add_scattering)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      for (unsigned int g_prime = 0; g_prime < _energy_bnds.size() - 1; ++g_prime)
      {
        for (unsigned int l = 0; l <= _l_order; ++l)
        {
          const std::string name = "scatter_xs_g" + Moose::stringify(g + 1) + "_gp" +
                                   Moose::stringify(g_prime + 1) + "_l" + Moose::stringify(l);
          auto params = _factory.getValidParams("MooseVariable");
          params.set<MooseEnum>("family") = "MONOMIAL";
          params.set<MooseEnum>("order") = "CONSTANT";
          setObjectBlocks(params, _blocks);

          openmcProblem()->checkDuplicateVariableName(name);
          _problem->addAuxVariable("MooseVariable", name, params);
        }
      }
    }
  }

  // Neutron production MGXS and discrete chi spectrum variables.
  if (_add_fission)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      {
        const std::string name = "nu_fission_xs_g" + Moose::stringify(g + 1);
        auto params = _factory.getValidParams("MooseVariable");
        params.set<MooseEnum>("family") = "MONOMIAL";
        params.set<MooseEnum>("order") = "CONSTANT";
        setObjectBlocks(params, _blocks);

        openmcProblem()->checkDuplicateVariableName(name);
        _problem->addAuxVariable("MooseVariable", name, params);
      }
      {
        const std::string name = "chi_g" + Moose::stringify(g + 1);
        auto params = _factory.getValidParams("MooseVariable");
        params.set<MooseEnum>("family") = "MONOMIAL";
        params.set<MooseEnum>("order") = "CONSTANT";
        setObjectBlocks(params, _blocks);

        openmcProblem()->checkDuplicateVariableName(name);
        _problem->addAuxVariable("MooseVariable", name, params);
      }
    }
  }

  // Discrete kappa-fission variables.
  if (_add_kappa_fission)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      const std::string name = "kappa_fission_g" + Moose::stringify(g + 1);
      auto params = _factory.getValidParams("MooseVariable");
      params.set<MooseEnum>("family") = "MONOMIAL";
      params.set<MooseEnum>("order") = "CONSTANT";
      setObjectBlocks(params, _blocks);

      openmcProblem()->checkDuplicateVariableName(name);
      _problem->addAuxVariable("MooseVariable", name, params);
    }
  }

  // Inverse velocity MGXS variables.
  if (_add_inv_vel)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      const std::string name = "inv_v_g" + Moose::stringify(g + 1);
      auto params = _factory.getValidParams("MooseVariable");
      params.set<MooseEnum>("family") = "MONOMIAL";
      params.set<MooseEnum>("order") = "CONSTANT";
      setObjectBlocks(params, _blocks);

      openmcProblem()->checkDuplicateVariableName(name);
      _problem->addAuxVariable("MooseVariable", name, params);
    }
  }

  // MG diffusion coefficients.
  if (_add_diffusion)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      const std::string name = "diff_g" + Moose::stringify(g + 1);
      auto params = _factory.getValidParams("MooseVariable");
      params.set<MooseEnum>("family") = "MONOMIAL";
      params.set<MooseEnum>("order") = "CONSTANT";
      setObjectBlocks(params, _blocks);

      openmcProblem()->checkDuplicateVariableName(name);
      _problem->addAuxVariable("MooseVariable", name, params);
    }
  }

  // Absorption MGXE variables.
  if (_add_absorption)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      const std::string name = "abs_xs_g" + Moose::stringify(g + 1);
      auto params = _factory.getValidParams("MooseVariable");
      params.set<MooseEnum>("family") = "MONOMIAL";
      params.set<MooseEnum>("order") = "CONSTANT";
      setObjectBlocks(params, _blocks);

      openmcProblem()->checkDuplicateVariableName(name);
      _problem->addAuxVariable("MooseVariable", name, params);
    }
  }
}

void
SetupMGXSAction::addAuxKernels()
{
  // Add auxkernels to compute the total MGXS'.
  for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
  {
    const auto n = "total_xs_g" + Moose::stringify(g + 1);
    auto params = _factory.getValidParams("ComputeMGXSAux");
    params.set<AuxVariableName>("variable") = n;
    params.set<std::vector<VariableName>>("rxn_rates")
        .emplace_back("mgxs_total_g" + Moose::stringify(g + 1) + "_" + std::string(_particle));
    params.set<std::vector<VariableName>>("normalize_by")
        .emplace_back("mgxs_flux_g" + Moose::stringify(g + 1) + "_" + std::string(_particle));
    setObjectBlocks(params, _blocks);

    _problem->addAuxKernel("ComputeMGXSAux", "comp_" + n, params);
  }

  // Add auxkernels to compute the elements of the MGXS scattering matrix.
  if (_add_scattering)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      for (unsigned int g_prime = 0; g_prime < _energy_bnds.size() - 1; ++g_prime)
      {
        // Handle within-group scattering cross sections separately for
        // transport-corrected P0 cross sections.
        if (g == g_prime && _transport_correction)
          continue;

        for (unsigned int l = 0; l <= _l_order; ++l)
        {
          const auto n = "scatter_xs_g" + Moose::stringify(g + 1) + "_gp" +
                         Moose::stringify(g_prime + 1) + "_l" + Moose::stringify(l);
          auto params = _factory.getValidParams("ComputeMGXSAux");
          params.set<AuxVariableName>("variable") = n;
          params.set<std::vector<VariableName>>("rxn_rates")
              .emplace_back("mgxs_scatter_g" + Moose::stringify(g + 1) + "_gp" +
                            Moose::stringify(g_prime + 1) + "_l" + Moose::stringify(l) + "_" +
                            std::string(_particle));
          params.set<std::vector<VariableName>>("normalize_by")
              .emplace_back("mgxs_flux_g" + Moose::stringify(g + 1) + "_" + std::string(_particle));
          setObjectBlocks(params, _blocks);

          _problem->addAuxKernel("ComputeMGXSAux", "comp_" + n, params);
        }
      }
    }

    if (_transport_correction)
    {
      for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
      {
        const auto n =
            "scatter_xs_g" + Moose::stringify(g + 1) + "_gp" + Moose::stringify(g + 1) + "_l0";
        auto params = _factory.getValidParams("ComputeTCScatterMGXSAux");
        params.set<AuxVariableName>("variable") = n;
        params.set<std::vector<VariableName>>("p0_scatter_rxn_rate")
            .emplace_back("mgxs_scatter_g" + Moose::stringify(g + 1) + "_gp" +
                          Moose::stringify(g + 1) + "_l0_" + std::string(_particle));
        for (unsigned int g_prime = 0; g_prime < _energy_bnds.size() - 1; ++g_prime)
          params.set<std::vector<VariableName>>("p1_scatter_rxn_rates")
              .emplace_back("mgxs_scatter_g" + Moose::stringify(g_prime + 1) + "_gp" +
                            Moose::stringify(g + 1) + "_l1_" + std::string(_particle));
        params.set<std::vector<VariableName>>("scalar_flux")
            .emplace_back("mgxs_flux_g" + Moose::stringify(g + 1) + "_" + std::string(_particle));
        setObjectBlocks(params, _blocks);

        _problem->addAuxKernel("ComputeTCScatterMGXSAux", "comp_" + n, params);
      }
    }
  }

  // Add auxkernels to compute the fission neutron production MGXS and the discrete chi spectrum.
  if (_add_fission)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      const auto n = "nu_fission_xs_g" + Moose::stringify(g + 1);
      auto params = _factory.getValidParams("ComputeMGXSAux");
      params.set<AuxVariableName>("variable") = n;
      for (unsigned int g_prime = 0; g_prime < _energy_bnds.size() - 1; ++g_prime)
      {
        params.set<std::vector<VariableName>>("rxn_rates")
            .emplace_back("mgxs_fission_g" + Moose::stringify(g + 1) + "_gp" +
                          Moose::stringify(g_prime + 1) + "_" + std::string(_particle));
      }
      params.set<std::vector<VariableName>>("normalize_by")
          .emplace_back("mgxs_flux_g" + Moose::stringify(g + 1) + "_" + std::string(_particle));
      setObjectBlocks(params, _blocks);

      _problem->addAuxKernel("ComputeMGXSAux", "comp_" + n, params);
    }

    std::vector<VariableName> all_fission;
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
      for (unsigned int g_prime = 0; g_prime < _energy_bnds.size() - 1; ++g_prime)
        all_fission.emplace_back("mgxs_fission_g" + Moose::stringify(g + 1) + "_gp" +
                                 Moose::stringify(g_prime + 1) + "_" + std::string(_particle));

    for (unsigned int g_prime = 0; g_prime < _energy_bnds.size() - 1; ++g_prime)
    {
      const auto n = "chi_g" + Moose::stringify(g_prime + 1);
      auto params = _factory.getValidParams("ComputeMGXSAux");
      params.set<AuxVariableName>("variable") = n;
      for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
      {
        params.set<std::vector<VariableName>>("rxn_rates")
            .emplace_back("mgxs_fission_g" + Moose::stringify(g + 1) + "_gp" +
                          Moose::stringify(g_prime + 1) + "_" + std::string(_particle));
      }
      params.set<std::vector<VariableName>>("normalize_by") = all_fission;
      setObjectBlocks(params, _blocks);

      _problem->addAuxKernel("ComputeMGXSAux", "comp_" + n, params);
    }
  }

  // Add auxkernels to compute a group-wise kappa fission.
  if (_add_kappa_fission)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      const auto n = "kappa_fission_g" + Moose::stringify(g + 1);
      auto params = _factory.getValidParams("ComputeMGXSAux");
      params.set<AuxVariableName>("variable") = n;
      params.set<std::vector<VariableName>>("rxn_rates")
          .emplace_back("mgxs_kappa_fission_g" + Moose::stringify(g + 1) + "_" +
                        std::string(_particle));
      params.set<std::vector<VariableName>>("normalize_by")
          .emplace_back("mgxs_flux_g" + Moose::stringify(g + 1) + "_" + std::string(_particle));
      setObjectBlocks(params, _blocks);

      _problem->addAuxKernel("ComputeMGXSAux", "comp_" + n, params);
    }
  }

  // Add auxkernels to compute a group-wise inverse velocity.
  if (_add_inv_vel)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      const auto n = "inv_v_g" + Moose::stringify(g + 1);
      auto params = _factory.getValidParams("ComputeMGXSAux");
      params.set<AuxVariableName>("variable") = n;
      params.set<std::vector<VariableName>>("rxn_rates")
          .emplace_back("mgxs_inverse_velocity_g" + Moose::stringify(g + 1) + "_" +
                        std::string(_particle));
      params.set<std::vector<VariableName>>("normalize_by")
          .emplace_back("mgxs_flux_g" + Moose::stringify(g + 1) + "_" + std::string(_particle));
      setObjectBlocks(params, _blocks);

      _problem->addAuxKernel("ComputeMGXSAux", "comp_" + n, params);
    }
  }

  // Add auxkernels to compute MG diffusion coefficients.
  if (_add_diffusion)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      const std::string n = "diff_g" + Moose::stringify(g + 1);
      auto params = _factory.getValidParams("ComputeDiffusionCoeffMGAux");
      params.set<AuxVariableName>("variable") = n;
      params.set<std::vector<VariableName>>("total_rxn_rate")
          .emplace_back("mgxs_total_g" + Moose::stringify(g + 1) + "_" + std::string(_particle));
      params.set<std::vector<VariableName>>("scalar_flux")
          .emplace_back("mgxs_flux_g" + Moose::stringify(g + 1) + "_" + std::string(_particle));
      params.set<Real>("void_diffusion_coefficient") = _void_diff;
      for (unsigned int g_prime = 0; g_prime < _energy_bnds.size() - 1; ++g_prime)
        params.set<std::vector<VariableName>>("p1_scatter_rxn_rates")
            .emplace_back("mgxs_scatter_g" + Moose::stringify(g_prime + 1) + "_gp" +
                          Moose::stringify(g + 1) + "_l1_" + std::string(_particle));
      setObjectBlocks(params, _blocks);

      _problem->addAuxKernel("ComputeDiffusionCoeffMGAux", "comp_" + n, params);
    }
  }

  // Add auxkernels to compute the absorption MGXS'.
  if (_add_absorption)
  {
    for (unsigned int g = 0; g < _energy_bnds.size() - 1; ++g)
    {
      const auto n = "abs_xs_g" + Moose::stringify(g + 1);
      auto params = _factory.getValidParams("ComputeMGXSAux");
      params.set<AuxVariableName>("variable") = n;
      params.set<std::vector<VariableName>>("rxn_rates")
          .emplace_back("mgxs_absorption_g" + Moose::stringify(g + 1) + "_" +
                        std::string(_particle));
      params.set<std::vector<VariableName>>("normalize_by")
          .emplace_back("mgxs_flux_g" + Moose::stringify(g + 1) + "_" + std::string(_particle));
      setObjectBlocks(params, _blocks);

      _problem->addAuxKernel("ComputeMGXSAux", "comp_" + n, params);
    }
  }
}

void
SetupMGXSAction::modifyOutputs()
{
  const auto & output_actions = _app.actionWarehouse().getActionListByName("add_output");
  for (const auto & act : output_actions)
  {
    // Extract the Output action.
    AddOutputAction * action = dynamic_cast<AddOutputAction *>(act);
    if (!action)
      continue;

    // Hide the tally variables.
    InputParameters & output_params = action->getObjectParams();
    if (output_params.have_parameter<std::vector<VariableName>>("hide"))
      for (const auto & t : _mgxs_tallies)
        for (const auto & v : t->getAuxVarNames())
          output_params.set<std::vector<VariableName>>("hide").emplace_back(v);
  }
}

#endif
