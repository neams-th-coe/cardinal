import openmc
import numpy as np

# this model correctly sets temperatures uniquely and is the reference solution

model = openmc.Model()

mat = openmc.Material()
mat.add_element('U', 1.0)
mat.set_density('g/cm3', 10.0)

univ = openmc.Universe()
univ.add_cell(openmc.Cell(fill=mat))

n = 2
planes = [openmc.XPlane(x0=x) for x in np.linspace(0, 10, n+1)]
planes[0].boundary_type = 'vacuum'
planes[-1].boundary_type = 'vacuum'

cells = []
for i in range(n):
  r = +planes[i] & -planes[i+1]
  cells.append(openmc.Cell(region=r, fill=mat))

cells[0].temperature = 625
cells[-1].temperature = 875

root = openmc.Universe(cells=cells)
model.geometry = openmc.Geometry(root)

model.settings.batches = 50
model.settings.inactive = 10
model.settings.particles = 1000

# Create an initial uniform spatial source distribution over fissionable zones
lower_left = (0, 0, 0)
upper_right = (10, 10, 10)
uniform_dist = openmc.stats.Box(lower_left, upper_right)
model.settings.source = openmc.IndependentSource(space=uniform_dist)
model.settings.temperature = {'method': 'interpolation',
                        'range': (294.0, 3000.0)}

model.export_to_xml()
model.run()
