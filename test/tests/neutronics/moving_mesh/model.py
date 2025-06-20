import openmc
import math

# define OpenMC materials; the names should match the names assigned to the blocks
# in the "block <n> material <name>" lines in the journal file

model = openmc.Model()

left = openmc.Material(name="left", temperature=500)
left.set_density('g/cc', 19.30)
left.add_nuclide('U235', 1.0)

right = openmc.Material(name="right", temperature=600)
right.set_density('g/cc', 10.0)
right.add_nuclide('U235', 1.0)

model.materials = openmc.Materials([left, right])

model.settings.dagmc = True
model.settings.batches = 10
model.settings.particles = 1000
model.settings.run_mode = "eigenvalue"

model.settings.temperature = {'default': 800.0,
                              'method': 'interpolation',
                              'range': (294.0, 3000.0),
                              'tolerance': 1000.0}

model.settings.source = openmc.IndependentSource(space=openmc.stats.Box(lower_left=(0, 0, 0), upper_right=(20, 5, 5)))
dagmc_univ = openmc.DAGMCUniverse(filename='surface.h5m')
model.geometry = openmc.Geometry(root=dagmc_univ)

model.export_to_xml()
