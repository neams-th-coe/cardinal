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

import openmc
import numpy as np
from argparse import ArgumentParser

ap = ArgumentParser()
ap.add_argument('-n', dest='n_axial', type=int, default=40,
                help='Number of axial cell divisions')
ap.add_argument('-s', '--entropy', action='store_true',
                help='Whether to add a Shannon entropy mesh')
args = ap.parse_args()

N = args.n_axial # Number of axial cells to build in the solid to receive feedback
height = 300.0   # Total height of pincell

###############################################################################
# Create materials for the problem

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

# Collect the materials together and export to XML
materials = openmc.Materials([uo2, helium, zircaloy, borated_water])
materials.export_to_xml()

###############################################################################
# Define problem geometry

# Create cylindrical surfaces
fuel_or = openmc.ZCylinder(r=0.39218, name='Fuel OR')
clad_ir = openmc.ZCylinder(r=0.40005, name='Clad IR')
clad_or = openmc.ZCylinder(r=0.45720, name='Clad OR')

# Create a region represented as the inside of a rectangular prism
pitch = 1.25984
box = openmc.rectangular_prism(pitch, pitch, boundary_type='reflective')

# Create cells, mapping materials to regions - split up the axial height
planes = np.linspace(0.0, height, N + 1)
plane_surfaces = []
for i in range(N + 1):
  plane_surfaces.append(openmc.ZPlane(z0=planes[i]))

# set the boundary condition on the topmost and bottommost planes to vacuum
plane_surfaces[0].boundary_type = 'vacuum'
plane_surfaces[-1].boundary_type = 'vacuum'

fuel_cells = []
clad_cells = []
gap_cells = []
water_cells = []
all_cells = []
for i in range(N):
  layer = +plane_surfaces[i] & -plane_surfaces[i + 1]
  fuel_cells.append(openmc.Cell(fill=uo2, region=-fuel_or & layer, name='Fuel{:n}'.format(i)))
  gap_cells.append(openmc.Cell(fill=helium, region=+fuel_or & -clad_ir & layer, name='Gap{:n}'.format(i)))
  clad_cells.append(openmc.Cell(fill=zircaloy, region=+clad_ir & -clad_or & layer, name='Clad{:n}'.format(i)))
  water_cells.append(openmc.Cell(fill=borated_water, region=+clad_or & layer & box, name='Water{:n}'.format(i)))
  all_cells.append(fuel_cells[i])
  all_cells.append(gap_cells[i])
  all_cells.append(clad_cells[i])
  all_cells.append(water_cells[i])

# Create a geometry and export to XML
geometry = openmc.Geometry(all_cells)
geometry.export_to_xml()

###############################################################################
# Define problem settings

# Indicate how many particles to run
settings = openmc.Settings()
settings.batches = 1500
settings.inactive = 500
settings.particles = 20000

# Create an initial uniform spatial source distribution over fissionable zones
lower_left = (-pitch/2, -pitch/2, 0.0)
upper_right = (pitch/2, pitch/2, height)
uniform_dist = openmc.stats.Box(lower_left, upper_right, only_fissionable=True)
settings.source = openmc.source.Source(space=uniform_dist)

if (args.entropy):
  entropy_mesh = openmc.RegularMesh()
  entropy_mesh.lower_left = lower_left
  entropy_mesh.upper_right = upper_right
  entropy_mesh.dimension = (10, 10, 20)
  settings.entropy_mesh = entropy_mesh

settings.temperature = {'default': 280.0 + 273.15,
                        'method': 'interpolation',
                        'range': (294.0, 3000.0),
                        'tolerance': 1000.0}

settings.export_to_xml()

# create some plots to look at the geometry for the sake of the tutorial
plot1          = openmc.Plot()
plot1.filename = 'plot1'
plot1.width    = (pitch, pitch)
plot1.basis    = 'xy'
plot1.origin   = (0.0, 0.0, height/2.0)
plot1.pixels   = (1000, 1000)
plot1.color_by = 'cell'

plot2          = openmc.Plot()
plot2.filename = 'plot2'
plot2.width    = (pitch, height)
plot2.basis    = 'xz'
plot2.origin   = (0.0, 0.0, height/2.0)
plot2.pixels   = (100, int(100 * (height/2.0/pitch)))
plot2.color_by = 'cell'

plots = openmc.Plots([plot1, plot2])
plots.export_to_xml()

