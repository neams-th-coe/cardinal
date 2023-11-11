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

#include "ExternalProblem.h"

/**
 * Base class for all MOOSE wrappings in Cardinal
 */
class CardinalProblem : public ExternalProblem
{
public:
  CardinalProblem(const InputParameters & params);

  static InputParameters validParams();

  /**
   * Check for duplicate entries in a 1-d vector
   * @param[in] var input vector
   * @param[in] name string to use for printing error message
   */
  template <typename T>
  void checkDuplicateEntries(const std::vector<T> & var, const std::string & name) const
  {
    std::set<T> set_var;
    for (const auto & v : var)
    {
      if (set_var.count(v))
        mooseError("Entries cannot be repeated in '" + name + "'!");
      set_var.insert(v);
    }
  }

  /**
   * Check whether the user has already created a variable using one of the protected
   * names that the wrapping is using.
   * @param[in] name variable name
   */
  void checkDuplicateVariableName(const std::string & name) const;

  /**
   * Whether a string ends in a particular sub-string
   * @param[in] full full string
   * @param[in] ending sub-string ending
   * @return whether full string has ending
   */
  bool stringHasEnding(std::string const & full, std::string const & ending) const;
};
