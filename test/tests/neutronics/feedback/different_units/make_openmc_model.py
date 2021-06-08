import math
import numpy as np
import openmc

R = 100.0                 # outer radius of the sphere (cm)
T_inlet = 573.0           # inlet water temperature (K)
N = 8

#######################

all_materials = openmc.Materials()

enrichments = np.linspace(0.0, 0.20, N)

for i in range(len(enrichments)):
  uo2 = openmc.Material(name='UO2{:n}'.format(i))
  uo2.set_density('g/cm3', 10.29769)
  uo2.add_nuclide('U235', enrichments[i])
  uo2.add_nuclide('U238', 1.0 - enrichments[i])
  uo2.add_element('O', 2.0)
  all_materials.append(uo2)

all_materials.export_to_xml()

model = openmc.model.Model()

# create all the surfaces
sphere_surface = openmc.Sphere(r=R, boundary_type='white')
plane_surface1 = openmc.XPlane(x0 = 0)
plane_surface2 = openmc.YPlane(y0 = 0)
plane_surface3 = openmc.ZPlane(z0 = 0)

quadrants = [-plane_surface1 & -plane_surface2 &  -plane_surface3,
             -plane_surface1 & -plane_surface2 &  +plane_surface3,
             -plane_surface1 & +plane_surface2 &  -plane_surface3,
             -plane_surface1 & +plane_surface2 &  +plane_surface3,
             +plane_surface1 & -plane_surface2 &  -plane_surface3,
             +plane_surface1 & -plane_surface2 &  +plane_surface3,
             +plane_surface1 & +plane_surface2 &  -plane_surface3,
             +plane_surface1 & +plane_surface2 &  +plane_surface3]

all_cells = []
for i in range(N):
  quadrant = quadrants[i]
  all_cells.append(openmc.Cell(fill=all_materials[i], region=-sphere_surface & quadrant, name='Fuel{:n}'.format(i)))

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
lower_left = (-R, -R, -R)
upper_right = (R, R, R)
uniform_dist = openmc.stats.Box(lower_left, upper_right, only_fissionable=True)
model.settings.source = openmc.source.Source(space=uniform_dist)

model.export_to_xml()
