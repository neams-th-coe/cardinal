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

#include "CardinalAction.h"

InputParameters
CardinalAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<std::vector<SubdomainName>>(
      "block", {}, "The list of block ids (SubdomainID) to which this action will be applied");
  return params;
}

CardinalAction::CardinalAction(const InputParameters & parameters)
  : Action(parameters), _blocks(getParam<std::vector<SubdomainName>>("block"))
{
}

void
CardinalAction::setObjectBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks)
{
  if (params.have_parameter<std::vector<SubdomainName>>("block"))
    for (const auto & id : blocks)
      params.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));
}
