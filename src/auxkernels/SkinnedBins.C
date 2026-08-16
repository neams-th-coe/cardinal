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
#include "MooseUtils.h"

#include <algorithm>

registerMooseObject("CardinalApp", SkinnedBins);

InputParameters
SkinnedBins::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>("skinner", "MOAB mesh skinner");

  params.addParam<std::string>("skin_by",
                               "all",
                               "Which skin distribution to display: 'all', 'subdomain', or the "
                               "name of one of the skinner's binned fields");
  params.addClassDescription("Bins created by a skinner");
  return params;
}

SkinnedBins::SkinnedBins(const InputParameters & parameters)
  : AuxKernel(parameters),
    _skin_by(getParam<std::string>("skin_by"))
{
  const UserObjectBase & base = getUserObjectBase("skinner");
  _skinner = dynamic_cast<const MoabSkinner *>(&base);
  if (!_skinner)
    paramError("skinner", "This userobject must be of type MoabSkinner!");

  if (_skin_by != "all" && _skin_by != "subdomain")
  {
    const auto names = _skinner->binnedFieldNames();
    if (std::find(names.begin(), names.end(), _skin_by) == names.end())
      paramError("skin_by",
                 "'",
                 _skin_by,
                 "' is not a valid choice. Valid choices are 'all', 'subdomain', or one of the "
                 "skinner's binned fields: ",
                 Moose::stringify(names));
  }
}

Real
SkinnedBins::computeValue()
{
  if (_skin_by == "all")
    return _skinner->getBin(_current_elem);
  else if (_skin_by == "subdomain")
    return _skinner->getSubdomainBin(_current_elem);
  else
    return _skinner->getFieldBin(_skin_by, _current_elem);
}

#endif
