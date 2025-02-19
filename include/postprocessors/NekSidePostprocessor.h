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
 * Base class for performing sideset postprocessor operations within nekRS.
 * This class just checks that the boundary supplied by the user is a valid
 * ID within the nekRS mesh.
 */
class NekSidePostprocessor : public GeneralPostprocessor, public NekBase
{
public:
  static InputParameters validParams();

  NekSidePostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

protected:
  /// Which NekRS mesh to act on
  const nek_mesh::NekMeshEnum _pp_mesh;

  /// boundary IDs of the nekRS mesh over which to find the extreme value
  const std::vector<int> & _boundary;
};
