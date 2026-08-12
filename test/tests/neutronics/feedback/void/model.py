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
rho = 1000.0

model = openmc.Model()

### Materials ###
fuel = openmc.Material(name='fuel')
fuel.add_nuclide('U235', 0.0001)
fuel.add_element('O'   , 0.50)
fuel.set_density('kg/m3', rho)

outer = openmc.model.RectangularPrism(l, l, boundary_type='reflective')
split = openmc.XPlane(x0=0.0)
axial_coords = np.linspace(0.0, h, n + 1)

# create additional axial regions
axial_planes = [openmc.ZPlane(z0=coord) for coord in axial_coords]

# axial planes
min_z = axial_planes[0]
min_z.boundary_type = 'reflective'
max_z = axial_planes[-1]
max_z.boundary_type = 'reflective'

fuel_cell = openmc.Cell(region=-split, fill=fuel)
fuel_cell.temperature = T
water_cell = openmc.Cell(region=+split, fill=None)
water_cell.temperature = T
univ = openmc.Universe(cells=[fuel_cell, water_cell])

# fill the universe into a lattice
lattice = openmc.RectLattice()
lattice.lower_left = (-l/2, -l/2, 0)
lattice.pitch = (l, l, h/n)
lattice.universes = np.full((n, 1, 1), univ)

c = []
c.append(openmc.Cell(region=-outer & +min_z & -max_z, fill=lattice))

U = openmc.Universe(cells=c)
model.geometry = openmc.Geometry(U)

### Settings ###
model.settings.particles = 1000
model.settings.inactive = 5
model.settings.batches = 10
model.settings.temperature['method'] = 'nearest'
model.settings.temperature['range'] = (294.0, 1500.0)
model.settings.temperature['tolerance'] = 200.0

source_dist = openmc.stats.Box((-l/2., -l/2., 0.0), (l/2., l/2., h))
source = openmc.IndependentSource(space=source_dist)
model.settings.source = source

model.export_to_model_xml()
