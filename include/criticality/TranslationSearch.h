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

#include "CriticalitySearchBase.h"
#include "OpenMCCellTransformBase.h"

/**
 * Perform a criticality search based on a translation distance
 */
class TranslationSearch : public CriticalitySearchBase, public OpenMCCellTransformBase
{
public:
  static InputParameters validParams();

  TranslationSearch(const InputParameters & parameters);

  /** Update OpenMC model with the next guess for critical.
   * @param[in] distance guess to pass to the next iteration, in mesh units
   */
  virtual void updateOpenMCModel(const Real & distance) override;
  virtual bool changingGeometry() const override { return true; }

protected:
  virtual std::string units() const override { return "[mesh units]"; }
  virtual std::string quantity() const override { return "Translation"; }

  /// the index of the translation axis used to search for criticality
  const int _translation_axis_idx;
};
