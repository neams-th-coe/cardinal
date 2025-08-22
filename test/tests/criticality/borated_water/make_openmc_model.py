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

model = openmc.Model()

u = openmc.Material()
u.set_density('g/cc', 22.0)
u.add_nuclide('U234', 0.001)
u.add_nuclide('U235', 0.1)
u.add_nuclide('U238', 1.0)
u.add_nuclide('H3', 1e-4)

bw = openmc.model.borated_water(boron_ppm=3000, density=0.8)
h1_percent = bw.nuclides[0].percent
for n in bw.nuclides:
  print(n.name, n.percent / h1_percent)

model.materials = openmc.Materials([u, bw])

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
model.settings.particles = 1000

lower_left = (0, 0, 0)
upper_right = (R, R, R)
uniform_dist = openmc.stats.Box(lower_left, upper_right)
model.settings.source = openmc.source.IndependentSource(space=uniform_dist)
model.settings.temperature = {'default': 600.0,
                        'method': 'nearest',
                        'range': (294.0, 1600.0)}

model.export_to_xml()
