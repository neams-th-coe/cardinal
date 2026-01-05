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

# whether to do the criticality search natively with OpenMC
search = False

model = openmc.Model()

u = openmc.Material()
u.set_density('g/cc', 22.0)
u.add_nuclide('U234', 0.001)
u.add_nuclide('U235', 0.1)
u.add_nuclide('U238', 1.0)
u.add_nuclide('H3', 1e-4)

# to get this input file to match the Cardinal test, the density needs to be set
# as 1.9 instead of 1.0. The test checks that the criticality search works when
# a density change via Cardinal is simultaneously imposed.
dens = 1.0
if (search):
  dens = 1.9
bw = openmc.model.borated_water(boron_ppm=1000.0, density=dens)

# Create two cubes side by side
R = 10.0
l = openmc.XPlane(x0=0, boundary_type='reflective')
r = openmc.XPlane(x0=R)
b = openmc.YPlane(y0=0, boundary_type='reflective')
t = openmc.YPlane(y0=R, boundary_type='reflective')
f = openmc.ZPlane(z0=0, boundary_type='reflective')
k = openmc.ZPlane(z0=R, boundary_type='reflective')
z = openmc.XPlane(x0=2*R, boundary_type='reflective')

# create a box
box1 = openmc.Cell(region=+l & -r & +b & -t & +f & -k, fill=u)
box2 = openmc.Cell(region=+r & -z & +b & -t & +f & -k, fill=bw)
model.geometry = openmc.Geometry([box1, box2])

# Finally, define some run settings
model.settings.batches = 10
model.settings.inactive = 3
model.settings.particles = 10000

lower_left = (0, 0, 0)
upper_right = (R, R, R)
uniform_dist = openmc.stats.Box(lower_left, upper_right)
model.settings.source = openmc.source.IndependentSource(space=uniform_dist)
model.settings.temperature = {'default': 600.0,
                        'method': 'nearest',
                        'range': (294.0, 1600.0)}

model.export_to_model_xml()

def borated_water_ppm(ppm):
  bw = openmc.model.borated_water(boron_ppm=ppm, density=dens)
  box2.fill = bw

if (search):
  result = model.keff_search(borated_water_ppm, x0=0, x1=1000, output=True, k_tol=1e-3, sigma_final=1e-3)
