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

a0 = openmc.Material()
a0.set_density('kg/m3', 550.0)
a0.add_nuclide('U235', 1.0)

a1 = openmc.Material()
a1.set_density('kg/m3', 600.0)
a1.add_nuclide('U235', 1.0)

a2 = openmc.Material()
a2.set_density('kg/m3', 650.0)
a2.add_nuclide('U235', 1.0)

a3 = openmc.Material()
a3.set_density('kg/m3', 700.0)
a3.add_nuclide('U235', 1.0)

b0 = openmc.Material()
b0.set_density('kg/m3', 550.0)
b0.add_nuclide('U235', 0.5)
b0.add_nuclide('U238', 0.5)

b1 = openmc.Material()
b1.set_density('kg/m3', 600.0)
b1.add_nuclide('U235', 0.5)
b1.add_nuclide('U238', 0.5)

b2 = openmc.Material()
b2.set_density('kg/m3', 650.0)
b2.add_nuclide('U235', 0.5)
b2.add_nuclide('U238', 0.5)

b3 = openmc.Material()
b3.set_density('kg/m3', 700.0)
b3.add_nuclide('U235', 0.5)
b3.add_nuclide('U238', 0.5)

mats = openmc.Materials([a0, a1, a2, a3, b0, b1, b2, b3])
mats.export_to_xml()

p1 = 12.5
p2 = 37.5
p3 = 62.5

xmin = openmc.XPlane(x0=-12.5, boundary_type='vacuum')
xmax = openmc.XPlane(x0=87.5,  boundary_type='vacuum')
ymin = openmc.YPlane(y0=-12.5,  boundary_type='vacuum')
ymax = openmc.YPlane(y0=37.5,  boundary_type='vacuum')
ymid = openmc.YPlane(y0=(-12.5+25.0))
zmin = openmc.ZPlane(z0=-12.5,  boundary_type='vacuum')
zmax = openmc.ZPlane(z0=12.5,  boundary_type='vacuum')

x1 = openmc.XPlane(x0=p1)
x2 = openmc.XPlane(x0=p2)
x3 = openmc.XPlane(x0=p3)

# bottom row of cells
cell1 = openmc.Cell(region=+xmin & -x1 & +ymin & -ymid & +zmin & -zmax, fill=b0)
cell2 = openmc.Cell(region=+x1 & -x2 & +ymin & -ymid & +zmin & -zmax, fill=b1)
cell3 = openmc.Cell(region=+x2 & -x3 & +ymin & -ymid & +zmin & -zmax, fill=b2)
cell4 = openmc.Cell(region=+x3 & -xmax & +ymin & -ymid & +zmin & -zmax, fill=b3)

# top row of cells
cell5 = openmc.Cell(region=+xmin & -x1 & +ymid & -ymax & +zmin & -zmax, fill=a0)
cell6 = openmc.Cell(region=+x1 & -x2 & +ymid & -ymax & +zmin & -zmax, fill=a1)
cell7 = openmc.Cell(region=+x2 & -x3 & +ymid & -ymax & +zmin & -zmax, fill=a2)
cell8 = openmc.Cell(region=+x3 & -xmax & +ymid & -ymax & +zmin & -zmax, fill=a3)

geom = openmc.Geometry([cell1, cell2, cell3, cell4, cell5, cell6, cell7, cell8])
geom.export_to_xml()
