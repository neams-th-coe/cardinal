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

# This is a model of three pebbles with varying enrichments in water
# (with uranium in it to cover the possibility of a fissile fluid phase).
# We are going to compare the tallies computed by OpenMC and how they are
# extracted and written into MOOSE auxiliary variables.

uo2_a = openmc.Material()
uo2_a.set_density('g/cc', 10.0)
uo2_a.add_element('U', 1.0, enrichment = 1.0)
uo2_a.add_element('O', 2.0)

uo2_b = openmc.Material()
uo2_b.set_density('g/cc', 10.0)
uo2_b.add_element('U', 1.0, enrichment = 5.0)
uo2_b.add_element('O', 2.0)

uo2_c = openmc.Material()
uo2_c.set_density('g/cc', 10.0)
uo2_c.add_element('U', 1.0, enrichment = 10.0)
uo2_c.add_element('O', 2.0)

water = openmc.Material()
water.set_density('g/cc', 1.0)
water.add_element('H', 2.0)
water.add_element('O', 1.0)
water.add_element('U', 1.0)

mats = openmc.Materials([uo2_a, uo2_b, uo2_c, water])
mats.export_to_xml()

# Create three pebbles
R = 1.5
zmin = 0.0
zmax = 8.0
sphere1 = openmc.Sphere(x0 = 0.0, y0 = 0.0, z0 = zmin, r = R)
sphere2 = openmc.Sphere(x0 = 0.0, y0 = 0.0, z0 = 4.0, r = R)
sphere3 = openmc.Sphere(x0 = 0.0, y0 = 0.0, z0 = zmax, r = R)

# create a box surrounding them
L = 5.0
l = 0.5
prism = openmc.model.rectangular_prism(L, L, boundary_type = 'reflective')
bot = openmc.ZPlane(z0 = zmin - R - l, boundary_type = 'reflective')
top = openmc.ZPlane(z0 = zmax + R + l, boundary_type = 'reflective')
box = prism & +bot & -top

solid_cell1 = openmc.Cell(fill = uo2_a, region = -sphere1)
solid_cell2 = openmc.Cell(fill = uo2_b, region = -sphere2)
solid_cell3 = openmc.Cell(fill = uo2_c, region = -sphere3)
fluid_cell = openmc.Cell(fill = water, region = +sphere1 & +sphere2 & +sphere3 & box)
geom = openmc.Geometry([solid_cell1, solid_cell2, solid_cell3, fluid_cell])
geom.export_to_xml()

# Finally, define some run settings
settings = openmc.Settings()
settings.batches = 50
settings.inactive = 10
settings.particles = 100

height = 8.0 + 2 * R + 2 * l
lower_left = (-L, -L, 0)
upper_right = (L, L, height)
uniform_dist = openmc.stats.Box(lower_left, upper_right, only_fissionable=True)
settings.source = openmc.source.Source(space=uniform_dist)
settings.temperature = {'default': 294.0,
                        'method': 'interpolation',
                        'multipole': False}
settings.export_to_xml()
