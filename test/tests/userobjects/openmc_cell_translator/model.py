import openmc

model = openmc.Model()

fuel = openmc.Material(material_id=1, name="fuel")
fuel.set_density("g/cm3", 10.0)
fuel.add_nuclide("U235", 1.0)
fuel.add_nuclide("O16", 2.0)

water = openmc.Material(material_id=2, name="water")
water.set_density("g/cm3", 1.0)
water.add_nuclide("H1", 2.0)
water.add_nuclide("O16", 1.0)

model.materials = openmc.Materials([fuel, water])

pitch = 4.0
fuel_radius = 1.0

root_half_xy = 40.0
root_half_z = 15.0

xmin = openmc.XPlane(surface_id=1, x0=-root_half_xy, boundary_type="reflective")
xmax = openmc.XPlane(surface_id=2, x0=+root_half_xy, boundary_type="reflective")
ymin = openmc.YPlane(surface_id=3, y0=-root_half_xy, boundary_type="reflective")
ymax = openmc.YPlane(surface_id=4, y0=+root_half_xy, boundary_type="reflective")
zmin = openmc.ZPlane(surface_id=5, z0=0, boundary_type="reflective")
zmax = openmc.ZPlane(surface_id=6, z0=root_half_z*2, boundary_type="reflective")

root_region = +xmin & -xmax & +ymin & -ymax & +zmin & -zmax

pin_prism = -openmc.model.RectangularPrism(width=pitch, height=pitch)
fuel_cyl = openmc.ZCylinder(surface_id=11, r=fuel_radius)

pin_fuel_cell = openmc.Cell(cell_id=1011, fill=fuel, region=pin_prism & -fuel_cyl)
pin_water_cell = openmc.Cell(cell_id=1012, fill=water, region=pin_prism & +fuel_cyl)

pin_univ = openmc.Universe(universe_id=100, cells=[pin_fuel_cell, pin_water_cell])

water_univ = openmc.Universe(universe_id=101, cells=[openmc.Cell(cell_id=1013, fill=water)])

lattice = openmc.RectLattice(lattice_id=200)
lattice.pitch = (pitch, pitch)
lattice.lower_left = (-pitch, -pitch)
lattice.universes = [
    [pin_univ, pin_univ],
    [pin_univ, pin_univ],
]
lattice.outer = water_univ

bundle_prism = -openmc.model.RectangularPrism(width=6.0 * pitch, height=6.0 * pitch)

bundle_lattice_cell = openmc.Cell(
    cell_id=2011, fill=lattice, region=bundle_prism & +zmin & -zmax
)
bundle_univ = openmc.Universe(universe_id=10, cells=[bundle_lattice_cell])

bundle_cell = openmc.Cell(cell_id=1, fill=bundle_univ, region=bundle_prism & +zmin & -zmax)
outside_water_cell = openmc.Cell(cell_id=2, fill=water, region=root_region & ~bundle_prism)

root_universe = openmc.Universe(universe_id=0, cells=[bundle_cell, outside_water_cell])
model.geometry = openmc.Geometry(root_universe)

model.settings.run_mode = "eigenvalue"
model.settings.batches = 50
model.settings.inactive = 10
model.settings.particles = 1000

lower = (-2, -2, 0)
upper = (16, 16, root_half_z*2)

model.settings.source = openmc.IndependentSource(
    space=openmc.stats.Box(lower_left=lower, upper_right=upper, only_fissionable=False)
)


model.export_to_xml()

plot = openmc.Plot()
plot.filename = "lattice_xy"
plot.origin = (0.0, 0.0, 15.0)
plot.width = (6.0 * pitch, 6.0 * pitch)
plot.pixels = (800, 800)
plot.color_by = "material"
plot.basis = "xy"

model.plots = openmc.Plots([plot])
model.plots.export_to_xml()
