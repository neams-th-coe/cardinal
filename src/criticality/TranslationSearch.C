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

#include "TranslationSearch.h"

registerMooseObject("CardinalApp", TranslationSearch);

InputParameters
TranslationSearch::validParams()
{
  auto params = CriticalitySearchBase::validParams();
  params += OpenMCCellTransformBase::validParams();
  params.addRequiredParam<MooseEnum>("translation_axis",
                                     MooseEnum("x=0 y=1 z=2"),
                                     "Axis along which to translate the cell's fill.");
  params.addClassDescription(
      "Searches for criticality by modifying cell translation(s) in mesh units");

  return params;
}

TranslationSearch::TranslationSearch(const InputParameters & parameters)
  : CriticalitySearchBase(parameters),
    OpenMCCellTransformBase(static_cast<MooseObject &>(*this)),
    _translation_axis_idx(getParam<MooseEnum>("translation_axis"))
{
}

void
TranslationSearch::updateOpenMCModel(const Real & distance)
{
  _console << "OpenMC will run with next guess for translation = " << distance << " " << units()
           << std::endl;

  // make a vectorized version of the translation with 0 (default Point value) for the
  // non-translating axes
  Point translation;
  // the enum default indices correspond to which vector component is non zero
  translation(_translation_axis_idx) = distance;

  // translation transformation
  auto transform_type_enum = OpenMCCellTransformBase::transform_type;
  transform_type_enum = "translation";

  // do the transform
  transform(transform_type_enum, translation);
}

#endif
