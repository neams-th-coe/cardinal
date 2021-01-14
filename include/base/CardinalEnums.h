#pragma once

#include "MooseEnum.h"

MooseEnum getNekOrderEnum();

namespace surface
{
  /// Enumeration of possible surface order reconstructions for nekRS solution transfer
  enum NekOrderEnum
  {
    first,
    second
  };
}
