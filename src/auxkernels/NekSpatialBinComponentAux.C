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

#include "NekSpatialBinComponentAux.h"
#include "CardinalEnums.h"

registerMooseObject("CardinalApp", NekSpatialBinComponentAux);

InputParameters
NekSpatialBinComponentAux::validParams()
{
  InputParameters params = SpatialUserObjectAux::validParams();
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component", "component < 3", "Component of user object");
  params.addClassDescription(
      "Component-wise (x, y, z) spatial value returned from a Nek user object");
  return params;
}

NekSpatialBinComponentAux::NekSpatialBinComponentAux(const InputParameters & parameters)
  : SpatialUserObjectAux(parameters), _component(getParam<unsigned int>("component"))
{
  // by requiring this userobject, we automatically ensure correct NekRSProblem-type problems
  // because the NekUserObject checks for compatibility
  _bin_uo = dynamic_cast<const NekSpatialBinUserObject *>(&_user_object);

  if (!_bin_uo)
    mooseError("This auxkernel can only be combined with NekSpatialBinUserObject-derived classes!\n"
               "You have specified the '" +
               _user_object.type() + "' user object instead.");

  if (_bin_uo->field() != field::velocity_component)
    mooseError("This auxkernel can only be used with a binning user object that sets "
               "'field = velocity_component'!");
}

Real
NekSpatialBinComponentAux::computeValue()
{
  if (isNodal())
    return _bin_uo->spatialValue(*_current_node, _component);
  else
    return _bin_uo->spatialValue(_current_elem->vertex_average(), _component);
}

#endif
