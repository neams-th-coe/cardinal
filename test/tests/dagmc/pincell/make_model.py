import openmc

 # materials
u235 = openmc.Material(name="fuel")
u235.add_nuclide('U235', 1.0, 'ao')
u235.set_density('g/cc', 11)
u235.id = 40

water = openmc.Material(name="water")
water.add_nuclide('H1', 2.0, 'ao')
water.add_nuclide('O16', 1.0, 'ao')
water.set_density('g/cc', 1.0)
water.add_s_alpha_beta('c_H_in_H2O')
water.id = 41

mats = openmc.Materials([u235, water])
mats.export_to_xml()

dagmc_univ = openmc.DAGMCUniverse(filename="dagmc.h5m")
geometry = openmc.Geometry(root=dagmc_univ)
geometry.export_to_xml()

settings = openmc.Settings()
settings.dagmc = True
settings.batches = 10
settings.inactive = 2
settings.particles = 5000
settings.export_to_xml()

settings.source = openmc.Source(space=openmc.stats.Box([-4., -4., -4.],
                                                       [ 4.,  4.,  4.]))
settings.export_to_xml()
