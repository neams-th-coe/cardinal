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

x_outer_min = openmc.XPlane(x0=-4.0, boundary_type="reflective")
x_outer_max = openmc.XPlane(x0=+5.0, boundary_type="reflective")
y_outer_min = openmc.YPlane(y0=-4.0, boundary_type="reflective")
y_outer_max = openmc.YPlane(y0=+7.0, boundary_type="reflective")
z_outer_min = openmc.ZPlane(z0=-15.0, boundary_type="reflective")
z_outer_max = openmc.ZPlane(z0=+15.0, boundary_type="reflective")

root_region = (
    +x_outer_min & -x_outer_max &
    +y_outer_min & -y_outer_max &
    +z_outer_min & -z_outer_max
)

x_mid = openmc.XPlane(x0=0.0)
y_mid = openmc.YPlane(y0=0.0)
zmin = openmc.ZPlane(z0=-15.0)
zmax = openmc.ZPlane(z0=15.0)

region_2011 = +x_outer_min & -x_mid       & +y_outer_min & -y_mid       & +zmin & -zmax
region_2012 = +x_mid       & -x_outer_max & +y_outer_min & -y_mid       & +zmin & -zmax
region_2013 = +x_outer_min & -x_mid       & +y_mid       & -y_outer_max & +zmin & -zmax
region_2014 = +x_mid       & -x_outer_max & +y_mid       & -y_outer_max & +zmin & -zmax

def make_pin_universe(univ_id, box_region, cx, cy, fuel_base_id):
    pin_prism = -openmc.model.RectangularPrism(width=pitch, height=pitch, origin=(cx, cy))
    fuel_cyl = openmc.ZCylinder(x0=cx, y0=cy, r=fuel_radius)

    fuel_cell = openmc.Cell(
        cell_id=fuel_base_id + 0,
        region=box_region & -fuel_cyl,
        fill=fuel,
    )
    water_in_cell = openmc.Cell(
        cell_id=fuel_base_id + 1,
        region=box_region & +fuel_cyl,
        fill=water,
    )
    outside_pin_cell = openmc.Cell(
        cell_id=fuel_base_id + 2,
        region=box_region & ~pin_prism,
        fill=water,
    )

    return openmc.Universe(universe_id=univ_id, cells=[fuel_cell, water_in_cell, outside_pin_cell])

pin_univ_1 = make_pin_universe(100, region_2011, -2.0, -2.0, 1011)
pin_univ_2 = make_pin_universe(110, region_2012,  2.0, -2.0, 1111)
pin_univ_3 = make_pin_universe(120, region_2013, -2.0,  2.0, 1211)
pin_univ_4 = make_pin_universe(130, region_2014,  2.0,  2.0, 1311)

c2011 = openmc.Cell(cell_id=2011, region=region_2011, fill=pin_univ_1)
c2012 = openmc.Cell(cell_id=2012, region=region_2012, fill=pin_univ_2)
c2013 = openmc.Cell(cell_id=2013, region=region_2013, fill=pin_univ_3)
c2014 = openmc.Cell(cell_id=2014, region=region_2014, fill=pin_univ_4)

quad_univ = openmc.Universe(universe_id=200, cells=[c2011, c2012, c2013, c2014])
root_cell = openmc.Cell(cell_id=1, region=root_region, fill=quad_univ)
root_universe = openmc.Universe(universe_id=0, cells=[root_cell])

model.geometry = openmc.Geometry(root_universe)

model.settings.run_mode = "eigenvalue"
model.settings.batches = 50
model.settings.inactive = 10
model.settings.particles = 1000

lower = (-4, -4, -15)
upper = (5, 7, 15)
model.settings.source = openmc.IndependentSource(
    space=openmc.stats.Box(lower_left=lower, upper_right=upper)
)

model.export_to_model_xml()
