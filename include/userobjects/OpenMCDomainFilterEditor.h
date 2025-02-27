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

#include "CardinalEnums.h"

class OpenMCProblemBase;

/**
 * User object to create and/or modify an OpenMC tally filter for a limited set of domain types.
 */
class OpenMCDomainFilterEditor : public OpenMCUserObject
{
public:
  static InputParameters validParams();

  OpenMCDomainFilterEditor(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override{};
  virtual void finalize() override {}

  /**
   * Get the index of the filter in OpenMC's data space
   */
  bool filterExists() const;

  /**
   * Return the index of the filter in the OpenMC data space
   */
  int32_t filterIndex() const;

  /**
   * Check that this object's filter type is valid and matches the type in the OpenMC data space
   */
  void checkFilterTypeMatch() const;

  /**
   * Error to throw if a filter ID is used in multiple editors
   * @param[in] id ID to use in error message
   */
  void duplicateFilterError(const int32_t & id) const;

  std::string filterTypeEnumToString(OpenMCFilterType t) const;
  OpenMCFilterType stringToFilterTypeEnum(const std::string & s) const;

  // Accessors
  int32_t filterId() const { return _filter_id; }

protected:
  const bool & _create_filter;

  int32_t _filter_id;
  OpenMCFilterType _filter_type;
};
