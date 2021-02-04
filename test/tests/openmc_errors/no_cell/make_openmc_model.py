import os

import numpy as np
import openmc

# -------------- Specs and dimensions --------------

mm = 0.1   # units of cm
μm = 1e-4  # units of cm

pebble_radius = 30.0*mm/2

uranium_enrichment = 19.9

clearance = 0.02
box_width = 2 * pebble_radius * (1 + clearance)

fuel_kernel_density = ('kg/m3', 10500.0)  # UCBTH-14-002, Table 2-1
flibe_density = ('kg/m3', 1900.0)

# FLiBe properties
inlet_temperature = 273.15 + 600.0        # UCBTH-14-002, Table 1-1
outlet_temperature = 273.15 + 700.0       # UCBTH-14-002, Table 1-1
flibe_temperature = (inlet_temperature + outlet_temperature)/2
li7_enrichment = 0.99995

# -------------- Materials --------------

uco = openmc.Material(name=f'Uranium oxycarbide')
uco.add_element('U', 1.0, enrichment=uranium_enrichment)
uco.add_element('C', 1.5)
uco.add_nuclide('O16', 0.5)
uco.set_density(*fuel_kernel_density)

flibe = openmc.Material(name='2LiF-BeF2')
flibe.set_density(*flibe_density)
flibe.add_nuclide('Li7', 2.0*li7_enrichment)
flibe.add_nuclide('Li6', 2.0*(1 - li7_enrichment))
flibe.add_element('Be', 1.0)
flibe.add_element('F', 4.0)

# -------------- Geometry --------------

bc = 'reflective'
xmin = openmc.XPlane(x0=-box_width/2, boundary_type=bc)
xmax = openmc.XPlane(x0=+box_width/2, boundary_type=bc)
ymin = openmc.YPlane(y0=-box_width/2, boundary_type=bc)
ymax = openmc.YPlane(y0=+box_width/2, boundary_type=bc)
zmin = openmc.ZPlane(z0=-box_width/2, boundary_type=bc)
zmax = openmc.ZPlane(z0=+box_width/2, boundary_type=bc)
inside_box = +xmin & -xmax & +ymin & -ymax & +zmin & -zmax

sphere = openmc.Sphere(r=pebble_radius)

pebble = openmc.Cell(name='Pebble', region=-sphere, fill = uco)
coolant = openmc.Cell(name='Coolant', fill=flibe, region=+sphere & inside_box)

univ = openmc.Universe(cells=[pebble, coolant])
geom = openmc.Geometry(univ)

# -------------- Settings --------------

settings = openmc.Settings()
settings.source = openmc.Source(
    space=openmc.stats.Box(*pebble.bounding_box, only_fissionable=True)
)
settings.particles = 1000
settings.inactive = 10
settings.batches = 50

# Export model
model = openmc.model.Model(geometry=geom, settings=settings)
model.export_to_xml()
