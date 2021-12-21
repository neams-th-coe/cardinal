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

#pragma once

#include "Action.h"

class CardinalAction;

/**
 * This is a base Action class for derived classes that set up syntax
 * in Cardinal.
 */
class CardinalAction : public Action
{
public:
  CardinalAction(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  /**
   * Set the blocks to which an object created by this action applies
   * @param[in] params input parameters
   * @param[in] blocks block that the object applies to
   */
  virtual void setObjectBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks);

  /// subdomains to which this action applies
  std::vector<SubdomainName> _blocks;
};
