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

#include "OpenMCBase.h"

class OpenMCProblemBase;

/**
 * User object to modify an OpenMC tally
 */
class OpenMCTallyEditor : public GeneralUserObject,
                          public OpenMCBase
{
public:
  static InputParameters validParams();

  OpenMCTallyEditor(const InputParameters & parameters);

  bool tallyExists() const;

  /**
   * Get the index of the tally in OpenMC's data space, creating it if necessary
   * @return tally index
   */
  int32_t tallyIndex() const;

  virtual void execute() override;
  virtual void initialize() override{};
  virtual void finalize() override {}

  /**
   * Error to throw if multiple tally editor objects have the same tally id
   * @param[in] id ID to use in error message
   */
  void duplicateTallyError(const int32_t & id) const;

  /**
   * Error to throw if tally editor is trying to edit a tally Cardinal is
   * controlling for multiphysics
   * @param[in] id ID to use in error message
   */
  void mappedTallyError(const int32_t & id) const;

  /**
   * Get the tally ID
   * @return tally ID
   */
  int32_t tallyId() const { return _tally_id; }

protected:
  /// Tally ID to modify
  const int32_t & _tally_id;
};
