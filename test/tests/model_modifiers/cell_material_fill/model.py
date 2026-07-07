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

model = openmc.Model()

mat1 = openmc.Material(name='U')
mat1.set_density('g/cm3', 10.29769)
mat1.add_element('U', 1.0)

mat2 = openmc.Material(name='C')
mat2.set_density('g/cm3', 3.0)
mat2.add_element('C', 1.0)

water = openmc.Material(name='water')
water.set_density('g/cm3', 1.0)
water.add_element('H', 2.0)
water.add_element('O', 1.0)

cyl = openmc.ZCylinder(r=5.0)

cell1_q1 = openmc.Cell(region=-cyl, fill=mat1)

# change fill to mat1 to locally get results to compare with Cardinal test
cell2_q1 = openmc.Cell(region=+cyl, fill=water)

universe1 = openmc.Universe(cells=[cell1_q1, cell2_q1])

cell1_q2 = openmc.Cell(region=-cyl, fill=mat2)
cell2_q2 = openmc.Cell(region=+cyl, fill=water)
universe2 = openmc.Universe(cells=[cell1_q2, cell2_q2])

# create the lattice
pitch = 10.0
lattice = openmc.RectLattice()
lattice.lower_left = (0, 0)
lattice.pitch = (pitch, pitch)
lattice.universes = [[universe1, universe1], \
                     [universe2, universe1]]

box = openmc.model.RectangularParallelepiped(0, 2*pitch, 0, 2*pitch, 0, 100.0, boundary_type='vacuum')
main_cell = openmc.Cell(fill=lattice, region=-box)
model.geometry = openmc.Geometry([main_cell])

model.settings = openmc.Settings()
model.settings.batches = 10
model.settings.inactive = 5
model.settings.particles = 1000

# Create an initial uniform spatial source distribution over fissionable zones
lower_left = (0, 0, 0)
upper_right = (2*pitch, 2*pitch, 100.0)
uniform_dist = openmc.stats.Box(lower_left, upper_right)
model.settings.source = openmc.IndependentSource(space=uniform_dist)

model.export_to_model_xml()
