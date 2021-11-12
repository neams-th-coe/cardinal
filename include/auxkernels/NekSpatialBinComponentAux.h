#pragma once

#include "SpatialUserObjectAux.h"
#include "NekSpatialBinUserObject.h"

class NekSpatialBinComponentAux;

template <>
InputParameters validParams<NekSpatialBinComponentAux>();

/**
 * Extracts a single component of a multi-component (i.e. velocity vector)
 * Nek binned user object. This is very helpful for displaying glyphs in many
 * common visualization packages.
 */
class NekSpatialBinComponentAux : public SpatialUserObjectAux
{
public:
  static InputParameters validParams();

  NekSpatialBinComponentAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Component of side bin user object
  const unsigned int & _component;

  /// Spatial bin user object
  const NekSpatialBinUserObject * _bin_uo;
};
