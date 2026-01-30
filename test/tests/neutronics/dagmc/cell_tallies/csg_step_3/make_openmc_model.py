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

water = openmc.Material(name="h2o")
water.add_nuclide('H1', 2.0)
water.add_nuclide('O16', 1.0)
water.set_density('g/cm3', 1.0)
water.add_s_alpha_beta('c_H_in_H2O')

mats = openmc.Materials([a, b, water])
mats.export_to_xml()

p1 = 12.5
p2 = 37.5
p3 = 62.5

tx = 0.01 * (87.5 + 12.5)
ty = 0.01 * (37.5 + 12.5)
tz = 0.01 * (12.5 + 12.5)

xmin = openmc.XPlane(x0=-12.5)
xmax = openmc.XPlane(x0=87.5)
ymin = openmc.YPlane(y0=-12.5)
ymax = openmc.YPlane(y0=37.5)
ymid = openmc.YPlane(y0=(-12.5+25.0))
zmin = openmc.ZPlane(z0=-12.5)
zmax = openmc.ZPlane(z0=12.5)

x1 = openmc.XPlane(x0=p1)
x2 = openmc.XPlane(x0=p2)
x3 = openmc.XPlane(x0=p3)

xmin_bc = openmc.XPlane(x0=-12.5 - tx, boundary_type="vacuum")
xmax_bc = openmc.XPlane(x0=87.5 + tx, boundary_type="vacuum")
ymin_bc = openmc.YPlane(y0=-12.5 - ty, boundary_type="vacuum")
ymax_bc = openmc.YPlane(y0=37.5 + ty, boundary_type="vacuum")
zmin_bc = openmc.ZPlane(z0=-12.5 - tz, boundary_type="vacuum")
zmax_bc = openmc.ZPlane(z0=12.5 + tz, boundary_type="vacuum")

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

inner_box = (+xmin & -xmax & +ymin & -ymax & +zmin & -zmax)
outer_box = (+xmin_bc & -xmax_bc & +ymin_bc & -ymax_bc & +zmin_bc & -zmax_bc)
water_shell = openmc.Cell(region=outer_box & ~inner_box, fill=water)

geom = openmc.Geometry([cell1, cell2, cell3, cell4, cell5, cell6, cell7, cell8, water_shell])
geom.export_to_xml()
