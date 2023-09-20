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

#include "NekPostprocessor.h"

/**
 * This postprocessor is used strictly for testing the construction
 * of NekRSMesh in terms of the number of elements constructed and the
 * node coordinates of those elements. This object solely exists because
 * MOOSE's unit testing system is difficult to adapt to test objects that
 * depend on a mesh (including the mesh objects themselves). See
 *
 * https://github.com/idaholab/moose/discussions/16181
 *
 * for more information on the motivation for this object.
 */
class NekMeshInfoPostprocessor : public NekPostprocessor
{
public:
  NekMeshInfoPostprocessor(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() const override;

private:
  /// Type of value to compute
  const MooseEnum _test_type;

  /// element for which to read mesh information
  const libMesh::Elem * _element;

  /// element-local node for which to read mesh information
  const libMesh::dof_id_type * _node;
};
