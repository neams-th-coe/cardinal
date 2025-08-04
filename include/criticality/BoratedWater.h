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

#include "OpenMCMaterialSearch.h"

/**
 * Perform a criticality search based on the boron ppm in water
 */
class BoratedWater : public OpenMCMaterialSearch
{
public:
  static InputParameters validParams();

  BoratedWater(const InputParameters & parameters);

  virtual void updateOpenMCModel(const Real & input) override;

protected:
  virtual std::string quantity() const override
  {
    return "material " + std::to_string(_material_id) + " boron";
  }

  virtual std::string units() const override { return "[ppm]"; }
};
