#pragma once

#include "NekSpatialBinUserObject.h"

/**
 * Class that performs various postprocessing operations on the
 * NekRS solution with a spatial binning formed as the product of
 * volume binning distributions.
 */
class NekVolumeSpatialBinUserObject : public NekSpatialBinUserObject
{
public:
  static InputParameters validParams();

  NekVolumeSpatialBinUserObject(const InputParameters & parameters);
};
