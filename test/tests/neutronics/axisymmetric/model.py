import openmc
import numpy as np
from argparse import ArgumentParser

N = 5
height = 300.0

model = openmc.Model()

uo2 = openmc.Material(name='UO2 fuel at 2.4% wt enrichment')
uo2.set_density('g/cm3', 10.29769)
uo2.add_element('U', 1., enrichment=2.4)
uo2.add_element('O', 2.)

helium = openmc.Material(name='Helium for gap')
helium.set_density('g/cm3', 0.001598)
helium.add_element('He', 2.4044e-4)

zircaloy = openmc.Material(name='Zircaloy 4')
zircaloy.set_density('g/cm3', 6.55)
zircaloy.add_element('Sn', 0.014  , 'wo')
zircaloy.add_element('Fe', 0.00165, 'wo')
zircaloy.add_element('Cr', 0.001  , 'wo')
zircaloy.add_element('Zr', 0.98335, 'wo')

borated_water = openmc.Material(name='Borated water')
borated_water.set_density('g/cm3', 0.740582)
borated_water.add_element('B', 4.0e-5)
borated_water.add_element('H', 5.0e-2)
borated_water.add_element('O', 2.4e-2)
borated_water.add_s_alpha_beta('c_H_in_H2O')

model.materials = openmc.Materials([uo2, helium, zircaloy, borated_water])

# Create cylindrical surfaces
fuel_or = openmc.ZCylinder(r=0.39218, name='Fuel OR')
clad_ir = openmc.ZCylinder(r=0.40005, name='Clad IR')
clad_or = openmc.ZCylinder(r=0.45720, name='Clad OR')

# Create a region represented as the inside of a rectangular prism
pitch = 1.25984
box = openmc.model.RectangularPrism(pitch, pitch, boundary_type='reflective')

# Create cells, mapping materials to regions - split up the axial height
planes = np.linspace(0.0, height, N + 1)
plane_surfaces = []
for i in range(N + 1):
  plane_surfaces.append(openmc.ZPlane(z0=planes[i]))

# set the boundary condition on the topmost and bottommost planes to vacuum
plane_surfaces[0].boundary_type = 'vacuum'
plane_surfaces[-1].boundary_type = 'vacuum'

all_cells = []
for i in range(N):
  layer = +plane_surfaces[i] & -plane_surfaces[i + 1]
  all_cells.append(openmc.Cell(fill=uo2, region=-fuel_or & layer))
  all_cells.append(openmc.Cell(fill=helium, region=+fuel_or & -clad_ir & layer))
  all_cells.append(openmc.Cell(fill=zircaloy, region=+clad_ir & -clad_or & layer))
  all_cells.append(openmc.Cell(fill=borated_water, region=+clad_or & layer & -box))

model.geometry = openmc.Geometry(all_cells)

###############################################################################
# Define problem settings

# Indicate how many particles to run
model.settings = openmc.Settings()
model.settings.batches = 150
model.settings.inactive = 50
model.settings.particles = 2000

# Create an initial uniform spatial source distribution over fissionable zones
lower_left = (-pitch/2, -pitch/2, 0.0)
upper_right = (pitch/2, pitch/2, height)
uniform_dist = openmc.stats.Box(lower_left, upper_right)
model.settings.source = openmc.IndependentSource(space=uniform_dist)

model.settings.temperature = {'default': 280.0 + 273.15,
                        'method': 'interpolation',
                        'range': (294.0, 3000.0),
                        'tolerance': 1000.0}

model.export_to_model_xml()

