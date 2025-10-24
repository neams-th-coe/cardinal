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

#include "NekTransferBase.h"
#include "AuxiliarySystem.h"

/**
 * Base class for facilitating a data transfer between MOOSE and the NekRS
 * code internals for a field (a variable defined at the GLL points).
 */
class FieldTransferBase : public NekTransferBase
{
public:
  static InputParameters validParams();

  FieldTransferBase(const InputParameters & parameters);

  ~FieldTransferBase();

  /**
   * Get the mapping of usrwrk slots to variable names for all field transfers
   * @return map ordered as (slot number, name)
   */
  static std::map<unsigned int, std::string> usrwrkMap() { return _field_usrwrk_map; }

  /**
   * Get the mapping of usrwrk slots to their scalings
   * @return map ordered as (MOOSE variable name, (additive, divisor))
   */
  static std::map<std::string, std::pair<Real, Real>> usrwrkScales() { return _field_usrwrk_scales; }

protected:
  /**
   * Fill an outgoing auxiliary variable field with nekRS solution data
   * @param[in] var_number auxiliary variable number
   * @param[in] value nekRS solution data to fill the variable with
   */
  void fillAuxVariable(const unsigned int var_number, const double * value);

  /**
   * Add a MOOSE variable to facilitate coupling
   * @param[in] name variable name
   */
  void addExternalVariable(const std::string name);

  /**
   * Add a MOOSE variable to facilitate coupling
   * @param[in] slot slot in usrwrk array holding this field
   * @param[in] name variable name
   * @param[in] additive shift to apply to the non-dimensional variable
   * @param[in] divisor division to apply to the non-dimensional variable
   */
  void addExternalVariable(const unsigned int slot, const std::string name, const Real additive, const Real divisor);

  /// Variable name (or prefix of names) to create in MOOSE to facilitate data passing
  std::string _variable;

  /// Slot in usrwrk array to use for writing data, if 'direction = to_nek'
  std::vector<unsigned int> _usrwrk_slot;

  /// Internal number for the variable(s) created in MOOSE (name, number)
  std::map<std::string, unsigned int> _variable_number;

  /**
   * Information about data stored in the usrwrk array for error checking and diagnostics;
   * stored as (slot, variable name in MOOSE)
   */
  static std::map<unsigned int, std::string> _field_usrwrk_map;

  /**
   * Information about nondimensional scaling to be applied to a MOOSE variable, stored as
   * (slot, {shift, divisor})
   */
  static std::map<std::string, std::pair<Real, Real>> _field_usrwrk_scales;

  /// Number of points on the MOOSE mesh to write per element surface
  int _n_per_surf;

  /// Number of points on the MOOSE mesh to write per element volume
  int _n_per_vol;

  /// MOOSE data interpolated onto the (boundary) data transfer mesh
  double * _v_face = nullptr;

  /// MOOSE data interpolated onto the (volume) data transfer mesh
  double * _v_elem = nullptr;

  /// Scratch space to place external NekRS fields before writing into auxiliary variables
  double * _external_data = nullptr;
};
