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

class OpenMCCellTransform : public GeneralUserObject, public OpenMCBase
{
public:
  static InputParameters validParams();

  OpenMCCellTransform(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// OpenMC cell IDs to which the translation will be applied
  const std::vector<int32_t> _cell_ids;

  /// Transform type: "translation" or "rotation"
  const MooseEnum _transform_type;

  /// Postprocessor providing the first entry of the transform array
  const PostprocessorValue * _t0_pp;

  /// Postprocessor providing the second entry of the transform array
  const PostprocessorValue * _t1_pp;

  /// Postprocessor providing the third entry of the transform array
  const PostprocessorValue * _t2_pp;
};

#endif
