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
cell1 = openmc.Cell(region=+xmin & -x1 & +ymin & -ymid & +zmin & -zmax, fill=b)
cell2 = openmc.Cell(region=+x1 & -x2 & +ymin & -ymid & +zmin & -zmax, fill=b)
cell3 = openmc.Cell(region=+x2 & -x3 & +ymin & -ymid & +zmin & -zmax, fill=b)
cell4 = openmc.Cell(region=+x3 & -xmax & +ymin & -ymid & +zmin & -zmax, fill=b)
cell1.temperature = 550.0
cell2.temperature = 600.0
cell3.temperature = 650.0
cell4.temperature = 700.0

# top row of cells
cell5 = openmc.Cell(region=+xmin & -x1 & +ymid & -ymax & +zmin & -zmax, fill=a)
cell6 = openmc.Cell(region=+x1 & -x2 & +ymid & -ymax & +zmin & -zmax, fill=a)
cell7 = openmc.Cell(region=+x2 & -x3 & +ymid & -ymax & +zmin & -zmax, fill=a)
cell8 = openmc.Cell(region=+x3 & -xmax & +ymid & -ymax & +zmin & -zmax, fill=a)
cell5.temperature = 550.0
cell6.temperature = 600.0
cell7.temperature = 650.0
cell8.temperature = 700.0

geom = openmc.Geometry([cell1, cell2, cell3, cell4, cell5, cell6, cell7, cell8])
geom.export_to_xml()
