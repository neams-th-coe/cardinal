#********************************************************************/
#*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
#*                             Cardinal                             */
#*                                                                  */
#*                  (c) 2021 UChicago Argonne, LLC                  */
#*                        ALL RIGHTS RESERVED                       */
#*                                                                  */
#*                 Prepared by UChicago Argonne, LLC                */
#*               Under Contract No. DE-AC02-06CH11357               */
#*                With the U. S. Department of Energy               */
#*                                                                  */
#*             Prepared by Battelle Energy Alliance, LLC            */
#*               Under Contract No. DE-AC07-05ID14517               */
#*                With the U. S. Department of Energy               */
#*                                                                  */
#*                 See LICENSE for full restrictions                */
#********************************************************************/

import openmc

a = openmc.Material()
a.set_density('g/cc', 11.0)
a.add_nuclide('U235', 1.0)

b = openmc.Material()
b.set_density('g/cc', 11.0)
b.add_nuclide('U235', 0.5)
b.add_nuclide('U238', 0.5)

mats = openmc.Materials([a, b])
mats.export_to_xml()

dagmc_univ = openmc.DAGMCUniverse(filename="../../mesh_tallies/slab.h5m", auto_geom_ids=True)

sphere = openmc.Sphere(r=10.0, boundary_type='vacuum')
csg = openmc.Cell(region=-sphere)
csg.fill = dagmc_univ

geometry = openmc.Geometry([csg])
geometry.export_to_xml()
