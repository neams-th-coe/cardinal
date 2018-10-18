import openmc

# -------------- Specs and dimensions --------------

mm = 0.1   # units of cm
μm = 1e-4  # units of cm

pebble_core_radius = 25.0*mm/2                       # UCBTH-14-002, Table 2-1 (differs slightly from Cisneros, Table 5-2)
pebble_outer_radius = 30.0*mm/2                      # UCBTH-14-002, Table 2-1; Cisneros, Table 5-2
pebble_active_radius = pebble_outer_radius - 1.0*mm  # UCBTH-14-002, Table 2-1; Cisneros, Table 5-2
uranium_enrichment = 19.9

clearance = 0.02
box_width = 2*pebble_outer_radius*(1 + clearance)

flibe_density = ('g/cm3', 1.9627)         # UCBTH-14-002, Table 1-2
pebble_core_density = ('g/cm3', 1.59368)  # Cisneros, Table 3-1
pebble_shell_density = ('g/cm3', 1.74)    # Cisneros, Table 3-1

# TRISO parameters
fuel_kernel_density = ('kg/m3', 10500.0)  # UCBTH-14-002, Table 2-1
fuel_kernel_radius = 400.0*μm             # UCBTH-14-002, Table 2-1
buffer_thickness = 100.0*μm               # UCBTH-14-002, Table 2-1
pyc_inner_thickness = 35.0*μm             # UCBTH-14-002, Table 2-1
sic_thickness = 35.0*μm                   # UCBTH-14-002, Table 2-1
pyc_outer_thickness = 35.0*μm             # UCBTH-14-002, Table 2-1

buffer_radius = fuel_kernel_radius + buffer_thickness
pyc_inner_radius = buffer_radius + pyc_inner_thickness
sic_radius = pyc_inner_radius + sic_thickness
pyc_outer_radius = sic_radius + pyc_outer_thickness

buffer_density = ('g/cm3', 1.0)  # Cisneros, Table 3-1
pyc_density = ('g/cm3', 1.87)    # Cisneros, Table 3-1
sic_density = ('g/cm3', 3.2)     # Cisneros, Table 3-1


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
uco.add_element('C', 1.5)
uco.add_nuclide('O16', 0.5)
uco.set_density(*fuel_kernel_density)

# From Cisneros, appendix B, material 24
flibe = openmc.Material(name='2LiF-BeF2')
flibe.set_density('atom/b-cm',  8.34284e-02)
flibe.add_nuclide('Li6', 3.28e-07)
flibe.add_nuclide('Li7', 2.38258e-02)
flibe.add_nuclide('Be9', 1.19185e-02)
flibe.add_nuclide('F19', 4.76740e-02)

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
kernel_sphere = openmc.Sphere(R=fuel_kernel_radius)
buffer_sphere = openmc.Sphere(R=buffer_radius)
pyc_inner_sphere = openmc.Sphere(R=pyc_inner_radius)
sic_sphere = openmc.Sphere(R=sic_radius)
pyc_outer_sphere = openmc.Sphere(R=pyc_outer_radius)

triso_kernel = openmc.Cell(name='Fuel kernel', fill=uco, region=-kernel_sphere)
triso_buffer = openmc.Cell(name='Buffer', fill=graphite_buffer, region=+kernel_sphere & -buffer_sphere)
triso_pyc_inner = openmc.Cell(name='Inner PyC', fill=graphite_pyc, region=+buffer_sphere & -pyc_inner_sphere)
triso_sic = openmc.Cell(name='SiC', fill=sic, region=+pyc_inner_sphere & -sic_sphere)
triso_pyc_outer = openmc.Cell(name='Outer PyC', fill=graphite_pyc, region=+sic_sphere & -pyc_outer_sphere)
active_filler = openmc.Cell(name='Filler', fill=graphite_filler, region=+pyc_outer_sphere)

triso_univ = openmc.Universe(cells=[triso_kernel, triso_buffer, triso_pyc_inner,
                                    triso_sic, triso_pyc_outer, active_filler])

# Ordered lattice for TRISOs
# TODO: Use random sphere packing to get real distribution of particles
triso_lat = openmc.RectLattice()
f = 1.498  # Manually adjusted to get a uranium mass of 1.5 g
triso_lat.lower_left = (-f*pyc_outer_radius, -f*pyc_outer_radius, -f*pyc_outer_radius)
triso_lat.pitch = (2*f*pyc_outer_radius, 2*f*pyc_outer_radius, 2*f*pyc_outer_radius)
triso_lat.universes = [[[triso_univ]]]
triso_lat.outer = triso_univ

# Pebble
outer_sphere = openmc.Sphere(R=pebble_outer_radius)
active_sphere = openmc.Sphere(R=pebble_active_radius)
core_sphere = openmc.Sphere(R=pebble_core_radius)
pebble_core = openmc.Cell(name='Graphite core', fill=graphite_core, region=-core_sphere)
active_region = openmc.Cell(name='Active region', fill=triso_lat, region=+core_sphere & -active_sphere)
pebble_shell = openmc.Cell(name='Graphite shell', fill=graphite_shell, region=+active_sphere & -outer_sphere)
coolant = openmc.Cell(name='Coolant', fill=flibe, region=+outer_sphere & inside_box)

geom = openmc.Geometry([pebble_core, active_region, pebble_shell, coolant])

# -------------- Settings --------------

settings = openmc.Settings()
settings.source = openmc.Source(
    space=openmc.stats.Box(*pebble_shell.bounding_box,
                           only_fissionable=True)
)
settings.particles = 1000
settings.inactive = 10
settings.batches = 50

# Add volume calculation
vol_calc = openmc.VolumeCalculation([uco], 100000, *active_region.bounding_box)
settings.volume_calculations = [vol_calc]

# Export model
model = openmc.model.Model(geometry=geom, settings=settings)
model.export_to_xml()
