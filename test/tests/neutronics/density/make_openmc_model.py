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

c = openmc.Material()
c.set_density('g/cc', 11.0)
c.add_nuclide('U238', 1.0)

d = openmc.Material()
d.set_density('g/cc', 11.0)
d.add_nuclide('Na23', 1.0)

mats = openmc.Materials([a, b, c, d])
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
cell1 = openmc.Cell(region=+xmin & -x2 & +ymin & -ymid & +zmin & -zmax, fill=a, cell_id=1)
cell2 = openmc.Cell(region=+x2 & -xmax & +ymin & -ymid & +zmin & -zmax, fill=b, cell_id=2)
cell1.temperature = 550.0
cell2.temperature = 700.0

# top row of cells
cell3 = openmc.Cell(region=+xmin & -x2 & +ymid & -ymax & +zmin & -zmax, fill=c, cell_id=3)
cell4 = openmc.Cell(region=+x2 & -xmax & +ymid & -ymax & +zmin & -zmax, fill=d, cell_id=4)
cell3.temperature = 550.0
cell4.temperature = 650.0

geom = openmc.Geometry([cell1, cell2, cell3, cell4])
geom.export_to_xml()
