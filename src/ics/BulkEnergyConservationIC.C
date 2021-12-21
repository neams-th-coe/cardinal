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

#include "BulkEnergyConservationIC.h"

registerMooseObject("CardinalApp", BulkEnergyConservationIC);

InputParameters
BulkEnergyConservationIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<UserObjectName>("layered_integral",
    "User object providing the cumulative integral of the heat source in layers "
    "in the direction of fluid flow");
  params.addRequiredParam<Real>("mass_flowrate", "Mass flowrate of the fluid");
  params.addRequiredParam<Real>("cp", "Fluid isobaric specific heat capacity");
  params.addRequiredRangeCheckedParam<Real>("inlet_T", "inlet_T >= 0.0", "Inlet temperature");

  params.addRequiredParam<PostprocessorName>("integral",
    "Postprocessor providing the integral of the heat source, for normalization");
  params.addRequiredParam<Real>("magnitude", "Magnitude of the heat source");
  return params;
}

BulkEnergyConservationIC::BulkEnergyConservationIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _layered_integral(getUserObject<FunctionLayeredIntegral>("layered_integral")),
    _mdot(getParam<Real>("mass_flowrate")),
    _cp(getParam<Real>("cp")),
    _inlet_T(getParam<Real>("inlet_T")),
    _pp_name(getParam<PostprocessorName>("integral")),
    _integral(getPostprocessorValue("integral")),
    _magnitude(getParam<Real>("magnitude"))
{
  if (std::abs(_mdot * _cp) < libMesh::TOLERANCE)
    mooseError("Product of mass flowrate and specific heat cannot be zero!");
}

void
BulkEnergyConservationIC::initialSetup()
{
  const UserObject & pp = _fe_problem.getUserObject<UserObject>(_pp_name);
  if (!pp.getExecuteOnEnum().contains(EXEC_INITIAL))
    mooseError("The 'execute_on' parameter for the '" + _pp_name +
               "' postprocessor must include 'initial'!");
}

Real
BulkEnergyConservationIC::value(const Point & p)
{
  if (std::abs(_integral) < libMesh::TOLERANCE)
    mooseError("The volume integral of '" + _pp_name + "' cannot be zero!");

  Real power = _magnitude * _layered_integral.spatialValue(p) / _integral;
  return power / (_mdot * _cp) + _inlet_T;
}
