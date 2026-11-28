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
    boundary_type="reflective"
)
cell = openmc.Cell(fill=mat, region=-box)

# Register geometry
geometry = openmc.Geometry([cell])

# Define a uniform velocity field on the mesh cells
# For a 2x2x2 mesh there are 8 unique nodes, each with 3 components.
velocity_field = openmc.VelocityField(
   mesh=mesh,
   values=np.array([1.0]*3*8).reshape(-1,3),
   mapping="cell"
)

# DNP drift
dnp_drift = openmc.DNPDrift(
   velocity_field=velocity_field,
   boundary_map={
      "inlet": [1],
      "outlet": [2],
      "wall": [3]
   },
   physical_group_map={
      "face_ids": [
            0, 24, 12, 36, 7, 31, 19, 43, 15, 39, 21, 45, 2,
            26, 8,32, 4, 16, 10, 22, 29, 41, 35, 47
      ],
      "physical_groups": [
            1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
            3, 3, 3, 3, 3, 3, 3, 3
      ]},
   integrator="RK4",
   integrator_dt=0.1,
   recycling=True,
   external_travel_time=2.0
)

# Register settings
settings = openmc.Settings()
settings.event_based = False
settings.batches = 2
settings.inactive = 1
settings.particles = 50
settings.dnp_drift = dnp_drift
spatial_dist = openmc.stats.Box(lower_left, upper_right)
settings.source = openmc.IndependentSource(
    space=spatial_dist, constraints={"fissionable": True})
settings.temperature = {
    "method": "interpolation", "multipole": True, "range": (270.0, 3000.0)}

model = openmc.Model(geometry=geometry, materials=materials, settings=settings)
model.export_to_model_xml()
