#include "CardinalEnums.h"

MooseEnum getNekOrderEnum()
{
  return MooseEnum("first second", "first");
}

MooseEnum getNekFieldEnum()
{
  return MooseEnum("temperature pressure unity");
}

MooseEnum getOperationEnum()
{
  return MooseEnum("max min", "max");
}

MooseEnum getTallyTypeEnum()
{
  return MooseEnum("cell mesh");
}
