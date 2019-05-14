# -------------- Legend ---------------------
# b_  = box
# c_  = cell
# cz_ = cylinder z
# l_  = lattice
# m_  = material
# px_ = plane x
# py_ = plane y
# pz_ = plane z
# r_  = region
# s_  = sphere
# u_  = universe
# -------------- Modules --------------------
import os
import math
import numpy as np
import openmc
import openmc.model
import scipy.stats
import pandas as pd
from   math   import pi
from   IPython.display import Image
import matplotlib.pyplot as plt
# -------------- Functions ------------------
def density_flibe(p, T):
    # assumes the default value for drho_dp of 1.7324e-7
    rho = -0.4884*T + 1.7324e-7*(p - 101325.0) + 2413.0 # kg/m3
    return ('kg/m3', rho)
# -------------- Arbitrary Parameters -------
voxel                 = 0              # Set 1 to generate 3d voxel plots              and 0 for 2d ppm plots
random_distribution   = 0              # Set 1 for TRISO particles random distribution and 0 for regular lattice
# -------------- Units Parameters -----------
mm                    = 0.1   # default cm
μm                    = 1e-4  # default cm
# -------------- Geometry Parameters --------
# TRISO particle
radius_fuel           = 400.0*μm/2     # UCBTH-14-002, Table 2-1
thickness_c_buffer    = 100.0*μm       # UCBTH-14-002, Table 2-1
thickness_pyc_inner   =  35.0*μm       # UCBTH-14-002, Table 2-1
thickness_sic         =  35.0*μm       # UCBTH-14-002, Table 2-1
thickness_pyc_outer   =  35.0*μm       # UCBTH-14-002, Table 2-1
radius_c_buffer       = radius_fuel      + thickness_c_buffer
radius_pyc_inner      = radius_c_buffer  + thickness_pyc_inner
radius_sic            = radius_pyc_inner + thickness_sic
radius_pyc_outer      = radius_sic       + thickness_pyc_outer
packing_fraction      = 0.40           # UCBTH-14-002, Table 2-1
pitch_triso_lattice   = (radius_fuel+thickness_c_buffer+thickness_pyc_inner+thickness_sic+thickness_pyc_outer)*(4*pi/3/packing_fraction)**0.33333333333333333333333
# Pebble - inner zone graphite; central active zone triso; outer zone graphite
# hyuan@anl.gov email April 2, 2019 8:40 PM
# In the actual TAMU experiment, the pebble diameter is 22.225mm.
# But in CAD, we made it 22.225mm*0.95 = 21.11375 mm To avoid any pebble contacts.
# So the pebble radius is 21.11375 mm/2 = 10.556875 mm = 1.0556875 cm
# To scale up tp 3 cm pebble radius ,  I should scale Nek mesh and CAD file by a factor 
# 3/1.0556875 = 2.8417500444
tamu_exp_factor       = 1/1.0556875             # Factor to scale geometry to TAMU experiment with pebble radius 1.0556875 cm
radius_pebble_inner   = 2.5/2*tamu_exp_factor   # UCBTH-14-002, Table 2-1 (differs slightly from Cisneros, Table 5-2) ; scaled by TAMU experiment factor
radius_pebble_outer   = 3.0/2*tamu_exp_factor   # UCBTH-14-002, Table 2-1; Cisneros, Table 5-2                        ; scaled by TAMU experiment factor
radius_pebble_central = radius_pebble_outer - 0.1*tamu_exp_factor # UCBTH-14-002, Table 2-1; Cisneros, Table 5-2      ; scaled by TAMU experiment factor
clearance             = 0.02
# width_flibe           = 2*radius_pebble_outer*(1+clearance)                                                         # scaled by TAMU experiment factor
radius_pebble_flibe   = radius_pebble_outer*(1+clearance)
# pebble centers
center_1              = (0,0,0)
center_2              = (0,0,1.05*3)
# Vessel
extra_thickness       =  4.3
x_vessel              =  0.0 
y_vessel              =  0.0
z1_vessel             = -2.54*radius_pebble_outer - extra_thickness # bottom plane ; scaled by TAMU experiment factor
z2_vessel             = 12.7 *radius_pebble_outer + extra_thickness # top plane    ; scaled by TAMU experiment factor
radius_vessel         =  7.0 *radius_pebble_outer + extra_thickness #                scaled by TAMU experiment factor
height_vessel         = z2_vessel - z1_vessel
# -------------- Materials Parameters --------
# TRISO particle
enrichment_uranium   = 0.199
fraction_u234        = 2e-3                 # U234/(U234+U235) mass from Giacint facility fuel
density_fuel         = ('g/cm3', 10.5)      # UCBTH-14-002, Table 2-1
density_c_buffer     = ('g/cm3', 1.0)       # Cisneros    , Table 3-1
density_pyc          = ('g/cm3', 1.87)      # Cisneros    , Table 3-1
density_sic          = ('g/cm3', 3.2)       # Cisneros    , Table 3-1
# Pebble
density_graphite_inner = ('g/cm3', 1.59368) # Cisneros    , Table 3-1
density_graphite_outer = ('g/cm3', 1.74)    # Cisneros    , Table 3-1
# FLiBe coolant
enrichment_li7       = 0.99995
temperature_inlet    = 273.15 + 600.0       # UCBTH-14-002, Table 1-1
temperature_outlet   = 273.15 + 700.0       # UCBTH-14-002, Table 1-1
temperature_flibe    = (temperature_inlet+temperature_outlet)/2
# --------------------------------------------------
# NO PARAMETERS DEFINITION BEYOND THIS LINE
# -------------- Materials Definition --------------
# TRISO particle
# Fuel from Nagley et al. Fabrication of Uranium Oxycarbide Kernels for HTR Fuel https://inldigitallibrary.inl.gov/sites/sti/sti/4886646.pdf Table 2
m_fuel = openmc.Material(name='m_fuel - uranium oxycarbide - triso partciles')
m_fuel.add_nuclide('U234', 89.58*   enrichment_uranium*   fraction_u234 , percent_type='wo')
m_fuel.add_nuclide('U235', 89.58*   enrichment_uranium*(1-fraction_u234), percent_type='wo')
m_fuel.add_nuclide('U238', 89.58*(1-enrichment_uranium)                 , percent_type='wo')
m_fuel.add_element('C'   ,  1.80                                        , percent_type='wo')
m_fuel.add_element('O'   ,  8.62                                        , percent_type='wo')
m_fuel.set_density(*density_fuel)
#
m_graphite_c_buffer = openmc.Material(name='m_graphite_c_buffer - triso partciles')
m_graphite_c_buffer.set_density(*density_c_buffer)
m_graphite_c_buffer.add_element('C', 1.0)
m_graphite_c_buffer.add_s_alpha_beta('c_Graphite')
#
m_graphite_pyc = openmc.Material(name='m_graphite_pyc - triso particles')
m_graphite_pyc.set_density(*density_pyc)
m_graphite_pyc.add_element('C', 1.0)
m_graphite_pyc.add_s_alpha_beta('c_Graphite')
#
m_sic = openmc.Material(name='sic - triso partciles')
m_sic.add_element('C' , 1.0)
m_sic.add_element('Si', 1.0)
m_sic.set_density(*density_sic)
# TODO: Add SiC S(alpha, beta) ; ENDF/B-VIIIb4 has data for both Si and C in SiC
#
m_graphite_matrix = openmc.Material(name='m_graphite_matrix - triso particles')
m_graphite_matrix.set_density('g/cm3', 1.6)
m_graphite_matrix.add_element('C', 1.0)
m_graphite_matrix.add_s_alpha_beta('c_Graphite')
# TODO: Need real specifications for the carbon filler
# Pebble
m_graphite_inner = openmc.Material(name='m_graphite_inner - pebble inner zone')
m_graphite_inner.set_density(*density_graphite_inner)
m_graphite_inner.add_element('C', 1.0)
m_graphite_inner.add_s_alpha_beta('c_Graphite')
#
m_graphite_outer = openmc.Material(name='m_graphite_outer - pebble outer zone')
m_graphite_outer.set_density(*density_graphite_outer)
m_graphite_outer.add_element('C', 1.0)
m_graphite_outer.add_s_alpha_beta('c_Graphite')
# FLiBe coolant - From Cisneros, appendix B, material 24
m_flibe = openmc.Material(name='m_flibe - 2LiF-BeF2')
m_flibe.set_density(*density_flibe(101.325e3, temperature_flibe))
m_flibe.add_nuclide('Li7', 2.0*     enrichment_li7)
m_flibe.add_nuclide('Li6', 2.0*(1 - enrichment_li7))
m_flibe.add_element('Be' , 1.0)
m_flibe.add_element('F'  , 4.0)
# TODO: FLiBe coolant - no S(alpha, beta) data available up to ENDF/B-VIIIb4
# -------------- Geometry Definition --------------
# TRISO particle
s_fuel             = openmc.Sphere(R=radius_fuel)
s_c_buffer         = openmc.Sphere(R=radius_c_buffer)
s_pyc_inner        = openmc.Sphere(R=radius_pyc_inner)
s_sic              = openmc.Sphere(R=radius_sic)
s_pyc_outer        = openmc.Sphere(R=radius_pyc_outer)
c_triso_fuel       = openmc.Cell(name='c_triso_fuel'     , fill=m_fuel,              region=-s_fuel)
c_triso_c_buffer   = openmc.Cell(name='c_triso_c_buffer' , fill=m_graphite_c_buffer, region=+s_fuel      & -s_c_buffer)
c_triso_pyc_inner  = openmc.Cell(name='c_triso_pyc_inner', fill=m_graphite_pyc,      region=+s_c_buffer  & -s_pyc_inner)
c_triso_sic        = openmc.Cell(name='c_triso_sic'      , fill=m_sic,               region=+s_pyc_inner & -s_sic)
c_triso_pyc_outer  = openmc.Cell(name='c_triso_pyc_outer', fill=m_graphite_pyc,      region=+s_sic       & -s_pyc_outer)
c_triso_matrix     = openmc.Cell(name='c_triso_matrix'   , fill=m_graphite_matrix,   region=+s_pyc_outer)
u_triso            = openmc.Universe(cells=[c_triso_fuel, c_triso_c_buffer, c_triso_pyc_inner, c_triso_sic, c_triso_pyc_outer, c_triso_matrix])
# Pebble
s_pebble_inner     = openmc.Sphere(R=radius_pebble_inner)
s_pebble_central   = openmc.Sphere(R=radius_pebble_central)
s_pebble_outer     = openmc.Sphere(R=radius_pebble_outer)
c_pebble_inner     = openmc.Cell(name='c_pebble_inner'  , fill=m_graphite_inner, region=-s_pebble_inner)
c_pebble_central   = openmc.Cell(name='c_pebble_central',                        region=+s_pebble_inner   & -s_pebble_central)
c_pebble_outer     = openmc.Cell(name='c_pebble_inner'  , fill=m_graphite_outer, region=+s_pebble_central & -s_pebble_outer)
c_pebble_flibe     = openmc.Cell(name='c_flibe'         , fill=m_flibe         , region=+s_pebble_outer)
# Fill c_pebble_central with TRISO particles
if random_distribution==0:
# TRISO particles regular lattice using 'pitch_triso_lattice'
      print ("Pitch of the TRISO regular lattice (pitch_triso_lattice) =", pitch_triso_lattice, "[cm]")
      l_triso                    = openmc.RectLattice(name='l_triso')
      l_triso.lower_left         = (-pitch_triso_lattice/2, -pitch_triso_lattice/2, -pitch_triso_lattice/2)
      l_triso.pitch              = ( pitch_triso_lattice  ,  pitch_triso_lattice  ,  pitch_triso_lattice)
      l_triso.outer              = u_triso
      l_triso.universes          = np.tile(u_triso, (1,1,1))
#     l_triso.universes[0, 0, 0] = universe_triso
else:
# TRISO particles random distribution using 'packing_fraction'
      c_triso_pyc_outer_random   = openmc.Cell(name='c_triso_pyc_outer_random', fill=m_graphite_pyc, region=+s_sic)
      u_triso_random             = openmc.Universe(cells=[c_triso_fuel, c_triso_c_buffer, c_triso_pyc_inner, c_triso_sic, c_triso_pyc_outer_random])
      if not os.path.exists('triso_centers.npy'):
            print("Writing random TRISO centers to file triso_centers")
            r_triso_random             = +s_pebble_inner & -s_pebble_central
            spheres_random             = openmc.model.pack_spheres(radius=radius_pyc_outer, region=r_triso_random, pf=packing_fraction, initial_pf=0.15)
            triso_random               = [openmc.model.TRISO(radius_pyc_outer, u_triso_random, i) for i in spheres_random]
            print(triso_random[0])
            print(triso_random[1])
            centers = np.vstack([i.center for i in triso_random])
            print(centers.min(axis=0))
            print(centers.max(axis=0))
            print("Packing fraction of the TRISO particles random distribution =",len(triso_random)*radius_pyc_outer**3/(radius_pebble_central**3-radius_pebble_inner**3))
            np.save('triso_centers.npy', triso_random)
      print("Reading random TRISO centers from file triso_centers")
      triso_random = np.load('triso_centers.npy')
      lower_left, upper_right    = c_pebble_central.region.bounding_box
      shape                      = (5, 5, 5)
      pitch                      = (upper_right - lower_left)/shape
      l_triso                    = openmc.model.create_triso_lattice(triso_random, lower_left, pitch, shape, m_graphite_matrix)
c_pebble_central.fill   = l_triso
u_pebble                = openmc.Universe(cells=[c_pebble_inner, c_pebble_central, c_pebble_outer, c_pebble_flibe])
# Vessel
s_pebble_1              = openmc.Sphere(x0=center_1[0], y0=center_1[1], z0=center_1[2], R=radius_pebble_flibe, boundary_type='transmission')
s_pebble_2              = openmc.Sphere(x0=center_2[0], y0=center_2[1], z0=center_2[2], R=radius_pebble_flibe, boundary_type='transmission')
cz_vessel               = openmc.ZCylinder(x0=x_vessel, y0=y_vessel, R=radius_vessel, boundary_type='reflective') # transmission | reflective | vacuum
pz_vessel_bottom        = openmc.ZPlane(z0=z1_vessel, boundary_type='reflective')
pz_vessel_top           = openmc.ZPlane(z0=z2_vessel, boundary_type='reflective')
c_pebble_1              = openmc.Cell(name='c_pebble_1', fill=u_pebble, region=-s_pebble_1)
c_pebble_2              = openmc.Cell(name='c_pebble_2', fill=u_pebble, region=-s_pebble_2)
c_pebble_1.translation = center_1
c_pebble_2.translation = center_2
c_vessel                = openmc.Cell(name='c_vessel',   fill=m_flibe,  region = -cz_vessel & +pz_vessel_bottom & -pz_vessel_top & +s_pebble_1 & +s_pebble_2)
# Pebbles pseudo-random distribution
# c_pebble_outer_random   = openmc.Cell(name='c_pebble_outer_random', fill=m_graphite_outer, region=+s_pebble_outer)
# u_pebble_random         = openmc.Universe(cells=[c_pebble_inner, c_pebble_central, c_pebble_outer])
# print("Reading random pebble centers from file pebble_centers.txt")
# spheres_random          = np.loadtxt('pebble_centers.txt')
# spheres_random          = spheres_random*radius_pebble_outer # pebbles centers have cm unit and must be scaled by TAMU experiment factor
# print("File pebble_centers.txt reading completed")
# print(spheres_random)
# pebble_random           = [openmc.model.TRISO(radius_pebble_outer, u_pebble_random, i) for i in spheres_random]
# lower_left, upper_right = c_vessel.region.bounding_box
# shape                   = (5, 5, 5)
# pitch                   = (upper_right - lower_left)/shape 
# l_pebble                = openmc.model.create_triso_lattice(pebble_random, lower_left, pitch, shape, m_flibe)
# c_vessel.fill           = l_pebble
# Global geometry
u_zero                  = openmc.Geometry([c_vessel,c_pebble_1,c_pebble_2])
# -------------- Settings --------------
settings           = openmc.Settings()
settings.source    = openmc.Source(space=openmc.stats.Box(*c_pebble_1.bounding_box, only_fissionable=True))
settings.particles = 10000
settings.inactive  = 5 
settings.batches   = 10
# Fuel volume calculation
volume_fuel                  = openmc.VolumeCalculation([m_fuel], 10000000, *c_pebble_central.bounding_box)
settings.volume_calculations = [volume_fuel]
model              = openmc.model.Model(geometry=u_zero, settings=settings)
model.export_to_xml()
# -------------- Tally ---------------
# print(l_pebble)
# print(c_pebble_central)
# print(u_pebble_random)
# print(pebble_random)
# # filter1            = openmc.DistribcellFilter([13])
# filter1            = openmc.UniverseFilter([u_pebble_random])
# filter2            = openmc.CellFilter([13])
# tally              = openmc.Tally(name='tally kappa-fission')
# tally.filters      = [filter1, filter2]
# tally.scores       = ['kappa-fission']
# tallies            = openmc.Tallies([tally])
# tallies.export_to_xml()
# -------------- Plots --------------
m_colors = {m_fuel:              'brown',
            m_graphite_c_buffer: 'LightSteelBlue',
            m_graphite_pyc:      'blue',
            m_sic:               'orange',
            m_graphite_matrix:   'cyan',
            m_graphite_inner:    'DeepSkyBlue',
            m_graphite_outer:    'Navy',
            m_flibe:             'yellow'}

plot1          = openmc.Plot()
plot1.filename = 'xy_1'
plot1.width    = (2*radius_vessel, 2*radius_vessel)
plot1.basis    = 'xy'
plot1.origin   = center_1
plot1.pixels   = (1000, 1000)
plot1.color_by = 'material'
plot1.colors   = m_colors

plot2          = openmc.Plot()
plot2.filename = 'xy_2'
plot2.width    = (2*radius_vessel, 2*radius_vessel)
plot2.basis    = 'xy'
plot2.origin   = center_2
plot2.pixels   = (1000, 1000)
plot2.color_by = 'material'
plot2.colors   = m_colors

plot3          = openmc.Plot()
plot3.filename = 'yz'
plot3.width    = (height_vessel, height_vessel)
plot3.basis    = 'yz'
plot3.origin   = (0, 0 ,height_vessel/2+z1_vessel)
plot3.pixels   = (1000, 1000)
plot3.color_by = 'material'
plot3.colors   = m_colors

plots          = openmc.Plots([plot1, plot2, plot3])
if voxel==1:
   plotv1 = openmc.Plot()
   plotv1.filename = 'plotv1'
   plotv1.width    = (2*radius_pebble_outer, 2*radius_pebble_outer,  2*radius_pebble_outer)
   plotv1.origin   = (0, 0 ,0)
   plotv1.pixels   = (300, 300, 300)
   plotv1.color_by = 'material'
   plotv1.colors   = m_colors
   plotv1.type     = 'voxel'
   plots           = openmc.Plots([plotv1])
plots.export_to_xml()
openmc.plot_geometry(output=False)
