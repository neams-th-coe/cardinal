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
#include "NekRSProblem.h"
#include "NekInterface.h"
#include "VariadicTable.h"
#include "NekRSMesh.h"
#include "UserErrorChecking.h"

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

  // for non-temperature passive scalars, these represent the coefficient on the time term
  params.addRangeCheckedParam<Real>("transport_coeff_1", 1.0, "transport_coeff_1 > 0", "Reference coefficient on the time derivative term in the governing equation for scalar 1");
  params.addRangeCheckedParam<Real>("transport_coeff_2", 1.0, "transport_coeff_2 > 0", "Reference coefficient on the time derivative term in the governing equation for scalar 2");
  params.addRangeCheckedParam<Real>("transport_coeff_3", 1.0, "transport_coeff_3 > 0", "Reference coefficient on the time derivative term in the governing equation for scalar 3");

  // for passive scalars, these are typically dimensionalized as (T - T0) / dT
  params.addRangeCheckedParam<Real>("T", 0.0, "T >= 0.0", "Reference temperature");
  params.addRangeCheckedParam<Real>("dT", 1.0, "dT > 0.0", "Reference temperature difference");
  params.addRangeCheckedParam<Real>("s01", 0.0, "s01 >= 0.0", "Reference scalar 1");
  params.addRangeCheckedParam<Real>("ds01", 1.0, "ds01 > 0.0", "Reference scalar 1 difference");
  params.addRangeCheckedParam<Real>("s02", 0.0, "s02 >= 0.0", "Reference scalar 2");
  params.addRangeCheckedParam<Real>("ds02", 2.0, "ds02 > 0.0", "Reference scalar 2 difference");
  params.addRangeCheckedParam<Real>("s03", 0.0, "s03 >= 0.0", "Reference scalar 3");
  params.addRangeCheckedParam<Real>("ds03", 3.0, "ds03 > 0.0", "Reference scalar 3 difference");
  return params;
}

DimensionalizeAction::DimensionalizeAction(const InputParameters & parameters)
  : Action(parameters),
    _U(getParam<Real>("U")),
    _T(getParam<Real>("T")),
    _dT(getParam<Real>("dT")),
    _s01(getParam<Real>("s01")),
    _ds01(getParam<Real>("ds01")),
    _s02(getParam<Real>("s02")),
    _ds02(getParam<Real>("ds02")),
    _s03(getParam<Real>("s03")),
    _ds03(getParam<Real>("ds03")),
    _L(getParam<Real>("L")),
    _rho(getParam<Real>("rho")),
    _Cp(getParam<Real>("Cp")),
    _transport_coeff_1(getParam<Real>("transport_coeff_1")),
    _transport_coeff_2(getParam<Real>("transport_coeff_2")),
    _transport_coeff_3(getParam<Real>("transport_coeff_3"))
{
  // inform NekRS of the scaling that we are using; the NekInterface holds all
  // the reference scales and provides accessor methods
  nekrs::initializeDimensionalScales(
      _U, _T, _dT, _L, _rho, _Cp, _transport_coeff_1, _transport_coeff_2, _transport_coeff_3, _s01, _ds01, _s02, _ds02, _s03, _ds03);
}

void
DimensionalizeAction::act()
{
  if (_current_task == "add_dimensionalization")
  {
    auto nek_problem = dynamic_cast<NekRSProblem *>(_problem.get());

    if (!nek_problem)
      mooseError("The [Dimensionalize] block can only be used with wrapped Nek cases! "
                 "You need to change the [Problem] block to type NekRSProblem.");

    // check if the temperature actually exists
    if (!nekrs::hasTemperatureVariable())
    {
      checkUnusedParam(parameters(),
                       "T",
                       "NekRS case files do not have a temperature variable! " +
                           nekrs::firstPassiveScalarNamingError());
      checkUnusedParam(parameters(),
                       "dT",
                       "NekRS case files do not have a temperature variable! " +
                           nekrs::firstPassiveScalarNamingError());
    }

    for (int i = 0; i < 3; ++i)
    {
      // check if the scalars actually exist; we currently support 3 scalars. The zeroth scalar
      // is always temperature, which is checked above
      if (!nekrs::hasScalarVariable(i + 1))
      {
        auto is = std::to_string(i + 1);
        checkUnusedParam(parameters(), "s0" + is, "NekRS case files do not have a SCALAR" + is);
        checkUnusedParam(parameters(), "ds0" + is, "NekRS case files do not have a SCALAR" + is);
        checkUnusedParam(parameters(), "transport_coeff" + is, "NekRS case files do not have a SCALAR" + is);
      }
    }

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
    vt.addRow("Pressure", "P / " + compress(nekrs::nondimensionalDivisor(field::pressure)));

    if (nekrs::hasScalarVariable(0))
    {
      vt.addRow("Temperature", "(T - " + compress(_T) + ") / " + compress(_dT));
      vt.addRow("Heat flux", "q'' / " + compress(nekrs::nondimensionalDivisor(field::heat_flux)));
      vt.addRow("Heat source",
                "qdot / " + compress(nekrs::nondimensionalDivisor(field::heat_source)));
    }

    if (nekrs::hasScalarVariable(1))
    {
      vt.addRow("Scalar 01", "(s - " + compress(_s01) + ") /" + compress(_ds01));
      vt.addRow("Scalar flux", "j'' / " + compress(nekrs::nondimensionalDivisor(field::scalar01_flux)));
      vt.addRow("Scalar volume source",
                "cdot / " + compress(nekrs::nondimensionalDivisor(field::scalar01_source)));
    }

    if (nekrs::hasScalarVariable(2))
    {
      vt.addRow("Scalar 02", "(s - " + compress(_s02) + ") /" + compress(_ds02));
      vt.addRow("Scalar flux", "j'' / " + compress(nekrs::nondimensionalDivisor(field::scalar02_flux)));
      vt.addRow("Scalar volume source",
                "cdot / " + compress(nekrs::nondimensionalDivisor(field::scalar02_source)));
    }

    if (nekrs::hasScalarVariable(3))
    {
      vt.addRow("Scalar 03", "(s - " + compress(_s03) + ") /" + compress(_ds03));
      vt.addRow("Scalar flux", "j'' / " + compress(nekrs::nondimensionalDivisor(field::scalar03_flux)));
      vt.addRow("Scalar volume source",
                "cdot / " + compress(nekrs::nondimensionalDivisor(field::scalar03_source)));
    }

    vt.addRow("Density", "rho / " + compress(_rho));
    vt.addRow("Specific heat", "Cp / " + compress(_Cp));
    vt.addRow("Transport coeff, scalar 1", "a / " + compress(_transport_coeff_1));
    vt.addRow("Transport coeff, scalar 2", "a / " + compress(_transport_coeff_2));
    vt.addRow("Transport coeff, scalar 3", "a / " + compress(_transport_coeff_3));

    _console << "Scales used for dimensionalizing the NekRS fields:" << std::endl;
    vt.print(_console);
    _console << std::endl;
  }
}

#endif
