#!/usr/bin/env python
import os
import numpy as np
import openmc
import openmc.model

# -------------- Unit Conversions -----------
mm = 0.1   # default cm
μm = 1e-4  # default cm

# geometric parameters for the TRISO particle
radius_fuel         = 400.0*μm/2
thickness_c_carbon_buffer  = 100.0*μm
thickness_pyc_inner = 35.0*μm
thickness_sic       = 35.0*μm
thickness_pyc_outer = 35.0*μm
radius_c_carbon_buffer     = radius_fuel + thickness_c_carbon_buffer
radius_pyc_inner    = radius_c_carbon_buffer + thickness_pyc_inner
radius_sic          = radius_pyc_inner + thickness_sic
radius_pyc_outer    = radius_sic + thickness_pyc_outer
packing_fraction    = 0.2
radius_pebble_outer = 1.5

# pebble centers coordinates (x,y,z)
pebble_centers = np.loadtxt('pebble_centers.txt', delimiter=' ')

# material definitions
enrichment_uranium   = 0.199
fraction_u234        = 2e-3
fuel = openmc.Material(name='fuel - uranium oxycarbide - triso partciles')
fuel.add_nuclide('U234', 89.58*enrichment_uranium*fraction_u234, percent_type='wo')
fuel.add_nuclide('U235', 89.58*enrichment_uranium*(1 - fraction_u234), percent_type='wo')
fuel.add_nuclide('U238', 89.58*(1 - enrichment_uranium), percent_type='wo')
fuel.add_nuclide('C0', 1.80, percent_type='wo')
fuel.add_element('O', 8.62, percent_type='wo')
fuel.set_density('g/cm3', 10.5)

carbon_buffer = openmc.Material(name='carbon_buffer - triso partciles')
carbon_buffer.set_density('g/cm3', 1.0)
carbon_buffer.add_nuclide('C0', 1.0)
carbon_buffer.add_s_alpha_beta('c_Graphite')

pyc = openmc.Material(name='pyc - triso particles')
pyc.set_density('g/cm3', 1.87)
pyc.add_nuclide('C0', 1.0)
pyc.add_s_alpha_beta('c_Graphite')

sic = openmc.Material(name='sic - triso partciles')
sic.add_nuclide('C0', 1.0)
sic.add_element('Si', 1.0)
sic.set_density('g/cm3', 3.2)

matrix = openmc.Material(name='matrix - triso particles')
matrix.set_density('g/cm3', 1.6)
matrix.add_nuclide('C0', 1.0)
matrix.add_s_alpha_beta('c_Graphite')

enrichment_li7       = 0.99995
flibe = openmc.Material(name='flibe - 2LiF-BeF2')
flibe.set_density('kg/m3', 1960.0)
flibe.add_nuclide('Li7', 2.0*enrichment_li7)
flibe.add_nuclide('Li6', 2.0*(1 - enrichment_li7))
flibe.add_element('Be', 1.0)
flibe.add_element('F', 4.0)

# define a universe consisting of a TRISO particle
s_fuel                  = openmc.Sphere(r=radius_fuel)
s_c_carbon_buffer       = openmc.Sphere(r=radius_c_carbon_buffer)
s_pyc_inner             = openmc.Sphere(r=radius_pyc_inner)
s_sic                   = openmc.Sphere(r=radius_sic)
s_pyc_outer             = openmc.Sphere(r=radius_pyc_outer)
c_triso_fuel            = openmc.Cell(name='TRISO Fuel', fill=fuel, region=-s_fuel)
c_triso_c_carbon_buffer = openmc.Cell(name='TRISO Graphite Buffer', fill=carbon_buffer, region=+s_fuel & -s_c_carbon_buffer)
c_triso_pyc_inner       = openmc.Cell(name='TRISO Pyrolitic Graphite Inner', fill=pyc, region=+s_c_carbon_buffer  & -s_pyc_inner)
c_triso_sic             = openmc.Cell(name='TIRSO Silicone Carbide', fill=sic, region=+s_pyc_inner & -s_sic)
c_triso_pyc_outer       = openmc.Cell(name='TRISO Pyrolitic Graphite Outer', fill=pyc, region=+s_sic & -s_pyc_outer)
c_triso_matrix          = openmc.Cell(name='TRISO Graphite Matrix', fill=matrix, region=+s_pyc_outer)

triso_cells = [c_triso_fuel, c_triso_c_carbon_buffer, c_triso_pyc_inner,
               c_triso_sic, c_triso_pyc_outer, c_triso_matrix]
u_triso = openmc.Universe(cells=triso_cells)

# define a universe consisting of a pebble, filled with random TRISOs
s_pebble_outer = openmc.Sphere(r=radius_pebble_outer)
c_pebble_outer = openmc.Cell(name='Pebble graphite outer region', region=-s_pebble_outer)
c_pebble_flibe = openmc.Cell(name='Pebble exterior (FLiBe)', fill=flibe, region=+s_pebble_outer)

l_triso = None
l_triso_shape = (20, 20, 20)

# TRISO particles random distribution using 'packing_fraction'
c_triso_pyc_outer_random = openmc.Cell(name='c_triso_pyc_outer_random',
                                       fill=pyc,
                                       region=+s_sic)
triso_rand_cells = [c_triso_fuel,
                    c_triso_c_carbon_buffer,
                    c_triso_pyc_inner,
                    c_triso_sic,
                    c_triso_pyc_outer_random]
u_triso_random = openmc.Universe(cells=triso_rand_cells)

r_triso_random = -s_pebble_outer
spheres_random = openmc.model.pack_spheres(radius=radius_pyc_outer,
                                           region=r_triso_random,
                                           pf=packing_fraction,
                                           initial_pf=0.15)
triso_random = [openmc.model.TRISO(radius_pyc_outer, u_triso_random, i) for i in spheres_random]
triso_centers = np.vstack([i.center for i in triso_random])

lower_left, upper_right = c_pebble_outer.region.bounding_box
shape = l_triso_shape
pitch = (upper_right - lower_left)/shape
l_triso = openmc.model.create_triso_lattice(triso_random,
                                            lower_left,
                                            pitch,
                                            shape,
                                            matrix)

if not l_triso:
    raise RuntimeError("TIRSO lattice is undefined.")

# Fill pebble cell with the TRISO lattice
c_pebble_outer.fill = l_triso

pebble_univ_cells = [c_pebble_outer,
                     c_pebble_flibe]
u_pebble = openmc.Universe(cells=pebble_univ_cells)

# Reactor cells
vessel_outer = openmc.rectangular_prism(4, 4, boundary_type='reflective')
vessel_bottom = openmc.ZPlane(z0=0.0, boundary_type='reflective')
vessel_top = openmc.ZPlane(z0=12.0, boundary_type='reflective')
vessel_region = vessel_outer & +vessel_bottom & -vessel_top
vessel_cell = openmc.Cell(name='Pebble Vessel', region=vessel_region)

# create an additional universe-cell layer for the pebbles
pebble_univs = []
pebble_cells = []
for center in pebble_centers:
    s = openmc.Sphere(r=radius_pebble_outer)
    c = openmc.Cell(region=-s, fill=u_pebble)
    u = openmc.Universe(cells=[c])
    pebble_cells.append(c)
    pebble_univs.append(u)

# Creating TRISOs for the pebbles to pack them into a lattice for efficiency
pebble_trisos = []
for pebble_num, (center, pebble_univ) in enumerate(zip(pebble_centers, pebble_univs)):
    cell_name = 'cell_pebble_{}'.format(pebble_num)
    triso = openmc.model.TRISO(radius_pebble_outer,
                               pebble_univ,
                               center)
    triso.name = cell_name
    pebble_trisos.append(triso)

# Place pebbles into a lattice
llc_vessel, urc_vessel = vessel_region.bounding_box
l_pebble_shape = np.asarray((1, 1, 1))
l_pebble_pitch = (urc_vessel - llc_vessel)/l_pebble_shape
l_pebble = openmc.model.create_triso_lattice(pebble_trisos,
                                             llc_vessel,
                                             l_pebble_pitch,
                                             l_pebble_shape,
                                             flibe)

# fill vessel with pebble lattice
vessel_cell.fill = l_pebble

# create geometry
geom = openmc.Geometry([vessel_cell])

# -------------- Settings --------------------
settings = openmc.Settings()
source_space = openmc.stats.Box(*vessel_cell.bounding_box,
                                only_fissionable=True)
settings.source    = openmc.Source(space=source_space)
settings.particles = 100
settings.inactive  = 10
settings.batches   = 50
settings.temperature = {'default': 650.0 + 273.15,
                        'method': 'interpolation',
                        'multipole': False,
                        'range': (300.0, 1500.0),
                        'tolerance': 1000.0}

# -------------- Plots --------------
# Plot parameters
L = 4
N = 3

m_colors = {fuel: 'brown',
            carbon_buffer: 'LightSteelBlue',
            pyc: 'blue',
            sic: 'orange',
            matrix: 'cyan',
            flibe: 'yellow'}

plot1          = openmc.Plot()
plot1.filename = 'plot1'
plot1.width    = (L, L)
plot1.basis    = 'xy'
plot1.origin   = (0, 0, 0)
plot1.pixels   = (1000, 1000)
plot1.color_by = 'material'
plot1.colors   = m_colors

plot_width   = max(L, N*L)
plot2          = openmc.Plot()
plot2.filename = 'plot3'
plot2.width    = (plot_width, plot_width)
plot2.basis    = 'xz'
plot2.origin   = (0, 0, N*L/2.0)
plot2.pixels   = (1000, 1000)
plot2.color_by = 'material'
plot2.colors   = m_colors

plots = openmc.Plots([plot1, plot2])

# Create model and export
model = openmc.model.Model(geometry=geom, settings=settings)
model.plots = plots
model.export_to_xml()
