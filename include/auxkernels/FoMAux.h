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

#include "OpenMCAuxKernel.h"

/**
 * A class which computes a Figure of Merit (FoM) for a tally quantity.
 * There are two options:
 * - The variance reduction FoM:             $FoM_{VR} = 1 / (T * R^2)$
 * - The Adaptive Mesh Refinement (AMR) FoM: $FoM_{AMR} = |u_j - u_0| / (T * R * u_0)$
 * where $T$ is the OpenMC simulation time, $R$ is the relative statistical error of the
 * field variable, $u_j$ is the variable value at adaptivity step j, and $u_0$ is the
 * initial value of the field variable on the initial simulation.
 */
class FoMAux : public OpenMCAuxKernel
{
public:
  static InputParameters validParams();

  FoMAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The variable containing the tally. Used for the AMR FoM.
  const VariableValue * _tally_val;

  /// A variable containing the initial value of the tally. Used for the AMR FoM.
  const VariableValue * _tally_val_init;

  /// The statistical relative error of the tally. Used for both FoMs.
  const VariableValue & _tally_val_rel_err;

  /// The simulation time. Used for both FoMs.
  const Real & _sim_time;

  /// The type of FoM to compute.
  const enum class FoMType
  {
    VR  = 0,
    AMR = 1
  } _fom_type;
};
