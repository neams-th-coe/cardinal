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

# Add materials for the pebbles and the FLiBe

uo2 = openmc.Material()
uo2.set_density('g/cc', 10.0)
uo2.add_element('U', 1.0, enrichment=5.0)
uo2.add_element('O', 2.0)

enrichment_li7 = 0.99995
flibe = openmc.Material(name='2LiF-BeF2')
flibe.set_density('kg/m3', 1960)
flibe.add_nuclide('Li7', 2.0*enrichment_li7)
flibe.add_nuclide('Li6', 2.0*(1 - enrichment_li7))
flibe.add_element('Be', 1.0)
flibe.add_element('F', 4.0)

mats = openmc.Materials([uo2, flibe])
mats.export_to_xml()

model = openmc.model.Model()

# create a universe containing the repeatable universe of
# a single pebble plus surrounding flibe
R = 1.5
L = 4.0
sphere_surface = openmc.Sphere(r=R, name='Sphere surface')
flibe_surface = openmc.model.RectangularPrism(L, L, boundary_type='reflective')

sphere_cell = openmc.Cell(fill=uo2, region=-sphere_surface, name='Pebble')
flibe_cell = openmc.Cell(fill=flibe, region=+sphere_surface & -flibe_surface, name='Flibe')
repeatable_univ = openmc.Universe(cells=[sphere_cell, flibe_cell])

outer_cell = openmc.Cell(fill=flibe, region=+sphere_surface & -flibe_surface, name='Outside')
outer_univ = openmc.Universe(cells=[outer_cell])

# create a lattice to repeat the pebble + flibe universe
N = 3
lattice = openmc.RectLattice()
lattice.lower_left = (-L/2.0, -L/2.0, 0)
lattice.pitch = (L, L, L)
lattice.universes = np.full((N, 1, 1), repeatable_univ)
lattice.outer = outer_univ

# create the surfaces that will bound the lattice
top = openmc.ZPlane(z0=N*L, boundary_type='reflective')
bottom = openmc.ZPlane(z0=0.0, boundary_type='reflective')
main_cell = openmc.Cell(fill=lattice, region=-flibe_surface & +bottom & -top, name='Main cell')

model.geometry = openmc.Geometry([main_cell])
model.geometry.export_to_xml()

# Finally, define some run settings
model.settings = openmc.Settings()
model.settings.batches = 1000
model.settings.inactive = 100
model.settings.particles = 10000

lower_left = (-L, -L, 0)
upper_right = (L, L, N*L)
uniform_dist = openmc.stats.Box(lower_left, upper_right, only_fissionable=True)
model.settings.source = openmc.IndependentSource(space=uniform_dist)
model.settings.temperature = {'default': 650.0 + 273.15,
                              'method': 'interpolation',
                              'multipole': False,
                              'tolerance': 1000.0,
                              'range': (294.0, 1600.0)}
model.settings.export_to_xml()

# create some plots for making images in the tutorial

height = N*L
plot1          = openmc.Plot()
plot1.filename = 'plot1'
plot1.width    = (L, height)
plot1.basis    = 'xz'
plot1.origin   = (0.0, 0.0, height/2.0)
plot1.pixels   = (100, 300)
plot1.color_by = 'cell'

plot2          = openmc.Plot()
plot2.filename = 'plot2'
plot2.width    = (L, L)
plot2.basis    = 'xy'
plot2.origin   = (0.0, 0.0, 2.0)
plot2.pixels   = (100, 100)
plot2.color_by = 'cell'

plots = openmc.Plots([plot1, plot2])
plots.export_to_xml()
