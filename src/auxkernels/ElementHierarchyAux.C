#include "ElementHierarchyAux.h"

registerMooseObject("CardinalApp", ElementHierarchyAux);

InputParameters
ElementHierarchyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("stores the element hierarchy in a aux variable");

  return params;
}

ElementHierarchyAux::ElementHierarchyAux(const InputParameters & parameters) : AuxKernel(parameters)
{
}

Real
ElementHierarchyAux::computeValue()
{
  return static_cast<Real>(_current_elem->level());
}
