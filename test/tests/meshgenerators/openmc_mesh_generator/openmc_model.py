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
mesh = openmc.RegularMesh()
mesh.lower_left = lower_left
mesh.upper_right = upper_right
mesh.dimension = (dim, dim, dim)

# Create geometry
box = openmc.model.RectangularParallelepiped(
    xmin=lower_left[0], xmax=upper_right[0],
    ymin=lower_left[1], ymax=upper_right[1],
    zmin=lower_left[2], zmax=upper_right[2],
    boundary_type="vacuum"
)
cell = openmc.Cell(fill=mat, region=-box)

# Register geometry
geometry = openmc.Geometry([cell])

# Create mesh tally
mesh_filter = openmc.MeshFilter(mesh)
tally = openmc.Tally(name="mesh_tally")
tally.filters = [mesh_filter]
tally.scores = ["flux"]

# Register tally
tallies = openmc.Tallies([tally])

# Register settings
settings = openmc.Settings()
settings.batches = 2
settings.particles = 50
spatial_dist = openmc.stats.Box(lower_left, upper_right)
settings.source = openmc.IndependentSource(
    space=spatial_dist, constraints={"fissionable": True})

model = openmc.Model(
    geometry=geometry, materials=materials, settings=settings, tallies=tallies)
model.export_to_model_xml()
