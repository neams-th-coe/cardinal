#pragma once

#include "GeneralUserObject.h"

#include "OpenMCBase.h"

/**
 * User object to modify the position of an OpenMC surface.
 */
class OpenMCTranslateZPlane : public GeneralUserObject, public OpenMCBase
{
public:
  static InputParameters validParams();

  OpenMCTranslateZPlane(const InputParameters & parameters);

  virtual void initialize() {}
  virtual void finalize() {}
  virtual void execute() {}

  /// Instead, we want to have a separate method that we can call from the OpenMC problem
  virtual void setValue();

protected:
  /// The surface ID
  const int32_t & _surface_id;

  /// The surface index
  int32_t _surface_index;

  /// Nuclide densities
  const double & _displacement;
};
