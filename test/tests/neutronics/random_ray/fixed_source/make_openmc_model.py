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
import openmc.mgxs
import numpy as np

model = openmc.Model()

a0 = openmc.Material()
a0.set_density('kg/m3', 1000.0)
a0.add_nuclide('H1', 2.0)
a0.add_nuclide('O16', 1.0)

model.materials = openmc.Materials([a0])

prism = openmc.model.RectangularPrism(width=50.0, height=50.0, boundary_type='reflective')
top = openmc.ZPlane(z0=50.0, boundary_type='vacuum')
bot = openmc.ZPlane(z0=-50.0, boundary_type='vacuum')
split_1 = openmc.ZPlane(z0=-1.0)
split_2 = openmc.ZPlane(z0=1.0)

cell1 = openmc.Cell(region=-prism & +bot & -split_1, fill=a0)
cell2 = openmc.Cell(region=-prism & +split_1 & -split_2, fill=a0)
cell3 = openmc.Cell(region=-prism & +split_2 & -top, fill=a0)

model.geometry = openmc.Geometry([cell1, cell2, cell3])

model.settings.ptables = True
model.settings.temperature['method'] = 'interpolation'
model.settings.temperature['tolerance'] = 5000.0
model.settings.temperature['range'] = (300.0, 2000.0)
model.settings.temperature['multipole'] = True
model.settings.temperature['default'] = 650.0

model.settings.batches = 100
model.settings.inactive = 10
model.settings.particles = 1000

src = openmc.IndependentSource(space=openmc.stats.Box(lower_left = (-25.0, -25.0, -1.0),
                                                      upper_right = (25.0, 25.0, 1.0)),
                               energy=openmc.stats.Discrete(x=[1e3], p=[1.0]))
src.constraints['domain_type'] = 'cell'
src.constraints['domain_ids'] = [cell2.id]
model.settings.source.append(src)
model.settings.run_mode = 'fixed source'

model.convert_to_multigroup(
  method = 'material_wise',
  groups = openmc.mgxs.EnergyGroups(np.array([1e-8, 1e8])),
  nparticles = 10000,
  overwrite_mgxs_library = True
)

model.convert_to_random_ray()

sr_mesh = openmc.RegularMesh().from_domain(model.geometry)
sr_mesh.dimension = (10, 10, 10)
model.settings.random_ray['source_region_meshes'] = [(sr_mesh, [cell1, cell2])]

model.export_to_model_xml()
