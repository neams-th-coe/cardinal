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

#include "VolumetricHeatSourceICAction.h"
#include "FEProblem.h"

registerMooseAction("CardinalApp", VolumetricHeatSourceICAction, "add_heat_source_ic");
registerMooseAction("CardinalApp", VolumetricHeatSourceICAction, "add_heat_source_postprocessor");

InputParameters
VolumetricHeatSourceICAction::validParams()
{
  InputParameters params = CardinalAction::validParams();
  params.addRequiredParam<FunctionName>("function", "Function providing shape of the heat source");
  params.addRequiredParam<VariableName>("variable", "Name of the volumetric heat source variable");
  params.addRequiredParam<Real>("magnitude", "Magnitude of the heat source upon integration");
  return params;
}

VolumetricHeatSourceICAction::VolumetricHeatSourceICAction(const InputParameters & parameters)
  : CardinalAction(parameters),
    _variable(getParam<VariableName>("variable")),
    _function(getParam<FunctionName>("function")),
    _magnitude(getParam<Real>("magnitude"))
{
}

void
VolumetricHeatSourceICAction::act()
{
  if (_current_task == "add_heat_source_postprocessor")
  {
    const std::string pp_type = "FunctionElementIntegral";
    InputParameters params = _factory.getValidParams(pp_type);
    params.set<FunctionName>("function") = _function;
    params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

    setObjectBlocks(params, _blocks);

    params.set<std::vector<OutputName>>("outputs") = {"none"};
    _problem->addPostprocessor(pp_type, "cardinal_heat_source_integral", params);
  }

  if (_current_task == "add_heat_source_ic")
  {
    const std::string ic_type = "IntegralPreservingFunctionIC";
    InputParameters params = _factory.getValidParams(ic_type);
    params.set<VariableName>("variable") = _variable;
    params.set<PostprocessorName>("integral") = "cardinal_heat_source_integral";
    params.set<FunctionName>("function") = _function;
    params.set<Real>("magnitude") = _magnitude;

    setObjectBlocks(params, _blocks);

    _problem->addInitialCondition(ic_type, "cardinal_heat_source_ic", params);
  }
}
