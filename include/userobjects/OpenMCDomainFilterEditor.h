/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2024 UChicago Argonne, LLC                  */
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

#include "GeneralUserObject.h"

// forward declarations
class OpenMCProblemBase;

/**
 * User object to create and/or modify an OpenMC tally filter for a limited set of domain types.
 */
class OpenMCDomainFilterEditor : public GeneralUserObject
{
public:
  static InputParameters validParams();

  OpenMCDomainFilterEditor(const InputParameters & parameters);

  /// Virtual method overrides
  virtual void execute() override;
  virtual void initialize() override {}
  virtual void finalize() override {}

  /// Utility methods
  const OpenMCProblemBase * openmc_problem() const;
  void check_existing_filter_type() const;
  void bad_filter_type_error() const;
  int32_t filter_index() const;
  std::string long_name() const { return "OpenMCDomainFilterEditor \"" + this->name() + "\""; }

protected:
  int32_t _filter_id;
  std::string _filter_type;
  const std::set<std::string> _allowed_types{"cell", "universe", "material", "mesh"};
};
