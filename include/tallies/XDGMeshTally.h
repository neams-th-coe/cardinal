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

#include "MeshTallyBase.h"

#include "xdg/xdg.h"
#include "xdg/mesh_managers.h"

class XDGMeshTally : public MeshTallyBase
{
public:
  static InputParameters validParams();

  XDGMeshTally(const InputParameters & parameters);

protected:
  virtual unsigned int getOpenMCMeshBuildDoFMaps() override;

  /// The XDG mesh manager representing this mesh.
  std::shared_ptr<xdg::LibMeshManager> _xdg_mesh_manager;

  /// The XDG instance used for this mesh tally.
  std::shared_ptr<xdg::XDG> _xdg_instance;
};
