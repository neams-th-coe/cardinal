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

settings.temperature = {'default': 500.0,
                        'method': 'interpolation',
                        'range': (294.0, 3000.0),
                        'tolerance': 1000.0}

settings.export_to_xml()

settings.source = openmc.IndependentSource(space=openmc.stats.Box([-4., -4., -4.],
                                                       [ 4.,  4.,  4.]))
settings.export_to_xml()

p1 = openmc.Plot()
p1.filename = 'plot1'
p1.basis = 'xy'
p1.width = (25.0, 25.0)
p1.pixels = (400, 400)
p1.color_by = 'material'
p1.colors = {u235: 'yellow', water: 'blue'}

p2 = openmc.Plot()
p2.filename = 'plot2'
p2.basis = 'xy'
p2.width = (18.0, 18.0)
p2.pixels = (400, 400)
p2.color_by = 'cell'

plots = openmc.Plots([p1, p2])
plots.export_to_xml()
