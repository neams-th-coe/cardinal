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

#include "CardinalEnums.h"
#include "PostprocessorInterface.h"

class AuxiliarySystem;

class NekBoundaryFlux : public FieldTransferBase, public PostprocessorInterface
{
public:
  static InputParameters validParams();

  NekBoundaryFlux(const InputParameters & parameters);

  ~NekBoundaryFlux();

  /// Send the boundary flux term to NekRS
  void sendDataToNek();

protected:
  /**
   * Print a warning to the user if the initial fluxes (before normalization) differ
   * significantly, since this can indicate an error with model setup.
   * @param[in] nek_flux flux to be received by Nek
   * @param[in] moose_flux flux sent by MOOSE
   */
  void checkInitialFluxValues(const Real & nek_flux, const Real & moose_flux) const;

  /**
   * Whether to conserve heat flux received in NekRS by individually re-normalizing
   * with integrals over individual sideset. This approach is technically more accurate,
   * but places limitations on how the sidesets are defined (they should NOT share any
   * nodes with one another) and more effort with vector postprocessors, so it is not
   * the default.
   */
  const bool & _conserve_flux_by_sideset;

  /// Initial value to use for the total volumetric source for ensuring conservation
  const Real & _initial_flux_integral;

  const std::vector<int> * _boundary;

  /**
   * \brief Total surface-integrated flux coming from the coupled MOOSE app.
   *
   * The mesh used for the MOOSE app may be very different from the mesh used by nekRS.
   * Elements may be much finer/coarser, and one element on the MOOSE app may not be a
   * clear subset/superset of the elements on the nekRS mesh. Therefore, to ensure
   * conservation of energy, we send the total flux integral to nekRS for internal
   * normalization of the heat flux applied on the nekRS mesh.
   */
  const PostprocessorValue * _flux_integral = nullptr;

  /**
   * \brief Sideset-wise surface-integrated flux coming from the coupled MOOSE app.
   *
   * The mesh used for the MOOSE app may be very different from the mesh used by nekRS.
   * Elements may be much finer/coarser, and one element on the MOOSE app may not be a
   * clear subset/superset of the elements on the nekRS mesh. Therefore, to ensure
   * conservation of energy, we send the flux integrals for each sideset to nekRS for internal
   * normalization of the heat flux applied on the nekRS mesh.
   */
  const VectorPostprocessorValue * _flux_integral_vpp = nullptr;

  /// MOOSE flux interpolated onto the (boundary) data transfer mesh
  double * _flux_face = nullptr;

  /// MOOSE flux interpolated onto the (volume) data transfer mesh
  double * _flux_elem = nullptr;

  /// Absolute tolerance for checking flux/heat source normalizations
  const Real & _abs_tol;

  /// Relative tolerance for checking flux/heat source normalizations
  const Real & _rel_tol;
};
