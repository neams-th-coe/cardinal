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

pu = openmc.Material()
pu.set_density('g/cc', 16.37749)
pu.add_nuclide('Pu239', 1.0)

model.materials = openmc.Materials([pu])

# Create cube
R = 10.0
l = openmc.XPlane(x0=0, boundary_type='vacuum')
r = openmc.XPlane(x0=R, boundary_type='vacuum')
b = openmc.YPlane(y0=0, boundary_type='vacuum')
t = openmc.YPlane(y0=R, boundary_type='vacuum')
f = openmc.ZPlane(z0=0, boundary_type='vacuum')
k = openmc.ZPlane(z0=R, boundary_type='vacuum')

# create a box
box = openmc.Cell(region=+l & -r & +b & -t & +f & -k, fill=pu)
model.geometry = openmc.Geometry([box])

# Finally, define some run settings
model.settings.batches = 50
model.settings.inactive = 10
model.settings.particles = 10000

lower_left = (0, 0, 0)
upper_right = (R, R, R)
uniform_dist = openmc.stats.Box(lower_left, upper_right)
model.settings.source = openmc.source.IndependentSource(space=uniform_dist)
model.settings.temperature = {'default': 600.0,
                        'method': 'nearest',
                        'range': (294.0, 1600.0)}

model.export_to_xml()
