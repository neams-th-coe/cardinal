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

#include "NekVolumeSpatialBinUserObject.h"
#include "PlaneSpatialBinUserObject.h"

InputParameters
NekVolumeSpatialBinUserObject::validParams()
{
  InputParameters params = NekSpatialBinUserObject::validParams();
  return params;
}

NekVolumeSpatialBinUserObject::NekVolumeSpatialBinUserObject(const InputParameters & parameters)
  : NekSpatialBinUserObject(parameters)
{
  // we need to enforce that there are only volume distributions
  unsigned int num_side_distributions = 0;

  for (auto & uo : _bins)
  {
    const PlaneSpatialBinUserObject * side_bin = dynamic_cast<const PlaneSpatialBinUserObject *>(uo);
    if (side_bin)
      ++num_side_distributions;
  }

  if (num_side_distributions != 0)
    mooseError("This user object requires all bins to be volume distributions; you have specified " +
      Moose::stringify(num_side_distributions) + " side distributions." +
      "\noptions: HexagonalSubchannelBin, LayeredBin, RadialBin");

  // the 'normal' velocity component direction does not apply to volume bins
  if (_field == field::velocity_component && _velocity_component == component::normal)
    mooseError("Setting 'velocity_component = normal' is not supported for the '" + name() + "' user object!");
}

#endif
