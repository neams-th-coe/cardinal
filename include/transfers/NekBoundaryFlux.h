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

#include "ConservativeFieldTransfer.h"

/**
 * Performs a transfer of a boundary flux between NekRS and MOOSE.
 * The mesh used for the MOOSE app may be very different from the mesh used by nekRS.
 * Elements may be much finer/coarser, and one element on the MOOSE app may not be a
 * clear subset/superset of the elements on the nekRS mesh. Therefore, to ensure
 * conservation, we send the flux integrals (either for all sidesets lumped
 * together or on a per-sideset basis) to nekRS for internal
 * normalization of the flux applied on the nekRS mesh.
 */
class NekBoundaryFlux : public ConservativeFieldTransfer
{
public:
  static InputParameters validParams();

  NekBoundaryFlux(const InputParameters & parameters);

  ~NekBoundaryFlux();

  virtual void sendDataToNek() override;

protected:
  /**
   * Print a warning to the user if the initial fluxes (before normalization) differ
   * significantly, since this can (but not always) indicate an error with model setup.
   * @param[in] nek_flux flux to be received by Nek
   * @param[in] moose_flux flux sent by MOOSE
   */
  void checkInitialFluxValues(const Real & nek_flux, const Real & moose_flux) const;

  /**
   * Normalize the flux sent to nekRS to conserve the total flux
   * @param[in] moose_integral total integrated flux from MOOSE to conserve
   * @param[in] nek_integral total integrated flux in nekRS to adjust
   * @param[out] normalized_nek_integral final normalized nek flux integral
   * @return whether normalization was successful, i.e. normalized_nek_integral equals
   * moose_integral
   */
  bool
  normalizeFlux(const double moose_integral, double nek_integral, double & normalized_nek_integral);

  /**
   * Normalize the flux sent to nekRS to conserve the total flux
   * @param[in] moose_integral total integrated flux from MOOSE to conserve
   * @param[in] nek_integral total integrated flux in nekRS to adjust
   * @param[out] normalized_nek_integral final normalized nek flux integral
   * @return whether normalization was successful, i.e. normalized_nek_integral equals
   * moose_integral
   */
  bool normalizeFluxBySideset(const std::vector<double> & moose_integral,
                              std::vector<double> & nek_integral,
                              double & normalized_nek_integral);

  /**
   * Whether to conserve flux received in NekRS by individually re-normalizing
   * with integrals over individual sideset. This approach is technically more accurate,
   * but places limitations on how the sidesets are defined (they should NOT share any
   * nodes with one another) and more effort with vector postprocessors, so it is not
   * the default.
   */
  const bool & _conserve_flux_by_sideset;

  /// Initial value to use for the total flux integral for ensuring conservation
  const Real & _initial_flux_integral;

  /// Boundaries through which the flux transfer occurs
  const std::vector<int> * _boundary;

  /// Dimensional reference flux, computed from characteristic scales
  const Real _reference_flux_integral;

  /// Total surface-integrated flux coming from the coupled MOOSE app.
  const PostprocessorValue * _flux_integral = nullptr;

  /// Sideset-wise surface-integrated flux coming from the coupled MOOSE app.
  const VectorPostprocessorValue * _flux_integral_vpp = nullptr;

  /// MOOSE flux interpolated onto the (boundary) data transfer mesh
  double * _flux_face = nullptr;

  /// MOOSE flux interpolated onto the (volume) data transfer mesh
  double * _flux_elem = nullptr;
};
