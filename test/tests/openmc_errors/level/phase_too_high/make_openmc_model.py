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
# extracted and written into MOOSE auxiliary variables. This input differs
# from the ../make_openmc_model.py script in that the cells are distributed,
# i.e. filling universes in the geometry.

uo2 = openmc.Material()
uo2.set_density('g/cc', 10.0)
uo2.add_element('U', 1.0, enrichment = 1.0)
uo2.add_element('O', 2.0)

water = openmc.Material()
water.set_density('g/cc', 1.0)
water.add_element('H', 2.0)
water.add_element('O', 1.0)

mats = openmc.Materials([uo2, water])
mats.export_to_xml()

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

# Create the pebble universe
solid_cell = openmc.Cell(fill = uo2, region = box)
pebble_univ = openmc.Universe(name = 'pebble')
pebble_univ.add_cells([solid_cell])

fluid_cell = openmc.Cell(fill = water, region = +sphere1 & +sphere2 & +sphere3 & box)
pebble1_cell = openmc.Cell(fill = pebble_univ, region = -sphere1)
pebble2_cell = openmc.Cell(fill = pebble_univ, region = -sphere2)
pebble3_cell = openmc.Cell(fill = pebble_univ, region = -sphere3)

root_univ = openmc.Universe(name = "root")
root_univ.add_cells([fluid_cell, pebble1_cell, pebble2_cell, pebble3_cell])

geometry = openmc.Geometry(root_univ)
geometry.export_to_xml()

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
settings.temperature = {'default': 600.0,
                        'method': 'nearest',
                        'multipole': False,
                        'range': (294.0, 1600.0)}
settings.export_to_xml()

plot = openmc.Plot()
plot.filename = 'plot1'
plot.width = (L, 2 * height)
plot.origin = (0, 0, 0)
plot.basis = 'xz'
plot.pixels = (200, 200)
plot.color_by = 'cell'

plots = openmc.Plots([plot])
plots.export_to_xml()
