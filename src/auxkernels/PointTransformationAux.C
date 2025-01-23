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

#include "PointTransformationAux.h"

registerMooseObject("CardinalApp", PointTransformationAux);

InputParameters
PointTransformationAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params += OpenMCBase::validParams();
  MooseEnum direction("x y z");
  params.addRequiredParam<MooseEnum>(
      "component", direction, "Component to visualize with this auxiliary kernel");

  params.addClassDescription("Spatial point transformation used for points sent in/out of OpenMC");
  return params;
}

PointTransformationAux::PointTransformationAux(const InputParameters & parameters)
  : AuxKernel(parameters), OpenMCBase(this, parameters), _d(getParam<MooseEnum>("component"))
{
}

Real
PointTransformationAux::computeValue()
{
  Point pt;

  if (!_openmc_problem->hasPointTransformations())
    pt = _current_elem->vertex_average();
  else
    pt = _openmc_problem->transformPoint(_current_elem->vertex_average());

  return pt(_d);
}

#endif
