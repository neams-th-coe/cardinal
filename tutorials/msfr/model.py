import openmc

salt = openmc.Material(name='salt')
salt.set_density('g/cm3', 4.1249)
salt.add_nuclide('Li7', 0.779961, 'ao')
salt.add_nuclide('Li6', 3.89999999999957e-05, 'ao')
salt.add_nuclide('U233', 0.02515, 'ao')
salt.add_element('F', 1.66, 'ao')
salt.add_element('Th', 0.19985, 'ao')
materials = openmc.Materials([salt])
materials.export_to_xml()

dagmc_univ = openmc.DAGMCUniverse(filename="msr.h5m")
geometry = openmc.Geometry(root=dagmc_univ)
geometry.export_to_xml()

settings = openmc.Settings()
lower_left = (-100.0, -100.0, -100.0)
upper_right = (100.0, 100.0, 100.0)
uniform_dist = openmc.stats.Box(lower_left, upper_right)

settings.source = openmc.source.Source(space=uniform_dist)

settings.batches = 1500
settings.inactive = 500
settings.particles = 50000

settings.temperature = {'default': 898.0,
                        'method': 'interpolation',
                        'multipole': True,
                        'range': (294.0, 3000.0)}

settings.export_to_xml()

