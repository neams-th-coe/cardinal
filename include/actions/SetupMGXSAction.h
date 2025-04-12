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

#include "CardinalAction.h"
#include "OpenMCBase.h"
#include "EnergyBinBase.h"

class SetupMGXSAction : public CardinalAction,
                        public OpenMCBase,
                        public EnergyBinBase
{
public:
  static InputParameters validParams();

  SetupMGXSAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /// A function to add the filters necessary for MGXS generation.
  void addFilters();

  /// A function to add the tallies necessary for MGXS generation.
  void addTallies();

  /// A function which adds auxvariables that store the computed MGXS.
  void addAuxVars();

  /// A function which adds auxkernels that compute the MGXS.
  void addAuxKernels();
};
