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

#include "OpenMCCellTransform.h"
#include "UserErrorChecking.h"
#include "MooseUtils.h"

registerMooseObject("CardinalApp", OpenMCCellTransform);

InputParameters
OpenMCCellTransform::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += OpenMCCellTransformBase::validParams();

  params.addParam<MooseEnum>("transform_type",
                             OpenMCCellTransformBase::transform_type,
                             "Type of transform to apply: 'translation' (dx,dy,dz) or 'rotation'"
                             "(" +
                                 OpenMCCellTransformBase::transform_vector_symbols_list +
                                 ") in degrees, where the angles are the rotations about the "
                                 "x, y, and z axes, respectively.");

  params.addRequiredParam<std::vector<PostprocessorName>>(
      "vector_value",
      "An array of three values/postprocessors. For translation this array expects (dx, dy, dz) in "
      "mesh"
      " units. For rotation this array expects '" +
          OpenMCCellTransformBase::transform_vector_symbols_list + "' in degrees.");

  params.addClassDescription(
      "UserObject that applies either translation or rotation on one or more OpenMC cells. "
      "The transform is driven by a transform array of three MOOSE postprocessors/scalar values");

  return params;
}

OpenMCCellTransform::OpenMCCellTransform(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    OpenMCCellTransformBase(static_cast<MooseObject &>(*this)),
    _transform_type(getParam<MooseEnum>("transform_type"))
{
  const auto & pp_name_vector = getParam<std::vector<PostprocessorName>>("vector_value");
  if (pp_name_vector.size() != 3)
    paramError("vector_value",
               "Provide exactly 3 values/postprocessors: 'dx dy dz' in mesh units for translation"
               "transform or '" +
                   OpenMCCellTransformBase::transform_vector_symbols_list +
                   "' in degrees for rotation transform.");

  for (const auto i : index_range(_t_pp))
    _t_pp[i] = &getPostprocessorValue("vector_value", i);
}

void
OpenMCCellTransform::execute()
{
  transform(_transform_type, Point(*_t_pp[0], *_t_pp[1], *_t_pp[2]));
}

#endif
