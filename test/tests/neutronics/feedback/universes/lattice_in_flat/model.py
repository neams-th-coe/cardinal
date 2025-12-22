import openmc
import numpy as np

model = openmc.Model()

mat = openmc.Material()
mat.add_element('U', 1.0)
mat.set_density('g/cm3', 10.0)

r = openmc.Sphere(r=0.25)
r_triso = openmc.Cell(region=-r, fill=mat)
u_triso = openmc.Universe(cells=[r_triso])

n = 2
planes = [openmc.XPlane(x0=x) for x in np.linspace(0, 10, n+1)]
planes[0].boundary_type = 'vacuum'
planes[-1].boundary_type = 'vacuum'

bot = openmc.YPlane(y0=0, boundary_type='reflective')
top = openmc.YPlane(y0=10, boundary_type='reflective')
fro = openmc.ZPlane(z0=0, boundary_type='reflective')
bac = openmc.ZPlane(z0=10, boundary_type='reflective')
lay = +bot & -top & +fro & -bac

overall_region = lay & +planes[0] & -planes[-1]
spheres = openmc.model.pack_spheres(radius=0.25, region=overall_region, num_spheres = 4, seed=1)
trisos = [openmc.model.TRISO(0.25, u_triso, i) for i in spheres]

lower_left = (0, 0, 0)
upper_right = (10, 10, 10)
triso_lattice_shape = (1, 1, 1)
pitch = (10, 10, 10)

# insert TRISOs into a lattice to accelerate point location queries; the commented out lines
# below demonstrate how to fix the issue and keep going
triso_lattice = openmc.model.create_triso_lattice(trisos, lower_left, pitch, triso_lattice_shape, mat)
#uni = openmc.Universe(cells=[openmc.Cell(fill=triso_lattice)])

cells = []
for i in range(n):
  r = +planes[i] & -planes[i+1]
  cells.append(openmc.Cell(region=r & lay, fill=triso_lattice))
  #cells.append(openmc.Cell(region=r & lay, fill=uni))

root = openmc.Universe(cells=cells)
model.geometry = openmc.Geometry(root)

model.settings.batches = 50
model.settings.inactive = 10
model.settings.particles = 3000

# Create an initial uniform spatial source distribution over fissionable zones
uniform_dist = openmc.stats.Box(lower_left, upper_right)
model.settings.source = openmc.IndependentSource(space=uniform_dist)
model.settings.temperature = {'method': 'interpolation',
                        'range': (294.0, 3000.0)}

model.export_to_xml()
