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

#include "CriticalitySearchBase.h"
#include "BrentsMethod.h"
#include "openmc/eigenvalue.h"

InputParameters
CriticalitySearchBase::validParams()
{
  auto params = MooseObject::validParams();
  params.addRequiredParam<Real>("minimum", "Minimum for values to search over");
  params.addRequiredParam<Real>("maximum", "Maximum for values to search over");
  params.addRangeCheckedParam<Real>("tolerance", 1e-3, "tolerance > 0", "Absolute tolerance to converge multiplication factor");
  params.addClassDescription("Base class for defining parameters used in a criticality search in OpenMC.");
  params.registerBase("CriticalitySearch");
  params.registerSystemAttributeName("CriticalitySearch");
  params.addPrivateParam<OpenMCCellAverageProblem *>("_openmc_problem");
  return params;
}

CriticalitySearchBase::CriticalitySearchBase(const InputParameters & parameters)
  : MooseObject(parameters),
    _maximum(getParam<Real>("maximum")),
    _minimum(getParam<Real>("minimum")),
    _tolerance(getParam<Real>("tolerance")),
    _openmc_problem(*getParam<OpenMCCellAverageProblem *>("_openmc_problem"))
{
  if (_minimum >= _maximum)
    paramError("minimum", "The 'minimum' value must be less than the 'maximum' value");

  auto pp_params = _factory.getValidParams("KEigenvalue");
  _openmc_problem.addPostprocessor("KEigenvalue", "k", pp_params);
}

void
CriticalitySearchBase::searchForCriticality()
{
  _console << "Running criticality search in OpenMC for " << quantity() << " in range " << std::to_string(_minimum) << " - " << std::to_string(_maximum) << std::endl;

  std::function <Real(Real)> func;
  func = [this](Real x) {
    updateOpenMCModel(x);
    int err = openmc_run();
    if (err)
      mooseError(openmc_err_msg);

    int n = openmc::simulation::n_realizations;
    const auto & gt = openmc::simulation::global_tallies;

    if (n <= 3)
      mooseError("Cannot compute combined k-effective estimate with fewer than 4 realizations!\n"
        "Please change the 'value_type' to either 'collision', 'tracklength', or 'absorption'.");

    double k_eff[2];
    openmc::openmc_get_keff(k_eff);
    std::cout << k_eff[0] << std::endl;
    return k_eff[0] - 1.0;
  };

  BrentsMethod::root(func, _minimum, _maximum, _tolerance);
}

#endif
