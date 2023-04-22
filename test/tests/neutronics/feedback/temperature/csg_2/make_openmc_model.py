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
a0.set_density('kg/m3', 1000.0)
a0.add_nuclide('U235', 1.0)

mats = openmc.Materials([a0])
mats.export_to_xml()

prism = openmc.rectangular_prism(100.0, 50.0, boundary_type='vacuum')
top = openmc.ZPlane(z0=50.0, boundary_type='vacuum')
bot = openmc.ZPlane(z0=00.0, boundary_type='vacuum')
split = openmc.XPlane(x0=0.0)

cell1 = openmc.Cell(region=prism & +bot & -top & -split, fill=a0)
cell2 = openmc.Cell(region=prism & +bot & -top & +split, fill=a0)

cell1.temperature = 500.0
cell2.temperature = 800.0

geom = openmc.Geometry([cell1, cell2])
geom.export_to_xml()
