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

class NekVolumetricSource : public FieldTransferBase, public PostprocessorInterface
{
public:
  static InputParameters validParams();

  NekVolumetricSource(const InputParameters & parameters);

  ~NekVolumetricSource();

  /// Send the volumetric source term to NekRS
  void sendDataToNek();

protected:
  /**
   * Normalize the volumetric source sent to NekRS
   * @param[in] moose_integral total integrated value from MOOSE to conserve
   * @param[in] nek_integral total integrated value in nekRS to adjust
   * @param[out] normalized_nek_integral final normalized value
   * @return whether normalization was successful, i.e. normalized_nek_integral equals
   * moose_integral
   */
  bool normalizeVolumetricSource(const double moose_integral,
                                 const double nek_integral,
                                 double & normalized_nek_integral);

  /// Initial value to use for the total volumetric source for ensuring conservation
  const Real & _initial_source_integral;

   /**
   * \brief Total volume-integrated volumetric source coming from the coupled MOOSE app.
   *
   * The mesh used for the MOOSE app may be very different from the mesh used by nekRS.
   * Elements may be much finer/coarser, and one element on the MOOSE app may not be a
   * clear subset/superset of the elements on the nekRS mesh. Therefore, to ensure
   * conservation, we send the total source integral to nekRS for internal
   * normalization of the source applied on the nekRS mesh.
   */
  const PostprocessorValue * _source_integral = nullptr;

  /// MOOSE source interpolated onto the data transfer mesh
  double * _source_elem = nullptr;

  /// Absolute tolerance for checking flux/heat source normalizations
  const Real & _abs_tol;

  /// Relative tolerance for checking flux/heat source normalizations
  const Real & _rel_tol;
};
