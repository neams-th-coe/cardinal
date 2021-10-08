import math
import numpy as np
import openmc

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

all_materials = openmc.Materials()

uo2 = openmc.Material(name='UO2')
uo2.set_density('g/cm3', 10.29769)
uo2.add_nuclide('U235', 0.05)
uo2.add_nuclide('U238', 0.95)
uo2.add_element('O', 2.0)
uo2.temperature = 500.0
all_materials.append(uo2)

zircaloy = openmc.Material(material_id=3, name='Zircaloy')
zircaloy.set_density('g/cm3', 6.55)
zircaloy.add_element('Zr', 1.0)
zircaloy.temperature = 450.0
all_materials.append(zircaloy)

water = openmc.Material(name='water')
water.set_density('g/cm3', water_density(T_inlet))
water.add_element('H', 2.0)
water.add_element('O', 1.0)
water.add_s_alpha_beta('c_H_in_H2O')
water.temperature = 300.0
all_materials.append(water)

all_materials.export_to_xml()

model = openmc.model.Model()

# create a universe containing the repeatable universe that will be used to fill the lattice
pincell_surface = openmc.ZCylinder(r=R, name='Pincell outer radius')
pellet_surface = openmc.ZCylinder(r=Rf, name='Pellet outer radius')

fuel_cell = openmc.Cell(fill=uo2, region=-pellet_surface, name='Fuel')
clad_cell = openmc.Cell(fill=zircaloy, region=+pellet_surface & -pincell_surface, name='Clad')
repeatable_univ = openmc.Universe(cells=[fuel_cell, clad_cell])

outer_cell = openmc.Cell(fill=water, name='Outside')
outer_univ = openmc.Universe(cells=[outer_cell])

# create the lattice
lattice = openmc.RectLattice()
lattice.lower_left = (-pitch / 2.0, -pitch / 2.0, 0)
lattice.pitch = (pitch, pitch, H)
lattice.universes = np.full((N, 1, 1), repeatable_univ)
lattice.outer = outer_univ

surface_to_enclose_lattice = openmc.ZCylinder(r=R)
top = openmc.ZPlane(z0=height, boundary_type='vacuum')
bottom = openmc.ZPlane(z0=0.0, boundary_type='vacuum')

water_surface = openmc.rectangular_prism(width=pitch, height=pitch, boundary_type='white')
lattice_cell = openmc.Cell(fill=lattice, region=-surface_to_enclose_lattice & +bottom & -top)
water_cell = openmc.Cell(fill=water, region=+surface_to_enclose_lattice & water_surface & +bottom & -top, name='Water')

model.geometry = openmc.Geometry([water_cell, lattice_cell])

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
