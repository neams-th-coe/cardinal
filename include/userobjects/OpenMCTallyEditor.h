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

#include "OpenMCUserObject.h"

class OpenMCProblemBase;

/**
 * User object to modify an OpenMC tally
 */
class OpenMCTallyEditor : public OpenMCUserObject
{
public:
  static InputParameters validParams();

  OpenMCTallyEditor(const InputParameters & parameters);

  bool tallyExists() const;

  // get the index of the tally in OpenMC's data space, creating it if necessary according to the
  // input parameters
  int32_t tallyIndex() const;

  /// We don't want this user object to execute in MOOSE's control
  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override {}

  std::string longName() const { return "OpenMCTallyEditor \"" + this->name() + "\""; }

  // Accessors
  int32_t tallyId() const { return _tally_id; }

protected:
  int32_t _tally_id;
};
