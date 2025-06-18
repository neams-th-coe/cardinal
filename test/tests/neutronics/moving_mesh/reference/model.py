import openmc
import math

# define OpenMC materials; the names should match the names assigned to the blocks
# in the "block <n> material <name>" lines in the journal file

model = openmc.Model()

left = openmc.Material(name="left", temperature=1000)
left.set_density('g/cc', 19.30)
left.add_nuclide('U235', 1.0)

right = openmc.Material(name="right", temperature=1000)
right.set_density('g/cc', 10.0)
right.add_nuclide('U235', 1.0)

model.materials = openmc.Materials([left, right])

model.settings.dagmc = True
model.settings.inactive = 200
model.settings.batches = 1000
model.settings.particles = 500000
model.settings.run_mode = "eigenvalue"

model.settings.temperature = {'default': 800.0,
                              'method': 'interpolation',
                              'range': (294.0, 3000.0),
                              'tolerance': 1000.0}

model.settings.source = openmc.IndependentSource(space=openmc.stats.Box(lower_left=(0, 0, 0), upper_right=(20, 5, 5)))

lefts = openmc.XPlane(x0=-5, boundary_type='vacuum')
rights = openmc.XPlane(x0=22.140275816016985+15, boundary_type='vacuum')
bot = openmc.YPlane(y0=-5, boundary_type='vacuum')
top = openmc.YPlane(y0=5, boundary_type='vacuum')
back = openmc.ZPlane(z0=-5, boundary_type='vacuum')
front = openmc.ZPlane(z0=5, boundary_type='vacuum')
middle = openmc.XPlane(x0=10.517091807564771+5)

lc = openmc.Cell(region=+lefts & -rights & +bot & -top & +back & -front & -middle, fill=left)
rc = openmc.Cell(region=+lefts & -rights & +bot & -top & +back & -front & +middle, fill=right)
univ = openmc.Universe(cells=[lc, rc])
model.geometry = openmc.Geometry(root=univ)

model.export_to_xml()
