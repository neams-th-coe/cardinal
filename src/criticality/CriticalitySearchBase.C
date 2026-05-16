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
#include "UserErrorChecking.h"
#include "VariadicTable.h"
#include "BrentsMethod.h"

// To disable tallies
#include "openmc/tallies/tally.h"

InputParameters
CriticalitySearchBase::validParams()
{
  auto params = MooseObject::validParams();
  params += OpenMCBase::validParams();
  params.addRequiredParam<Real>(
      "minimum",
      "Minimum for values to search over; the root must occur at a value greater than the minimum");
  params.addRequiredParam<Real>(
      "maximum",
      "Maximum for values to search over; the root must occur at a value smaller than the maximum");
  params.addRangeCheckedParam<Real>(
      "target", 1.0, "target > 0.0", "Target value of k effective to search for");
  params.addRequiredRangeCheckedParam<Real>(
      "root_tol",
      "root_tol > 0",
      "Absolute tolerance to converge root that yields target value of the multiplication factor.");
  params.addRangeCheckedParam<Real>(
      "k_tol",
      1e-3,
      "k_tol > 0",
      "Absolute tolerance to converge multiplication factor; be aware that if too few particles "
      "are used, statistical noise may require many criticality calculations to converge.");
  params.addParam<MooseEnum>(
      "estimator", getEigenvalueEnum(), "Type of eigenvalue estimator to use");
  params.addParam<bool>(
      "run_critical_state",
      true,
      "Whether a final k-eigenvalue calculation should be performed on the critical model state.");
  params.addParam<bool>(
      "tally_during_search",
      false,
      "Whether non-eigenvalue tallies should be disabled during the search process as a "
      "performance optimization. If set to 'false', `run_critical_state` must be set to 'true' "
      "to ensure tallies are computed on the last iteration.");

  params.addClassDescription(
      "Base class for defining parameters used in a criticality search in OpenMC.");
  params.registerBase("CriticalitySearch");
  params.registerSystemAttributeName("CriticalitySearch");
  params.addPrivateParam<OpenMCCellAverageProblem *>("_openmc_problem");
  return params;
}

CriticalitySearchBase::CriticalitySearchBase(const InputParameters & parameters)
  : MooseObject(parameters),
    OpenMCBase(this, parameters),
    _maximum(getParam<Real>("maximum")),
    _minimum(getParam<Real>("minimum")),
    _k_tol(getParam<Real>("k_tol")),
    _root_tol(getParam<Real>("root_tol")),
    _estimator(getParam<MooseEnum>("estimator").getEnum<eigenvalue::EigenvalueEnum>()),
    _run_critical_state(getParam<bool>("run_critical_state")),
    _tally_during_search(getParam<bool>("tally_during_search")),
    _target(getParam<Real>("target"))
{
  if (_minimum >= _maximum)
    paramError("minimum",
               "The 'minimum' value (" + std::to_string(_minimum) +
                   ") must be less than the 'maximum' value (" + std::to_string(_maximum) + ").");

  if (!_tally_during_search && !_run_critical_state)
    paramError(
        "tally_during_search",
        "When disabling tallies during intermediate calculations in the criticality search, "
        "you must set 'run_critical_state' to 'true'! This ensures you get tallies on the final "
        "iteration.");

  auto pp_params = _factory.getValidParams("Receiver");
  _openmc_problem->addPostprocessor("Receiver", _pp_name, pp_params);
}

void
CriticalitySearchBase::searchForCriticality(std::function<void()> step_callback)
{
  _console << "Running criticality search in OpenMC for " << quantity() << " in range "
           << std::to_string(_minimum) << " - " << std::to_string(_maximum) << " " << units() << " "
           << std::endl;

  VariadicTable<int, Real, Real, Real> vt(
      {"Iteration", quantity() + " " + units(), "  k (mean)  ", " k (std dev) "});
  vt.setColumnFormat({VariadicTableColumnFormat::AUTO,
                      VariadicTableColumnFormat::SCIENTIFIC,
                      VariadicTableColumnFormat::SCIENTIFIC,
                      VariadicTableColumnFormat::SCIENTIFIC});

  std::function<Real(Real)> func;
  func = [&](Real x)
  {
    // update the OpenMC model with a new parameter
    updateOpenMCModel(x);
    _inputs.push_back(x);

    // Execute the callback after updating the model prior to running the OpenMC problem.
    // This is used by OpenMCCellAverageProblem to update the MOOSE->OpenMC coupling, where
    // we need to ensure the correct temperatures and densities are applied to OpenMC cells
    // to maintain a critical state on the final solve. This also ensures that cell tallies
    // are extracted using the correct cell->element maps.
    step_callback();

    if (!_tally_during_search)
    {
      _console << "Disabling tallies" << std::endl;
      for (auto & t : openmc::model::tallies)
        t->set_active(false);
    }

    // re-run the model
    int err = openmc_run();
    if (err)
      mooseError(openmc_err_msg);

    // fetch k and print values to console
    Real k = kMean(_estimator);
    Real k_std_dev = kStandardDeviation(_estimator);
    _k_values.push_back(k);
    _k_std_dev_values.push_back(k_std_dev);

    vt.addRow(_k_values.size() - 1, x, k, k_std_dev);
    vt.print(_console);

    if (_k_tol < 3 * k_std_dev)
      mooseDoOnce(mooseWarning(
          "The 'k_tol' for the criticality search (" + std::to_string(_k_tol) +
          ") is smaller than 3-sigma standard deviation in k (" + std::to_string(3 * k_std_dev) +
          "), which may require many search iterations to converge to this tolerance "
          "Consider a looser 'k_tol' or increase the number of particles."));

    return k - _target;
  };

  BrentsMethod::root(func, _minimum, _maximum, _root_tol);

  // check if the method converged
  if (abs(kMean(_estimator) - _target) >= _k_tol)
    mooseWarning("The eigenvalue produced by the criticality search was not within "
                 "specified 'k_tol' of the target! This could occur if 'k_tol' is too "
                 "tight for simulation's statistical error. It can also occur if "
                 "'root_tol' is too loose and the worth curve is steep near the target");

  // Run the critical state calculation, if requested.
  if (_run_critical_state)
  {
    _console << "Running the critical state calculation" << std::endl;
    if (!_tally_during_search)
    {
      _console << "Re-enabling tallies" << std::endl;
      for (auto & t : openmc::model::tallies)
        t->set_active(true);
    }

    // Run the critical state model.
    int err = openmc_run();
    if (err)
      mooseError(openmc_err_msg);
  }

  // fill the converged value into a postprocessor
  _openmc_problem->setPostprocessorValueByName(_pp_name, _inputs.back());
}

#endif
