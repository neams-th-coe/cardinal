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

#include "GeneralPostprocessor.h"

#include "NekBase.h"

/**
 * This postprocessor is used to integrate the usrwrk array over an
 * STL file. The contents in the usrwrk are interepreted to be one value
 * per STL triangle (at the centroid). TODO: this will need to be generalized
 * and tested if we ever change the order of the STL data transfers
 */
class NekUsrWrkSTLIntegral : public GeneralPostprocessor, public NekBase
{
public:
  NekUsrWrkSTLIntegral(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() const override;

private:
  /// Slot in the usrwrk array to integrate
  const unsigned int & _usrwrk_slot;
};
