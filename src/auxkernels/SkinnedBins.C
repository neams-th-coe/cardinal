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

#ifdef ENABLE_DAGMC

#include "SkinnedBins.h"

registerMooseObject("CardinalApp", SkinnedBins);

InputParameters
SkinnedBins::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>("skinner", "MOAB mesh skinner");

  MooseEnum skin_type("temperature subdomain density all", "all");
  params.addParam<MooseEnum>("skin_by", skin_type, "Which skin distribution to display");
  params.addClassDescription("Display the mapping of mesh elements to the skinned bins created by a skinner");
  return params;
}

SkinnedBins::SkinnedBins(const InputParameters & parameters) :
  AuxKernel(parameters),
  _skin_by(getParam<MooseEnum>("skin_by"))
{
  const UserObject & base = getUserObjectBase("skinner");
  _skinner = dynamic_cast<const MoabSkinner *>(&base);
  if (!_skinner)
    paramError("skinner", "This userobject must be of type MoabSkinner!");
}

Real
SkinnedBins::computeValue()
{
  if (_skin_by == "temperature")
    return _skinner->getTemperatureBin(_current_elem);
  else if (_skin_by == "subdomain")
    return _skinner->getSubdomainBin(_current_elem);
  else if (_skin_by == "density")
    return _skinner->getDensityBin(_current_elem);
  else if (_skin_by == "all")
    return _skinner->getBin(_skinner->getTemperatureBin(_current_elem), _skinner->getDensityBin(_current_elem),
      _skinner->getSubdomainBin(_current_elem));
  else
    mooseError("Unhandled skin_type enum in SkinnedBins!");
}

#endif
