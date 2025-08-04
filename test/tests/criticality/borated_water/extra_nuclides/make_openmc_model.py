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
u.set_density('g/cc', 10.0)
u.add_element('U', 1.0)
u.add_nuclide('H3', 1.0)

bw = openmc.model.borated_water(boron_ppm=1000, density=0.8)

model.materials = openmc.Materials([u, bw])

# Create two cubes side by side
R = 10.0
l = openmc.XPlane(x0=0, boundary_type='vacuum')
r = openmc.XPlane(x0=R)
b = openmc.YPlane(y0=0, boundary_type='vacuum')
t = openmc.YPlane(y0=R, boundary_type='vacuum')
f = openmc.ZPlane(z0=0, boundary_type='vacuum')
k = openmc.ZPlane(z0=R, boundary_type='vacuum')
z = openmc.XPlane(x0=2*R, boundary_type='vacuum')

# create a box
box1 = openmc.Cell(region=+l & -r & +b & -t & +f & -k, fill=u)
box2 = openmc.Cell(region=+r & -z & +b & -t & +f & -k, fill=bw)
model.geometry = openmc.Geometry([box1, box2])

# Finally, define some run settings
model.settings.batches = 50
model.settings.inactive = 10
model.settings.particles = 100

lower_left = (0, 0, 0)
upper_right = (R, R, R)
uniform_dist = openmc.stats.Box(lower_left, upper_right)
model.settings.source = openmc.source.IndependentSource(space=uniform_dist)
model.export_to_xml()
