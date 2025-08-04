/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#include "UserErrorChecking.h"
#include "CardinalApp.h"

void
checkUnusedParam(const InputParameters & p,
                 const std::vector<std::string> & names,
                 const std::string & explanation,
                 const bool error)
{
  for (const auto & n : names)
    checkUnusedParam(p, n, explanation, error);
}

void
checkUnusedParam(const InputParameters & p,
                 const std::string & name,
                 const std::string & explanation,
                 const bool error)
{
  if (p.isParamSetByUser(name))
  {
    if (error)
      mooseError("When " + explanation + ", the '" + name + "' parameter is unused!");
    else
      mooseWarning("When " + explanation + ", the '" + name + "' parameter is unused!");
  }
}

void
checkRequiredParam(const InputParameters & p,
                   const std::vector<std::string> & names,
                   const std::string & explanation)
{
  for (const auto & n : names)
    checkRequiredParam(p, n, explanation);
}

void
checkRequiredParam(const InputParameters & p,
                   const std::string & name,
                   const std::string & explanation)
{
  if (!p.isParamValid(name))
    mooseError("When " + explanation + ", the '" + name + "' parameter is required!");
}

void
checkJointParams(const InputParameters & p,
                 const std::vector<std::string> & name,
                 const std::string & explanation)
{
  bool at_least_one_present = false;
  bool at_least_one_not_present = false;
  std::string name_list = "";

  for (const auto & s : name)
  {
    name_list += " '" + s + "',";

    if (p.isParamValid(s))
      at_least_one_present = true;
    else
      at_least_one_not_present = true;
  }

  if (at_least_one_present && at_least_one_not_present)
  {
    name_list.pop_back();
    mooseError("When " + explanation + ", the" + name_list +
               " parameters\nmust either ALL "
               "be specified or ALL omitted; you have only provided a subset of parameters!");
  }
}

#ifdef ENABLE_OPENMC_COUPLING
void
catchOpenMCError(const int & err, const std::string descriptor)
{
  if (err)
    mooseError(
        "In attempting to ", descriptor, ", OpenMC reported:\n\n", std::string(openmc_err_msg));
}
#endif
