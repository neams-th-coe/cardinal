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

#include "NekRSProblemBase.h"

class NekRSStandaloneProblem;

template<>
InputParameters validParams<NekRSStandaloneProblem>();

/**
 * Run NekRS as a standalone application, without any data transfers
 * to/from MOOSE.
 */
class NekRSStandaloneProblem : public NekRSProblemBase
{
public:
  NekRSStandaloneProblem(const InputParameters & params);

  virtual bool movingMesh() const override;
};
