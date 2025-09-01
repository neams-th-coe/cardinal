from argparse import ArgumentParser
import numpy as np
import openmc

R = 0.97 / 2.0           # outer radius of the pincell (cm)
Rf = 0.825 / 2.0         # outer radius of the pellet (cm)
pitch = 1.28             # pitch between pincells (cm)
height = 50.0            # height of the pincell (cm)
T_inlet = 573.0          # inlet sodium temperature (K)

ap = ArgumentParser()
ap.add_argument('-n', dest='n_axial', type=int, default=25,
                help='Number of axial cell divisions')

args = ap.parse_args()
N = args.n_axial

all_materials = []

# Then, add the materials for UO2 and zircaloy
uo2 = openmc.Material(name = 'UO2')
uo2.set_density('g/cm3', 10.29769)
uo2.add_element('U', 1.0, enrichment = 2.5)
uo2.add_element('O', 2.0)
all_materials.append(uo2)

zircaloy = openmc.Material(material_id=3, name='Zircaloy 4')
zircaloy.set_density('g/cm3', 6.55)
zircaloy.add_element('Sn', 0.014, 'wo')
zircaloy.add_element('Fe', 0.00165, 'wo')
zircaloy.add_element('Cr', 0.001, 'wo')
zircaloy.add_element('Zr', 0.98335, 'wo')
all_materials.append(zircaloy)

sodium = openmc.Material(name = 'sodium')
sodium.set_density('g/cm3', 1.0)
sodium.add_element('Na', 1.0)
all_materials.append(sodium)

materials = openmc.Materials(all_materials)
materials.export_to_xml()

pincell_surface = openmc.ZCylinder(r = R, name = 'Pincell outer radius')
pellet_surface = openmc.ZCylinder(r = Rf, name = 'Pellet outer radius')
rectangular_prism = openmc.model.RectangularPrism(width = pitch, height = pitch, axis = 'z', origin = (0.0, 0.0), boundary_type = 'reflective')

axial_coords = np.linspace(0.0, height, N + 1)
plane_surfaces = [openmc.ZPlane(z0=coord) for coord in axial_coords]
plane_surfaces[0].boundary_type = 'vacuum'
plane_surfaces[-1].boundary_type = 'vacuum'

fuel_cells = []
clad_cells = []
sodium_cells = []

for i in range(N):
  # these are the two planes that bound the current layer on top and bottom
  layer = +plane_surfaces[i] & -plane_surfaces[i + 1]

  fuel_cells.append(openmc.Cell(fill = uo2, region = -pellet_surface & layer, name = 'Fuel{:n}'.format(i)))
  clad_cells.append(openmc.Cell(fill = zircaloy, region = +pellet_surface & -pincell_surface & layer, name = 'Clad{:n}'.format(i)))
  sodium_cells.append(openmc.Cell(fill = sodium, region = +pincell_surface & layer & -rectangular_prism, name = 'sodium{:n}'.format(i)))

root = openmc.Universe(name = 'root')
root.add_cells(fuel_cells)
root.add_cells(clad_cells)
root.add_cells(sodium_cells)

geometry = openmc.Geometry(root)
geometry.export_to_xml()

settings = openmc.Settings()

# Create an initial uniform spatial source distribution over fissionable zones
lower_left = (-pitch, -pitch, 0.0)
upper_right = (pitch, pitch, height)
uniform_dist = openmc.stats.Box(lower_left, upper_right)

settings.source = openmc.IndependentSource(space=uniform_dist)

settings.batches = 50
settings.inactive = 10
settings.particles = 10000

settings.temperature = {'default': T_inlet,
                        'method': 'interpolation',
                        'multipole': True,
                        'range': (294.0, 3000.0)}

settings.export_to_xml()
