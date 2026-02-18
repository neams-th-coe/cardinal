# This input file is used by ../cells_shadow.i test where xml_directory parameter points here.
import openmc

model = openmc.Model()

fuel = openmc.Material(name="fuel")
fuel.set_density("g/cm3", 10.0)
fuel.add_nuclide("U235", 1.0)
fuel.add_nuclide("O16", 2.0)

water = openmc.Material(name="water")
water.set_density("g/cm3", 1.0)
water.add_nuclide("H1", 2.0)
water.add_nuclide("O16", 1.0)

xmin = openmc.XPlane(x0=-12.0, boundary_type="reflective")
xmax = openmc.XPlane(x0= 12.0, boundary_type="reflective")
ymin = openmc.YPlane(y0=-12.0, boundary_type="reflective")
ymax = openmc.YPlane(y0=  4.0, boundary_type="reflective")
zmin = openmc.ZPlane(z0=  0.0, boundary_type="reflective")
zmax = openmc.ZPlane(z0= 30.0, boundary_type="reflective")

root_region = +xmin & -xmax & +ymin & -ymax & +zmin & -zmax

fuel_radius = 1.0
fuel_cyl = openmc.ZCylinder(r=fuel_radius)

moving_fuel_cell = openmc.Cell(fill=fuel,  region=-fuel_cyl)
moving_water_cell = openmc.Cell(fill=water, region=+fuel_cyl)

moving_univ = openmc.Universe(cells=[moving_fuel_cell, moving_water_cell])

moving_root_cell = openmc.Cell(cell_id=2011, fill=moving_univ, region=root_region)

root_universe = openmc.Universe(cells=[moving_root_cell])
model.geometry = openmc.Geometry(root_universe)

model.settings.batches = 50
model.settings.inactive = 10
model.settings.particles = 1000

lower = (-12.0, -12.0, 0.0)
upper = ( 12.0,  12.0, 30.0)

model.settings.source = openmc.IndependentSource(
    space=openmc.stats.Box(lower_left=lower, upper_right=upper)
)

model.export_to_model_xml()
