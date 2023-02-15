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

uo2 = openmc.Material()
uo2.set_density('g/cc', 10.0)
uo2.add_element('U', 1.0, enrichment=2.5)
uo2.add_element('O', 2.0)

mats = openmc.Materials([uo2])
mats.export_to_xml()

# Create three pebbles filled with plutonium
R = 1.5
zmin = 0.0
zmax = 8.0
sphere1 = openmc.Sphere(x0=0.0, y0=0.0, z0=zmin, r=R, boundary_type='vacuum')
sphere2 = openmc.Sphere(x0=0.0, y0=0.0, z0=4.0, r=R, boundary_type='vacuum')
sphere3 = openmc.Sphere(x0=0.0, y0=0.0, z0=zmax, r=R, boundary_type='vacuum')

solid_cell1 = openmc.Cell(fill=None, region=-sphere1)
solid_cell2 = openmc.Cell(fill=uo2, region=-sphere2)
solid_cell3 = openmc.Cell(fill=uo2, region=-sphere3)
geom = openmc.Geometry([solid_cell1, solid_cell2, solid_cell3])
geom.export_to_xml()

# Finally, define some run settings
settings = openmc.Settings()
settings.batches = 50
settings.inactive = 10
settings.particles = 100

L = 5.0
l = 0.5
height = 8.0 + 2 * R + 2 * l
lower_left = (-L, -L, 0)
upper_right = (L, L, height)
uniform_dist = openmc.stats.Box(lower_left, upper_right, only_fissionable=True)
settings.source = openmc.source.Source(space=uniform_dist)
settings.temperature = {'default': 600.0,
                        'method': 'nearest',
                        'multipole': False,
                        'range': (294.0, 1600.0)}
settings.export_to_xml()
