import openmc
import numpy as np


# Register materials
mat = openmc.Material()
mat.add_element("U", 1.0, enrichment=3)
mat.add_element("O", 2.0)
mat.add_element("H", 4.0)
mat.set_density("g/cm3", 5.0)
materials = openmc.Materials([mat])

# Create a 2x2x2 regular mesh
dim = 2
lower_left = (0., 0., 0.)
upper_right = (10.0, 10.0, 10.0)
mesh_1 = openmc.RegularMesh()
mesh_1.lower_left = lower_left
mesh_1.upper_right = upper_right
mesh_1.dimension = (dim, dim, dim)

# Create a cylindrical mesh
mesh_2 = openmc.CylindricalMesh(
    r_grid=[0.0, 5.0], z_grid=[0.0, 10.0], phi_grid=[0.0, 2 * np.pi],
    origin=[5.0, 5.0, 0.0])

# Create geometry
box = openmc.model.RectangularParallelepiped(
    xmin=lower_left[0], xmax=upper_right[0],
    ymin=lower_left[1], ymax=upper_right[1],
    zmin=lower_left[2], zmax=upper_right[2],
    boundary_type="vacuum"
)

plane = openmc.ZPlane(z0=5.0)

cell_1 = openmc.Cell(fill=mat, region=(-box & -plane))
cell_2 = openmc.Cell(fill=mat, region=(-box & +plane))

# Register geometry
geometry = openmc.Geometry([cell_1, cell_2])

# Create a regular mesh tally
mesh_filter_1 = openmc.MeshFilter(mesh_1)
tally_1 = openmc.Tally(name="regular_mesh_tally")
tally_1.filters = [mesh_filter_1]
tally_1.scores = ["flux"]

# Create a cylindrical mesh tally
mesh_filter_2 = openmc.MeshFilter(mesh_2)
tally_2 = openmc.Tally(name="cylindrical_mesh_tally")
tally_2.filters = [mesh_filter_2]
tally_2.scores = ["flux"]

# Register tally
tallies = openmc.Tallies([tally_1, tally_2])

# Register settings
settings = openmc.Settings()
settings.batches = 2
settings.particles = 1000
spatial_dist = openmc.stats.Box(lower_left, upper_right)
settings.source = openmc.IndependentSource(
    space=spatial_dist, constraints={"fissionable": True})

model = openmc.Model(
    geometry=geometry, materials=materials, settings=settings, tallies=tallies)
model.export_to_model_xml()
