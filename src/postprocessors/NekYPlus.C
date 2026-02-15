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

#include "NekYPlus.h"

registerMooseObject("CardinalApp", NekYPlus);

InputParameters
NekYPlus::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  MooseEnum value_type("max min avg", "max");
  params.addParam<MooseEnum>("value_type", value_type, "Type of value to report");
  params.addParam<unsigned int>(
      "wall_distance_index", 1, "Index into nek::scPtr where the wall distance is stored");
  params.addClassDescription("Compute y+ on boundaries");
  return params;
}

NekYPlus::NekYPlus(const InputParameters & parameters)
  : NekSidePostprocessor(parameters),
    _value_type(getParam<MooseEnum>("value_type")),
    _wall_distance_index(getParam<unsigned int>("wall_distance_index"))
{
  if (_pp_mesh != nek_mesh::fluid)
    mooseError("The 'NekYPlus' postprocessor can only be applied to the fluid mesh "
               "boundaries!\n"
               "Please change 'mesh' to 'fluid'.");
}

Real
NekYPlus::getValue() const
{
  auto yplus = nekrs::yPlus(_boundary, _wall_distance_index);

  if (_value_type == "max")
    return yplus[0];
  else if (_value_type == "min")
    return yplus[1];
  else if (_value_type == "avg")
    return yplus[2];
  else
    mooseError("Unhandled 'value_type' in NekYPlus!");
}

#endif
