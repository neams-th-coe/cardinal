import openmc
import math

# define OpenMC materials; the names should match the names assigned to the blocks
# in the "block <n> material <name>" lines in the journal file

model = openmc.Model()

pf = openmc.Material(name="pf")
pf.set_density('g/cc', 19.30)
pf.add_element('W', 1.0)

ss316 = openmc.Material(name="ss316")
ss316.add_element('C', 0.0003, 'wo')
ss316.add_element('Mn', 0.02, 'wo')
ss316.add_element('Si', 0.01, 'wo')
ss316.add_element('P', 0.0005, 'wo')
ss316.add_element('S', 0.0002, 'wo')
ss316.add_element('Cr', 0.185, 'wo')
ss316.add_element('Mo', 0.025, 'wo')
ss316.add_element('Ni', 0.13, 'wo')
ss316.add_element('Fe', 0.629, 'wo')
ss316.set_density("g/cm3", 8.0)

eurofer = openmc.Material(name="eurofer")
eurofer.add_element('Fe', 0.011   , 'wo')
eurofer.add_element('Al', 0.002   , 'wo')
eurofer.add_element('As', 0.0002  , 'wo')
eurofer.add_element('B', 0.0012  , 'wo')
eurofer.add_element('C', 0.0005  , 'wo')
eurofer.add_element('Co', 0.0005  , 'wo')
eurofer.add_element('Cr', 0.0005  , 'wo')
eurofer.add_element('Cu', 0.00005 , 'wo')
eurofer.add_element('Mn', 0.00005 , 'wo')
eurofer.add_element('Mo', 0.0001  , 'wo')
eurofer.add_element('N', 0.0001  , 'wo')
eurofer.add_element('Nb', 0.00005 , 'wo')
eurofer.add_element('Ni', 0.0003  , 'wo')
eurofer.add_element('O', 0.00005 , 'wo')
eurofer.add_element('P', 0.004   , 'wo')
eurofer.add_element('S', 0.0001  , 'wo')
eurofer.add_element('Sb', 0.09    , 'wo')
eurofer.add_element('Sn', 0.0001  , 'wo')
eurofer.add_element('Si', 0.0011  , 'wo')
eurofer.add_element('Ta', 0.00002 , 'wo')
eurofer.add_element('Ti', 0.0005  , 'wo')
eurofer.add_element('V', 0       , 'wo')
eurofer.add_element('W', 0.0001  , 'wo')
eurofer.add_element('Zr', 0.88698 , 'wo')
eurofer.set_density("g/cm3", 7.798)

beryllium = openmc.Material(name="beryllium")
beryllium.add_element('Be', 1.0)
beryllium.set_density("g/cm3", 1.85)

Li4SiO4 = openmc.Material(name="Li4SiO4")
Li4SiO4.add_element('Li', 4.0)
Li4SiO4.add_element('Si', 1.0)
Li4SiO4.add_element('O', 4.0)
Li4SiO4.set_density("g/cm3", 2.39)

Helium = openmc.Material(name="Helium")
Helium.add_element('He', 1.0)
Helium.set_density("kg/m3", 0.166)

# define the breeder and multiplier as homogeneous mixtures of materials according to
# given atomic percents
Breeder = openmc.Material.mix_materials([eurofer, beryllium, Li4SiO4, Helium], [0.1, 0.37, 0.15, 0.38], 'ao',name="breeder")

Multiplier = openmc.Material.mix_materials([beryllium, Helium], [0.65, 0.35], 'ao', name="multiplier")

model.materials = openmc.Materials([pf, Breeder, Multiplier, ss316])

model.settings.dagmc = True
model.settings.photon_transport = True
model.settings.batches = 10
model.settings.particles = 10000
model.settings.run_mode = "fixed source"

model.settings.temperature = {'default': 800.0,
                              'method': 'interpolation',
                              'range': (294.0, 3000.0),
                              'tolerance': 1000.0}

# define the neutron source
source = openmc.IndependentSource()

r = openmc.stats.PowerLaw(600, 700, 1.0)
phi = openmc.stats.Uniform(0.0, 2*math.pi)
z = openmc.stats.Discrete([0,], [1.0,])
spatial_dist = openmc.stats.CylindricalIndependent(r, phi, z)

source.angle = openmc.stats.Isotropic()
source.energy = openmc.stats.Discrete([14.08e6], [1.0])
source.space=spatial_dist
model.settings.source = source

dagmc_univ = openmc.DAGMCUniverse(filename='tokamak.h5m')
model.geometry = openmc.Geometry(root=dagmc_univ)

model.export_to_xml()
