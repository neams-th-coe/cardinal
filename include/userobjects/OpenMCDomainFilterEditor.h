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

/**
 * User object to modify the nuclides in an OpenMC tally.
 */
class OpenMCDomainFilterEditor : public GeneralUserObject
{
public:
  static InputParameters validParams();

  OpenMCDomainFilterEditor(const InputParameters & parameters);


  /// We don't want this user object to execute in MOOSE's control
  virtual void execute() override;

  virtual void initialize() override {}
  virtual void finalize() override {}

  void check_existing_filter_type() const;
  void bad_filter_error() const;

  std::string long_name () const { return "OpenMCDomainFilterEditor \"" + this->name() + "\""; }

protected:
  int32_t _filter_id;
  std::string _filter_type;
  const std::set<std::string> _allowed_types{"cell", "universe", "material", "mesh"};
};
