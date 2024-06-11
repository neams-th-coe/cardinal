import openmc

# materials
fuel = openmc.Material(name="fuel")
fuel.add_nuclide('U235', 0.05)
fuel.add_nuclide('U238', 0.95)
fuel.add_nuclide('O16', 2.0)
fuel.set_density('g/cm3', 10.0)

water = openmc.Material(name="water")
water.add_nuclide('H1', 2.0, 'ao')
water.add_nuclide('O16', 1.0, 'ao')
water.set_density('g/cc', 1.0)
water.add_s_alpha_beta('c_H_in_H2O')

mats = openmc.Materials([fuel, water])
mats.export_to_xml()

# geometry

dagmc_univ = openmc.DAGMCUniverse(filename='2blckt03.h5m', auto_geom_ids=True)
geometry = openmc.Geometry(root=dagmc_univ)
geometry.export_to_xml()

# settings

settings = openmc.Settings()
settings.dagmc = True
settings.batches = 10
settings.inactive = 2
settings.particles = 5000

settings.temperature = {'default': 500.0,
                        'method': 'interpolation',
                        'range': (294.0, 3000.0),
                        'tolerance': 1000.0}

settings.source = openmc.IndependentSource(space=openmc.stats.Box([-25., -25., -25.],
                                                       [ 25.,  25.,  25.]))
settings.export_to_xml()

# plot

p1 = openmc.Plot()
p1.filename = 'plot1'
p1.basis = 'xz'
p1.width = (50.05, 50.05)
p1.pixels = (400, 400)
p1.color_by = 'material'
p1.colors = {fuel: 'yellow', water: 'blue'}

p2 = openmc.Plot()
p2.filename = 'plot2'
p2.basis = 'yz'
p2.width = (50.0, 50.0)
p2.pixels = (400, 400)
p2.color_by = 'material'
p2.colors = {fuel: 'yellow', water: 'blue'}

plots = openmc.Plots([p1, p2])
plots.export_to_xml()