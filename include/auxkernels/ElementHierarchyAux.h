#pragma once

#include "AuxKernel.h"

/**
 * An aux kernel object which stores the element hierarchy
 * in an aux variable.
 */

class ElementHierarchyAux:public AuxKernel{

public:
  static InputParameters validParams();
  ElementHierarchyAux(const InputParameters & parameters);

protected:
  /// method for storing the element hierarchy
  virtual Real computeValue() override;
};