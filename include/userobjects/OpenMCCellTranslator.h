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

#ifdef ENABLE_OPENMC_COUPLING

#include "GeneralUserObject.h"
#include "OpenMCBase.h"

#include <vector>
#include <cstdint>

class OpenMCCellTranslator : public GeneralUserObject,
                             public OpenMCBase
{
public:
  static InputParameters validParams();

  OpenMCCellTranslator(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// OpenMC cell IDs to which the translation will be applied
  const std::vector<int32_t> _cell_ids;

  /// Postprocessors providing x-translation component
  const PostprocessorValue & _dx_pp;

  /// Postprocessors providing y-translation component
  const PostprocessorValue & _dy_pp;

  /// Postprocessors providing z-translation component
  const PostprocessorValue & _dz_pp;

  /// Length multiplier to get from [Mesh] units into OpenMC's centimeters
  Real _scaling;
};

#endif
