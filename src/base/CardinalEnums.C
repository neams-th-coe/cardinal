#include "CardinalEnums.h"

MooseEnum getNekOrderEnum()
{
  return MooseEnum("first second", "first");
}

MooseEnum getNekFieldEnum()
{
  return MooseEnum("velocity_x velocity_y velocity_z velocity temperature pressure unity");
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

MooseEnum getEigenvalueEnum()
{
  return MooseEnum("collision absorption tracklength combined", "combined");
}
