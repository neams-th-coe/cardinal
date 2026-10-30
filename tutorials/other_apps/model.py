import openmc

thickness = 100.0
width = 100.0
n_cells = 20

model = openmc.Model()

lithium = openmc.Material(name='lithium')
lithium.add_element('Li', 1.0)
lithium.set_density('g/cm3', 0.534)
model.materials = openmc.Materials([lithium])

x = [openmc.XPlane(x0=thickness * i / n_cells) for i in range(n_cells + 1)]
x[0].boundary_type = 'vacuum'
x[-1].boundary_type = 'vacuum'

y_min = openmc.YPlane(y0=-width / 2, boundary_type='reflective')
y_max = openmc.YPlane(y0=width / 2, boundary_type='reflective')
z_min = openmc.ZPlane(z0=-width / 2, boundary_type='reflective')
z_max = openmc.ZPlane(z0=width / 2, boundary_type='reflective')
transverse = +y_min & -y_max & +z_min & -z_max

cells = [openmc.Cell(fill=lithium, region=+x[i] & -x[i + 1] & transverse)
         for i in range(n_cells)]
model.geometry = openmc.Geometry(cells)

source = openmc.IndependentSource()
source.space = openmc.stats.CartesianIndependent(
    x=openmc.stats.Discrete([1e-6], [1.0]),
    y=openmc.stats.Uniform(-width / 2, width / 2),
    z=openmc.stats.Uniform(-width / 2, width / 2))
source.angle = openmc.stats.Monodirectional((1.0, 0.0, 0.0))
source.energy = openmc.stats.Discrete([14.1e6], [1.0])

model.settings.run_mode = 'fixed source'
model.settings.source = source
model.settings.batches = 50
model.settings.particles = 10000

model.export_to_model_xml()
