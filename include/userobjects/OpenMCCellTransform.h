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

  /** Returns whether the UserObject is a translation or rotation
   * @return the transform type
   */
  MooseEnum getTransformType() const;

  /** Returns a vector of PostprocessorNames corresponding to
   * the entries in vector_value
   * @return vector of PostprocessorName
   */
  std::vector<PostprocessorName> getVectorValue() const;

  /** Given a std::vector<Real>, this function sets the Postprocessor
   * associated in the order they are defined in vector_value if a
   * Receiver postprocessor exists at the position correspondign to the value
   * @param[in] std::vector of values to set
   */
  void setTransformPPValues(const std::vector<Real> pp_values);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// OpenMC cell IDs to which the translation will be applied
  std::set<int32_t> _cell_ids;

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
