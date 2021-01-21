#include "CardinalEnums.h"

MooseEnum getNekOrderEnum()
{
  return MooseEnum("first second", "first");
}

MooseEnum getNekFieldEnum()
{
  return MooseEnum("temperature unity");
}

MooseEnum getOperationEnum()
{
  return MooseEnum("max min", "max");
}
