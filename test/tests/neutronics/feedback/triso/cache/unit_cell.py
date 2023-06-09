#!/bin/env python
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

import numpy as np
import openmc

m = 100.0

h = 1.0 * m
l = 0.2 * m
n = 10

T = 1000.0

model = openmc.model.Model()

### Materials ###
fuel = openmc.Material(name='fuel')
fuel.add_nuclide('U235', 0.0001)
fuel.add_element('O'   , 0.50)
fuel.set_density('kg/m3', 1000.0)

water = openmc.Material(name='water')
water.add_element('H', 2.0)
water.add_element('O', 1.0)
water.set_density('kg/m3', 100.0)

outer = openmc.rectangular_prism(l, l, boundary_type='reflective')
split = openmc.XPlane(x0=0.0)
axial_coords = np.linspace(0.0, h, n + 1)

# create additional axial regions
axial_planes = [openmc.ZPlane(z0=coord) for coord in axial_coords]

# axial planes
min_z = axial_planes[0]
min_z.boundary_type = 'reflective'
max_z = axial_planes[-1]
max_z.boundary_type = 'reflective'

c = []
for i in range(n):
  layer = +axial_planes[i] & -axial_planes[i + 1]

  if (i == n - 1):
    fuel_cell = openmc.Cell(region=-split & layer & outer, fill=fuel)
    fuel_cell.temperature = T
    water_cell = openmc.Cell(region=+split & layer & outer, fill=water)
    water_cell.temperature = T
    c.append(fuel_cell)
    c.append(water_cell)
  else:
    fuel_cell = openmc.Cell(region=-split, fill=fuel)
    fuel_cell.temperature = T
    water_cell = openmc.Cell(region=+split, fill=water)
    water_cell.temperature = T
    univ = openmc.Universe(cells=[fuel_cell, water_cell])
    c.append(openmc.Cell(region=layer & outer, fill=univ))

U = openmc.Universe(cells=c)
model.geometry = openmc.Geometry(U)

### Settings ###
settings = openmc.Settings()

settings.particles = 1000
settings.inactive = 5
settings.batches = 10
settings.temperature['method'] = 'nearest'
settings.temperature['range'] = (294.0, 1500.0)
settings.temperature['tolerance'] = 200.0

source_dist = openmc.stats.Box((-l/2., -l/2., 0.0), (l/2., l/2., h))
source = openmc.Source(space=source_dist)
settings.source = source

model.settings = settings

model.export_to_xml()
