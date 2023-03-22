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

xmin = openmc.XPlane(x0=-12.5, boundary_type='vacuum')
xmax = openmc.XPlane(x0=87.5,  boundary_type='vacuum')
ymin = openmc.YPlane(y0=-12.5,  boundary_type='vacuum')
ymax = openmc.YPlane(y0=37.5,  boundary_type='vacuum')
ymid = openmc.YPlane(y0=(-12.5+25.0))
zmin = openmc.ZPlane(z0=-12.5,  boundary_type='vacuum')
zmax = openmc.ZPlane(z0=12.5,  boundary_type='vacuum')

cell1 = openmc.Cell(region=+xmin & -xmax & +ymin & -ymid & +zmin & -zmax, fill=b)
cell2 = openmc.Cell(region=+xmin & -xmax & +ymid & -ymax & +zmin & -zmax, fill=a)

geom = openmc.Geometry([cell1, cell2])
geom.export_to_xml()
