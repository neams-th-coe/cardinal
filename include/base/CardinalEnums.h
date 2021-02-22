#pragma once

#include "MooseEnum.h"

MooseEnum getNekOrderEnum();
MooseEnum getNekFieldEnum();
MooseEnum getOperationEnum();

namespace order
{
  /// Enumeration of possible surface order reconstructions for nekRS solution transfer
  enum NekOrderEnum
  {
    first,
    second
  };
}

namespace field
{
  /// Enumeration of possible fields to integrate in nekRS
  enum NekFieldEnum
  {
    temperature,
    pressure,
    unity
  };
}

namespace operation
{
  /// Enumeration of possible operations to perform in global postprocessors
  enum OperationEnum
  {
    max,
    min
  };
}
