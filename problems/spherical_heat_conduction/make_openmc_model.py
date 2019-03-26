import os

import numpy as np
import openmc


def flibe_density(p, T):
    # assumes the default value for drho_dp of 1.7324e-7
    rho = -0.4884*T + 1.7324e-7*(p - 101325.0) + 2413.0 # kg/m3
    return ('kg/m3', rho)

# -------------- Specs and dimensions --------------

mm = 0.1   # units of cm
μm = 1e-4  # units of cm

pebble_core_radius = 25.0*mm/2                       # UCBTH-14-002, Table 2-1 (differs slightly from Cisneros, Table 5-2)
pebble_outer_radius = 30.0*mm/2                      # UCBTH-14-002, Table 2-1; Cisneros, Table 5-2
pebble_active_radius = pebble_outer_radius - 1.0*mm  # UCBTH-14-002, Table 2-1; Cisneros, Table 5-2
uranium_enrichment = 19.9

clearance = 0.02
box_width = 2*pebble_outer_radius*(1 + clearance)

# TRISO parameters
fuel_kernel_density = ('kg/m3', 10500.0)  # UCBTH-14-002, Table 2-1
fuel_kernel_radius = 400.0*μm / 2         # UCBTH-14-002, Table 2-1
buffer_thickness = 100.0*μm               # UCBTH-14-002, Table 2-1
pyc_inner_thickness = 35.0*μm             # UCBTH-14-002, Table 2-1
sic_thickness = 35.0*μm                   # UCBTH-14-002, Table 2-1
pyc_outer_thickness = 35.0*μm             # UCBTH-14-002, Table 2-1
packing_fraction = 0.40                   # UCBTH-14-002, Table 2-1

buffer_radius = fuel_kernel_radius + buffer_thickness
pyc_inner_radius = buffer_radius + pyc_inner_thickness
sic_radius = pyc_inner_radius + sic_thickness
pyc_outer_radius = sic_radius + pyc_outer_thickness

# Densities
pebble_core_density = ('g/cm3', 1.59368)  # Cisneros, Table 3-1
pebble_shell_density = ('g/cm3', 1.74)    # Cisneros, Table 3-1
buffer_density = ('g/cm3', 1.0)  # Cisneros, Table 3-1
pyc_density = ('g/cm3', 1.87)    # Cisneros, Table 3-1
sic_density = ('g/cm3', 3.2)     # Cisneros, Table 3-1

# FLiBe properties
inlet_temperature = 273.15 + 600.0        # UCBTH-14-002, Table 1-1
outlet_temperature = 273.15 + 700.0       # UCBTH-14-002, Table 1-1
flibe_temperature = (inlet_temperature + outlet_temperature)/2
li7_enrichment = 0.99995

# -------------- Materials --------------

# Materials for pebble
graphite_core = openmc.Material(name='Graphite, core')
graphite_core.set_density(*pebble_core_density)
graphite_core.add_element('C', 1.0)
graphite_core.add_s_alpha_beta('c_Graphite')

graphite_shell = openmc.Material(name='Graphite, shell')
graphite_shell.set_density(*pebble_shell_density)
graphite_shell.add_element('C', 1.0)
graphite_shell.add_s_alpha_beta('c_Graphite')

# Materials for TRISO
graphite_buffer = openmc.Material(name='Graphite, buffer')
graphite_buffer.set_density(*buffer_density)
graphite_buffer.add_element('C', 1.0)
graphite_buffer.add_s_alpha_beta('c_Graphite')

graphite_pyc = openmc.Material(name='Pyrolytic graphite')
graphite_pyc.set_density(*pyc_density)
graphite_pyc.add_element('C', 1.0)
graphite_pyc.add_s_alpha_beta('c_Graphite')

sic = openmc.Material(name='SiC')
sic.add_element('C', 1.0)
sic.add_element('Si', 1.0)
sic.set_density(*sic_density)
# TODO: Add SiC S(alpha, beta)

# TODO: Need real specifications for filler carbon
graphite_filler = openmc.Material(name='Graphite, filler')
graphite_filler.set_density('g/cm3', 1.6)
graphite_filler.add_element('C', 1.0)
graphite_filler.add_s_alpha_beta('c_Graphite')

uco = openmc.Material(name=f'Uranium oxycarbide')
uco.add_element('U', 1.0, enrichment=uranium_enrichment)
# TODO: More realistic isotopic composition for 19.9% enrichment
uco.add_element('C', 1.5)
uco.add_nuclide('O16', 0.5)
uco.set_density(*fuel_kernel_density)

# From Cisneros, appendix B, material 24
flibe = openmc.Material(name='2LiF-BeF2')
flibe.set_density(*flibe_density(101.325e3, flibe_temperature))
flibe.add_nuclide('Li7', 2.0*li7_enrichment)
flibe.add_nuclide('Li6', 2.0*(1 - li7_enrichment))
flibe.add_element('Be', 1.0)
flibe.add_element('F', 4.0)
# TODO: FLiBe S(a,b)??

# -------------- Geometry --------------

bc = 'reflective'
xmin = openmc.XPlane(x0=-box_width/2, boundary_type=bc)
xmax = openmc.XPlane(x0=+box_width/2, boundary_type=bc)
ymin = openmc.YPlane(y0=-box_width/2, boundary_type=bc)
ymax = openmc.YPlane(y0=+box_width/2, boundary_type=bc)
zmin = openmc.ZPlane(z0=-box_width/2, boundary_type=bc)
zmax = openmc.ZPlane(z0=+box_width/2, boundary_type=bc)
inside_box = +xmin & -xmax & +ymin & -ymax & +zmin & -zmax

# TRISO particle
kernel_sphere = openmc.Sphere(r=fuel_kernel_radius)
buffer_sphere = openmc.Sphere(r=buffer_radius)
pyc_inner_sphere = openmc.Sphere(r=pyc_inner_radius)
sic_sphere = openmc.Sphere(r=sic_radius)

triso_kernel = openmc.Cell(name='Fuel kernel', fill=uco, region=-kernel_sphere)
triso_buffer = openmc.Cell(name='Buffer', fill=graphite_buffer, region=+kernel_sphere & -buffer_sphere)
triso_pyc_inner = openmc.Cell(name='Inner PyC', fill=graphite_pyc, region=+buffer_sphere & -pyc_inner_sphere)
triso_sic = openmc.Cell(name='SiC', fill=sic, region=+pyc_inner_sphere & -sic_sphere)
triso_pyc_outer = openmc.Cell(name='Outer PyC', fill=graphite_pyc, region=+sic_sphere)

triso_univ = openmc.Universe(cells=[triso_kernel, triso_buffer, triso_pyc_inner,
                                    triso_sic, triso_pyc_outer])

# Pebble
outer_sphere = openmc.Sphere(r=pebble_outer_radius)
active_sphere = openmc.Sphere(r=pebble_active_radius)
core_sphere = openmc.Sphere(r=pebble_core_radius)
pebble_core = openmc.Cell(name='Graphite core', fill=graphite_core, region=-core_sphere)
active_region = openmc.Cell(name='Active region', region=+core_sphere & -active_sphere)
pebble_shell = openmc.Cell(name='Graphite shell', fill=graphite_shell, region=+active_sphere & -outer_sphere)
pebble_univ = openmc.Universe(cells=[pebble_core, active_region, pebble_shell])

pebble = openmc.Cell(name='Pebble', fill=pebble_univ, region=-outer_sphere)
coolant = openmc.Cell(name='Coolant', fill=flibe, region=+outer_sphere & inside_box)

if os.path.exists('triso_centers.npy'):
    triso_centers = np.load('triso_centers.npy')
    trisos = [openmc.model.TRISO(pyc_outer_radius, triso_univ, c)
              for c in triso_centers]
else:
    # Use random sphere packing to generate TRISOs
    triso_centers = openmc.model.pack_spheres(
        radius=pyc_outer_radius,
        region=active_region.region,
        pf=packing_fraction,
        initial_pf=0.15
    )

    trisos = [openmc.model.TRISO(pyc_outer_radius, triso_univ, c)
              for c in triso_centers]

    # Create file for TRISO centers
    np.save('triso_centers.npy', triso_centers)


# Put TRISOS into a lattice and assign to pebble active region
lower_left, upper_right = active_region.bounding_box
shape = (5, 5, 5)
pitch = (upper_right - lower_left)/shape
triso_lat = openmc.model.create_triso_lattice(
    trisos, lower_left, pitch, shape, graphite_filler)
active_region.fill = triso_lat

geom = openmc.Geometry([pebble, coolant])

# -------------- Settings --------------

settings = openmc.Settings()
settings.source = openmc.Source(
    space=openmc.stats.Box(*pebble_shell.bounding_box, only_fissionable=True)
)
settings.particles = 1000
settings.inactive = 10
settings.batches = 50

# Add volume calculation
vol_calc = openmc.VolumeCalculation([uco], 10000000, *active_region.bounding_box)
settings.volume_calculations = [vol_calc]

# Export model
model = openmc.model.Model(geometry=geom, settings=settings)
model.export_to_xml()
