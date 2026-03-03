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
#include "PostprocessorInterface.h"
#include "OpenMCBase.h"

class OpenMCCellTransform : public GeneralUserObject, public OpenMCBase
{
public:
  static InputParameters validParams();

  OpenMCCellTransform(const InputParameters & parameters);

  /// Name of each postprocessor
  const PostprocessorName _t0_pp_name;
  const PostprocessorName _t1_pp_name;
  const PostprocessorName _t2_pp_name;

  /// Transform type: "translation" or "rotation"
  const MooseEnum _transform_type;

  void setTransformPPValues(std::tuple<PostprocessorName, Real> pp_name_value_tuple_0,
                            std::tuple<PostprocessorName, Real> pp_name_value_tuple_1,
                            std::tuple<PostprocessorName, Real> pp_name_value_tuple_2);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// OpenMC cell IDs to which the translation will be applied
  std::set<int32_t> _cell_ids;

  /// Postprocessor providing the first entry of the transform array
  const PostprocessorValue * _t0_pp;

  /// Postprocessor providing the second entry of the transform array
  const PostprocessorValue * _t1_pp;

  /// Postprocessor providing the third entry of the transform array
  const PostprocessorValue * _t2_pp;
};

#endif
