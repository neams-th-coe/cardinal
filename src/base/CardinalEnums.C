#include "CardinalEnums.h"

MooseEnum getNekOrderEnum()
{
  return MooseEnum("first second", "first");
}

MooseEnum getNekFieldEnum()
{
  return MooseEnum("x_velocity y_velocity z_velocity velocity temperature pressure unity");
}

MooseEnum getOperationEnum()
{
  return MooseEnum("max min", "max");
}

MooseEnum getTallyTypeEnum()
{
  return MooseEnum("cell mesh");
}

MooseEnum getTallyCellFilterEnum()
{
  return MooseEnum("cell cell_instance", "cell_instance");
}
