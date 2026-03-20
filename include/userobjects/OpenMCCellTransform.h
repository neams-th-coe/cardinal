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

#include "OpenMCCellTransformBase.h"
#include "GeneralUserObject.h"

#include <array>

class OpenMCCellTransform : public GeneralUserObject, public OpenMCCellTransformBase
{
public:
  static InputParameters validParams();

  OpenMCCellTransform(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// Transform type: "translation" or "rotation"
  const MooseEnum & _transform_type;
  /// Postprocessor providing the entries of the transform array
  std::array<const PostprocessorValue *, 3> _t_pp;
};

#endif
