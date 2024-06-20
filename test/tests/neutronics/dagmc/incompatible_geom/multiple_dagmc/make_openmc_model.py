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

# materials in slab.h5m
a = openmc.Material()
a.set_density('g/cc', 11.0)
a.add_nuclide('U235', 1.0)

b = openmc.Material()
b.set_density('g/cc', 11.0)
b.add_nuclide('U235', 0.5)
b.add_nuclide('U238', 0.5)

# materials in dagmc.h5m
u235 = openmc.Material(name="fuel")
u235.add_nuclide('U235', 1.0, 'ao')
u235.set_density('g/cc', 11)
u235.id = 40

water = openmc.Material(name="water")
water.add_nuclide('H1', 2.0, 'ao')
water.add_nuclide('O16', 1.0, 'ao')
water.set_density('g/cc', 1.0)
water.add_s_alpha_beta('c_H_in_H2O')
water.id = 41

mats = openmc.Materials([a, b, u235, water])
mats.export_to_xml()

univ1 = openmc.DAGMCUniverse(filename="../../mesh_tallies/slab.h5m", auto_geom_ids=True)
univ2 = openmc.DAGMCUniverse(filename="../../../../../../tutorials/dagmc/dagmc.h5m", auto_geom_ids=True)

sphere1 = openmc.Sphere(r=10.0, boundary_type='vacuum')
csg1 = openmc.Cell(region=-sphere1, fill=univ1)
sphere2 = openmc.Sphere(x0=100, y0=100, z0=100, r=10.0, boundary_type='vacuum')
csg2 = openmc.Cell(region=-sphere2, fill=univ2)

geometry = openmc.Geometry([csg1, csg2])
geometry.export_to_xml()
