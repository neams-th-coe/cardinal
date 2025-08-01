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

#include "MooseObject.h"
#include "OpenMCCellAverageProblem.h"

class CriticalitySearchBase : public MooseObject
{
public:
  static InputParameters validParams();

  CriticalitySearchBase(const InputParameters & parameters);

  virtual void updateOpenMCModel(const Real & input) = 0;

  virtual void searchForCriticality();

protected:
  virtual std::string quantity() const = 0;

  const Real & _maximum;
  const Real & _minimum;
  const Real & _tolerance;

  OpenMCCellAverageProblem & _openmc_problem;
};
