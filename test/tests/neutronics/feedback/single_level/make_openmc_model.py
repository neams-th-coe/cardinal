#********************************************************************/
#*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
#*                             Cardinal                             */
#*                                                                  */
#*                  (c) 2021 UChicago Argonne, LLC                  */
#*                        ALL RIGHTS RESERVED                       */
#*                                                                  */
#*                 Prepared by UChicago Argonne, LLC                */
#*               Under Contract No. DE-AC02-06CH11357               */
#*                With the U. S. Department of Energy               */
#*                                                                  */
#*             Prepared by Battelle Energy Alliance, LLC            */
#*               Under Contract No. DE-AC07-05ID14517               */
#*                With the U. S. Department of Energy               */
#*                                                                  */
#*                 See LICENSE for full restrictions                */
#********************************************************************/

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
all_materials.append(uo2)

zircaloy = openmc.Material(material_id=3, name='Zircaloy')
zircaloy.set_density('g/cm3', 6.55)
zircaloy.add_element('Zr', 1.0)
all_materials.append(zircaloy)

# In order to be able to change the water density, we'll need to create
# 10 materials for the water
water_materials = []
for i in range(N):
  water = openmc.Material(name='water{:n}'.format(i))
  water.set_density('g/cm3', water_density(T_inlet))
  water.add_element('H', 2.0)
  water.add_element('O', 1.0)
  water.add_s_alpha_beta('c_H_in_H2O')
  water_materials.append(water)
  all_materials.append(water)

all_materials.export_to_xml()

model = openmc.model.Model()

# create all the surfaces
pincell_surface = openmc.ZCylinder(r=R, name='Pincell outer radius')
pellet_surface = openmc.ZCylinder(r=Rf, name='Pellet outer radius')
water_surface = openmc.ZCylinder(r=pitch / 2.0, name='Water surface', boundary_type='white')

# loop over all the surfaces needed to define the pincell (N + 1 for the N layers)
planes = np.linspace(0.0, height, N + 1)

plane_surfaces = []
for i in range(N + 1):
  plane_surfaces.append(openmc.ZPlane(z0=planes[i]))

# set the boundary condition on the topmost and bottommost planes to vacuum
plane_surfaces[0].boundary_type = 'vacuum'
plane_surfaces[-1].boundary_type = 'vacuum'

fuel_cells = []
clad_cells = []
water_cells = []
all_cells = []
for i in range(N):
  water_material = water_materials[i]
  layer = +plane_surfaces[i] & -plane_surfaces[i + 1]
  fuel_cells.append(openmc.Cell(fill=uo2, region=-pellet_surface & layer, name='Fuel{:n}'.format(i)))
  clad_cells.append(openmc.Cell(fill=zircaloy, region=+pellet_surface & -pincell_surface & layer, name='Clad{:n}'.format(i)))
  water_cells.append(openmc.Cell(fill=water_material, region=+pincell_surface & layer & -water_surface, name='Water{:n}'.format(i)))
  all_cells.append(fuel_cells[i])
  all_cells.append(clad_cells[i])
  all_cells.append(water_cells[i])

model.geometry = openmc.Geometry(all_cells)

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
