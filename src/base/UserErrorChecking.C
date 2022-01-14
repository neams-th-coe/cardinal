#include "UserErrorChecking.h"
#include "CardinalApp.h"

void
checkUnusedParam(const InputParameters & p, const std::string & name, const std::string & explanation)
{
  if (p.isParamSetByUser(name))
    mooseWarning("When " + explanation + ", the '" + name + "' parameter is unused!");
}

void
checkRequiredParam(const InputParameters & p, const std::string & name, const std::string & explanation)
{
  if (!p.isParamValid(name))
    mooseError("When " + explanation + ", the '" + name + "' parameter is required!");
}
