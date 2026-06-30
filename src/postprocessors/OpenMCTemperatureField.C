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

#include "OpenMCTemperatureField.h"
#include "openmc/simulation.h"

registerMooseObject("CardinalApp", OpenMCTemperatureField);

InputParameters
OpenMCTemperatureField::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Returns a vector of temperatures corresponding to each mesh cell of "
                             "the OpenMC temperature field.");
  return params;
}

OpenMCTemperatureField::OpenMCTemperatureField(const InputParameters & params)
  : GeneralVectorPostprocessor(params),
    _cell_id(declareVector("cell_id")),
    _temperature(declareVector("temperature"))
{
}

void
OpenMCTemperatureField::initialize()
{
  _cell_id.clear();
  _temperature.clear();
}

void
OpenMCTemperatureField::execute()
{
  for (std::size_t i = 0; i < openmc::simulation::temperature_field.values().size(); i++)
  {
    double temperature = openmc::simulation::temperature_field.value(i);

    _cell_id.push_back(i);
    _temperature.push_back(temperature);
  }
}
