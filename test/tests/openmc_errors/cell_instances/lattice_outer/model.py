import openmc
import numpy as np

model = openmc.Model()

fuel = openmc.Material()
fuel.add_element('U', 1.0)
fuel.set_density('g/cm3', 11.0)

unit = openmc.Universe(cells=[openmc.Cell(fill=fuel)])
out = openmc.Universe(cells=[openmc.Cell(fill=fuel)])

N = 3
lattice = openmc.RectLattice()
lattice.lower_left = (0, 0, 0)
lattice.pitch = (1, 1, 1)
lattice.universes = np.full((N, 1, 1), unit)
lattice.outer = out

bottom = openmc.ZPlane(z0 = -1, boundary_type = 'reflective')
top = openmc.ZPlane(z0 = 2, boundary_type = 'reflective')

back = openmc.YPlane(y0 = -1, boundary_type = 'reflective')
front = openmc.YPlane(y0 = 2, boundary_type = 'reflective')

left = openmc.XPlane(x0 = -1, boundary_type = 'reflective')
right = openmc.XPlane(x0 = 2, boundary_type = 'reflective')

r = +bottom & -top & +back & -front & +left & -right
outer_cell = openmc.Cell(region=r, fill=lattice)
univ = openmc.Universe(cells=[outer_cell])

model.geometry = openmc.Geometry(univ)
model.settings.batches = 200
model.settings.inactive = 100
model.settings.particles = 1000

lower_left = (0, 0, 0)
upper_right = (1, 1, 1)
uniform_dist = openmc.stats.Box(lower_left, upper_right, only_fissionable=True)
model.settings.source = openmc.source.IndependentSource(space=uniform_dist)
model.settings.temperature = {'default': 650.0 + 273.15,
                              'method': 'interpolation',
                              'multipole': False,
                              'tolerance': 1000.0,
                              'range': (294.0, 1600.0)}

model.export_to_xml()
