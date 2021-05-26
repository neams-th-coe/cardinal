import openmc

uo2 = openmc.Material()
uo2.set_density('g/cc', 10.0)
uo2.add_element('U', 1.0, enrichment = 2.5)
uo2.add_element('O', 2.0)

water = openmc.Material()
water.set_density('g/cc', 1.0)
water.add_element('H', 2.0)
water.add_element('O', 1.0)
water.add_element('U', 1.0)

mats = openmc.Materials([uo2, water])
mats.export_to_xml()

# Create three pebbles filled with plutonium
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

solid_cell1 = openmc.Cell(fill = uo2, region = -sphere1)
solid_cell2 = openmc.Cell(fill = uo2, region = -sphere2)
solid_cell3 = openmc.Cell(fill = uo2, region = -sphere3)
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
settings.temperature = {'default': 600.0,
                        'method': 'nearest',
                        'multipole': False,
                        'range': (294.0, 1600.0)}
settings.export_to_xml()

material_colors = {uo2: 'red', water: 'blue'}

plot1 = openmc.Plot()
plot1.filename = 'plot1'
plot1.width = (20.0, 20.0)
plot1.basis = 'xz'
plot1.origin = (0, 0, 0)
plot1.pixels = (1000, 1000)
plot1.color_by = 'material'
plot1.colors = material_colors

plots = openmc.Plots([plot1])
plots.export_to_xml()
