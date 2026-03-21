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

#include "MooseTypes.h"
#include "InputParameters.h"

class MooseEnum;

class OpenMCCellAverageProblem;

class OpenMCCellTransformBase
{
public:
  static InputParameters validParams();

  /// Enumeration for the transform type
  static const MooseEnum transform_type;
  /// The symbols for each dimension in the transform vector: ["φ", "θ", "ψ"]
  static const std::array<std::string, 3> rotation_vector_symbols;
  /// The symbols for the transform vector in a list form: "φ, θ, ψ"
  static const std::string rotation_vector_symbols_list;

  OpenMCCellTransformBase(const MooseObject & moose_object);

  /**
   * Do a transformation on the given cells.
   *
   * @param transform_type The transform type (translation or rotation)
   * @param transform_vector The vector by to transform
   */
  void transform(const MooseEnum & transform_type, const Point & transform_vector);

protected:
  /// OpenMC cell IDs to which the translation will be applied
  const std::set<int32_t> _cell_ids;

private:
  /// The MooseObject owning this transform
  const MooseObject & _moose_object;

  /// The OpenMC problem; for calling the transforms
  OpenMCCellAverageProblem & _openmc_problem;
};

#endif
