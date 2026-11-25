import openmc
import numpy as np
from argparse import ArgumentParser

height = 1.0

model = openmc.Model()

uo2 = openmc.Material()
uo2.set_density('g/cm3', 10.29769)
uo2.add_element('U', 1., enrichment=2.4)
uo2.add_element('O', 2.)

helium = openmc.Material()
helium.set_density('g/cm3', 0.001598)
helium.add_element('He', 2.4044e-4)

zircaloy = openmc.Material()
zircaloy.set_density('g/cm3', 6.55)
zircaloy.add_element('Zr', 1.0, 'wo')

borated_water = openmc.Material()
borated_water.set_density('g/cm3', 0.740582)
borated_water.add_element('H', 2.0)
borated_water.add_element('O', 1.0)

model.materials = openmc.Materials([uo2, helium, zircaloy, borated_water])

clad_ir = openmc.YCylinder(r=0.40005)
clad_or = openmc.YCylinder(r=0.45720)

# Create a region represented as the inside of a rectangular prism
pitch = 1.25984
box = openmc.model.RectangularPrism(pitch, pitch, boundary_type='reflective', axis='y')

# Create cylindrical surfaces
fuel_or = openmc.YCylinder(r=0.39218)
s1 = openmc.ZPlane(z0=0, boundary_type='vacuum')
s2 = openmc.ZPlane(z0=height, boundary_type='vacuum')
layer = +s1 & -s2
all_cells = []
all_cells.append(openmc.Cell(fill=uo2, region=-fuel_or & layer))
all_cells.append(openmc.Cell(fill=helium, region=+fuel_or & -clad_ir & layer))
all_cells.append(openmc.Cell(fill=zircaloy, region=+clad_ir & -clad_or & layer))
all_cells.append(openmc.Cell(fill=borated_water, region=+clad_or & layer & -box))

model.geometry = openmc.Geometry(all_cells)

###############################################################################
# Define problem settings

# Indicate how many particles to run
model.settings = openmc.Settings()
model.settings.batches = 15
model.settings.inactive = 5
model.settings.particles = 200

# Create an initial uniform spatial source distribution over fissionable zones
lower_left = (-pitch/2, 0, -pitch/2)
upper_right = (pitch/2, height, pitch/2)
uniform_dist = openmc.stats.Box(lower_left, upper_right)
model.settings.source = openmc.IndependentSource(space=uniform_dist)

model.settings.temperature = {'default': 280.0 + 273.15,
                        'method': 'interpolation',
                        'range': (294.0, 3000.0),
                        'tolerance': 1000.0}

model.export_to_model_xml()
