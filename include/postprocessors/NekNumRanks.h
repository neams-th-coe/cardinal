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

#include "NekPostprocessor.h"

/**
 * Display the number of MPI ranks used to run NekRS. This can be
 * useful for understanding how MPI communicators are split when
 * more than one NekRS case is being run (either for physics multiapps
 * or in stochastic simulation, for instance).
 */
class NekNumRanks : public NekPostprocessor
{
public:
  static InputParameters validParams();

  NekNumRanks(const InputParameters & parameters);

  virtual Real getValue() const override;
};
