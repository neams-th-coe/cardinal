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

#ifdef ENABLE_NEK_COUPLING
#include "DimensionalizeAction.h"
#include "NekRSProblemBase.h"
#include "NekInterface.h"
#include "VariadicTable.h"
#include "NekRSMesh.h"

registerMooseAction("CardinalApp", DimensionalizeAction, "add_dimensionalization");

InputParameters
DimensionalizeAction::validParams()
{
  auto params = Action::validParams();
  params.addClassDescription("Defines how to dimensionalize the NekRS fields when accessing from "
                             "MOOSE as AuxVariables, in Postprocessors, in UserObjects, etc.");

  params.addRangeCheckedParam<Real>("U", 1.0, "U > 0.0", "Reference velocity");
  params.addRangeCheckedParam<Real>("L", 1.0, "L > 0.0", "Reference length");
  params.addRangeCheckedParam<Real>("rho", 1.0, "rho > 0.0", "Reference density");
  params.addRangeCheckedParam<Real>("Cp", 1.0, "Cp > 0.0", "Reference isobaric specific heat");

  // for passive scalars, these are typically dimensionalized as (T - T0) / dT
  params.addRangeCheckedParam<Real>("T", 0.0, "T >= 0.0", "Reference temperature");
  params.addRangeCheckedParam<Real>("dT", 1.0, "dT > 0.0", "Reference temperature difference");
  return params;
}

DimensionalizeAction::DimensionalizeAction(const InputParameters & parameters)
  : Action(parameters),
    _U(getParam<Real>("U")),
    _T(getParam<Real>("T")),
    _dT(getParam<Real>("dT")),
    _L(getParam<Real>("L")),
    _rho(getParam<Real>("rho")),
    _Cp(getParam<Real>("Cp"))
{
  // inform NekRS of the scaling that we are using; the NekInterface holds all
  // the reference scales and provides accessor methods
  nekrs::initializeDimensionalScales(_U, _T, _dT, _L, _rho, _Cp);
}

void
DimensionalizeAction::act()
{
  if (_current_task == "add_dimensionalization")
  {
    auto nek_problem = dynamic_cast<NekRSProblemBase *>(_problem.get());

    if (!nek_problem)
      mooseError("The [Dimensionalize] block can only be used with wrapped Nek cases! "
                 "You need to change the [Problem] block to a Nek-wrapped problem.\n\n"
                 "options: 'NekRSProblem', 'NekRSSeparateDomainProblem', 'NekRSStandaloneProblem'");

    // It's too complicated to make sure that the dimensional form _also_ works when our
    // reference coordinates are different from what MOOSE is expecting, so just throw an error
    auto nek_mesh = dynamic_cast<NekRSMesh *>(&(_problem->mesh()));
    if (!MooseUtils::absoluteFuzzyEqual(nek_mesh->scaling(), _L))
      paramError("L",
                 "If solving NekRS in nondimensional form, you must choose "
                 "reference dimensional scales in the same units as expected by MOOSE, i.e. 'L' "
                 "must match 'scaling' in 'NekRSMesh'.");

    VariadicTable<std::string, std::string> vt({"Quantity (Non-Dimensional)", "Expression"});

    auto compress = [](Real a)
    {
      std::ostringstream v;
      v << std::setprecision(3) << std::scientific << a;
      return v.str();
    };

    vt.addRow("Position", "x / " + compress(_L));
    vt.addRow("Time", "t / " + compress(nekrs::referenceTime()));
    vt.addRow("Velocity", "u / " + compress(_U));
    vt.addRow("Temperature", "(T - " + compress(_T) + ") / " + compress(_dT));
    vt.addRow("Heat flux", "q'' / " + compress(nekrs::referenceFlux()));
    vt.addRow("Power density", "q / " + compress(nekrs::referenceSource()));
    vt.addRow("Density", "rho / " + compress(_rho));
    vt.addRow("Specific heat", "Cp / " + compress(_Cp));

    _console << "\nScales used for dimensionalizing the NekRS fields:" << std::endl;
    vt.print(_console);
    _console << std::endl;
  }
}

#endif
