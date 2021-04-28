import math
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

import openmc
import openmc.lib

def water_density(T):
  """
  Returns water density at a temperature T (K) in units of g/cm3
  """
  rho = 0.001 * (0.14395 / math.pow(0.0112, 1.0 + math.pow(1.0 - T / 649.727, 0.05107)))
  return rho

R = 0.97 / 2.0           # outer radius of the pincell (cm)
Rf = 0.825 / 2.0         # outer radius of the pellet (cm)
pitch = 1.28             # pitch between pincells (cm)
height = 10.0            # height of the pincell (cm)
N = 10                   # number of axial layers for multiphysics coupling
T_inlet = 573.0          # inlet water temperature (K)
H = height / N

#######################

all_materials = []

uo2 = openmc.Material(name = 'UO2')
uo2.set_density('g/cm3', 10.29769)
uo2.add_nuclide('U235', 0.05)
uo2.add_nuclide('U238', 0.95)
uo2.add_element('O', 2.0)
all_materials.append(uo2)

zircaloy = openmc.Material(material_id=3, name='Zircaloy')
zircaloy.set_density('g/cm3', 6.55)
zircaloy.add_element('Zr', 1.0)
all_materials.append(zircaloy)

# In order to be able to change the water density, we'll need to create
# 10 materials for the water
water_materials = []
for i in range(N):
  water = openmc.Material(name = 'water{:n}'.format(i))
  water.set_density('g/cm3', water_density(T_inlet))
  water.add_element('H', 2.0)
  water.add_element('O', 1.0)
  water.add_s_alpha_beta('c_H_in_H2O')
  water_materials.append(water)
  all_materials.append(water)

materials = openmc.Materials(all_materials)
materials.export_to_xml()

model = openmc.model.Model()

# create a universe containing the repeatable universe that will be used to fill the lattice
pincell_surface = openmc.ZCylinder(r = R, name = 'Pincell outer radius')
pellet_surface = openmc.ZCylinder(r = Rf, name = 'Pellet outer radius')
water_surface = openmc.ZCylinder(r = pitch / 2.0, name = 'Water surface', boundary_type = 'white')
fuel_cell = openmc.Cell(fill = uo2, region = -pellet_surface, name = 'Fuel')
clad_cell = openmc.Cell(fill = zircaloy, region = +pellet_surface & -pincell_surface, name = 'Clad')
water_cell = openmc.Cell(fill = water_materials, region = +pincell_surface & -water_surface, name = 'Water')
repeatable_univ = openmc.Universe(cells = [fuel_cell, clad_cell, water_cell])

outer_cell = openmc.Cell(fill = water_materials[0], region = +pincell_surface & -water_surface, name = 'Outside')
outer_univ = openmc.Universe(cells = [outer_cell])

# create the lattice
lattice = openmc.RectLattice()
lattice.lower_left = (-pitch / 2.0, -pitch / 2.0, 0)
lattice.pitch = (pitch, pitch, H)
lattice.universes = np.full((N, 1, 1), repeatable_univ)
lattice.outer = outer_univ

top = openmc.ZPlane(z0 = height, boundary_type = 'vacuum')
bottom = openmc.ZPlane(z0 = 0.0, boundary_type = 'vacuum')
main_cell = openmc.Cell(fill = lattice, region = -water_surface & +bottom & -top)

model.geometry = openmc.Geometry([main_cell])

model.settings = openmc.Settings()
model.settings.batches = 5
model.settings.inactive = 2
model.settings.particles = 1000
model.settings.temperature = {'default': T_inlet,
                        'method': 'nearest',
                        'range': (294.0, 3000.0),
                        'tolerance': 1000.0}

# Create an initial uniform spatial source distribution over fissionable zones
lower_left = (-pitch, -pitch, 0.0)
upper_right = (pitch, pitch, height)
uniform_dist = openmc.stats.Box(lower_left, upper_right, only_fissionable=True)
model.settings.source = openmc.source.Source(space=uniform_dist)

model.export_to_xml()
