#!/usr/bin/env python
from argparse import ArgumentParser
import os
import math
from math import pi

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import scipy.stats

import openmc
import openmc.model


# -------------- Text Colors ----------------
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


# -------------- Unit Conversions -----------
mm = 0.1   # default cm
μm = 1e-4  # default cm

# -------------- Geometry Constants --------
# TRISO particle (UCBTH-14-002, Table 2-1)
radius_fuel = 400.0*μm/2
thickness_c_buffer = 100.0*μm
thickness_pyc_inner = 35.0*μm
thickness_sic = 35.0*μm
thickness_pyc_outer = 35.0*μm
radius_c_buffer = radius_fuel + thickness_c_buffer
radius_pyc_inner = radius_c_buffer + thickness_pyc_inner
radius_sic = radius_pyc_inner + thickness_sic
radius_pyc_outer = radius_sic + thickness_pyc_outer
packing_fraction = 0.40

# UCBTH-14-002, Table 2-1 (differs slightly from Cisneros, Table 5-2)
radius_pebble_inner = 2.5/2
# UCBTH-14-002, Table 2-1; Cisneros, Table 5-2
radius_pebble_outer = 3.0/2
# UCBTH-14-002, Table 2-1; Cisneros, Table 5-2
radius_pebble_central = radius_pebble_outer - 0.1
# Tolerance for pebbles universe filling
tolerance = 0.00
radius_pebble_flibe = radius_pebble_outer+tolerance


# -------------- Functions ------------------
def flibe_density(p, t):
    """
    Returns FLiBe density at a pressure, p,
    and temperature, t in units of kg/m3
    """
    # assumes the default value for drho_dp of 1.7324e-7
    rho = -0.4884*t + 1.7324e-7*(p - 101325.0) + 2413.0  # kg/m3
    return rho


def report_pebble_cell_level(geom: openmc.Geometry, pebble_cells):
    """
    Traverses the geometry until the cell name key
    is found in the cell name and reports the level
    of the pebble cells in the geometry for ease of
    use with Cardinal

    Note: this function assumes the pebble cells
    are all at the same level of the geometry
    """

    # ensure the pebble cell container is efficient
    peb_cells = set(pebble_cells)

    level = None

    univ_stack = [(0, geom.root_universe)]

    while True and univ_stack:
        curr_lvl, curr_univ = univ_stack.pop()
        cells = curr_univ.cells

        for cell in cells.values():
            # if one of the cells on this level
            # is a pebble cell, set the level and break
            if cell in peb_cells:
                level = curr_lvl
                break

            if isinstance(cell.fill, openmc.Universe):
                univ_stack.append((curr_lvl + 1, cell.fill))
            elif isinstance(cell.fill, openmc.Lattice):
                unique_univs = set(np.asarray(cell.fill.universes).flatten())
                univ_stack += [(curr_lvl + 1, u) for u in unique_univs]

    if level is not None:
        msg = ("Level of the pebble cells: {} \n"
              "(To be used as the 'pebble_cell_level' parameter "
              "for a cell tally in Cardinal's OpenMCProblem)".format(level))
        print("{}{}{}".format(bcolors.OKGREEN, msg, bcolors.ENDC))
    else:
        raise RuntimeError("Pebble cells were not"
                           "found in the geometry.")


# -------------- Materials Parameters --------
# TRISO particle
enrichment_uranium   = 0.199
fraction_u234        = 2e-3  # U234/(U234+U235) mass from Giacint facility fuel
density_fuel         = ('g/cm3', 10.5)  # UCBTH-14-002, Table 2-1
density_c_buffer     = ('g/cm3', 1.0)  # Cisneros, Table 3-1
density_pyc          = ('g/cm3', 1.87)  # Cisneros, Table 3-1
density_sic          = ('g/cm3', 3.2)  # Cisneros, Table 3-1
# Pebble Graphite
density_graphite_inner = ('g/cm3', 1.59368)  # Cisneros, Table 3-1
density_graphite_outer = ('g/cm3', 1.74)  # Cisneros, Table 3-1
# FLiBe coolant
enrichment_li7       = 0.99995
temperature_inlet    = 273.15 + 600.0  # UCBTH-14-002, Table 1-1
temperature_outlet   = 273.15 + 700.0  # UCBTH-14-002, Table 1-1
temperature_flibe    = (temperature_inlet + temperature_outlet)/2
rho_flibe            = flibe_density(101.325e3, temperature_flibe)
density_flibe        = ('kg/m3', rho_flibe)
# Reflector, a mixture of graphite and flibe.
reflector_porosity   = 0.112  # Novak thesis, Table 6.4, assuming 5 mm gaps b/w blocks
rho_graphite         = 1632.0  # Novak thesis, page 234
rho_reflector = reflector_porosity*rho_flibe + (1 - reflector_porosity)*rho_graphite
density_reflector    = ('kg/m3', rho_reflector)

# --------------------------------------------------
# NO CONSTANT DEFINITIONS BEYOND THIS LINE

# --------------------------- Main ----------------------------

program_description = ("Script for producing a pebble bed geometry "
                       "from a file of pebble centers.")

program_epilog = ("This script expects the presence of a "
                  "'pebble_centers_rescaled.txt' file in which "
                  "all pebble centers are represented for pebbles "
                  "with a radius of 1.5 cm.")

ap = ArgumentParser(description=program_description,
                    epilog=program_epilog)

ap.add_argument('-e', dest='extra_flibe_layer', type=float,
                default=0.00001, help='Flibe layer thickness')
ap.add_argument('-v', dest='verbose', action='store_true',
                default=False, help='Enable verbose output')
ap.add_argument('-r', dest='random_trisos', action='store_true',
                default=False, help='Enable random TRISO distribution')
ap.add_argument('-b', dest='bc', type=str,
                default='reflective', help='Reactor top/bottom B.C.')
ap.add_argument('-p', dest='particles', type=int,
                default=1000, help='Particles per batch')
ap.add_argument('-i', dest='inactive', type=int,
                default=10, help='Inactive batches')
ap.add_argument('-a', dest='active', type=int,
                default=40, help='Active batches')
ap.add_argument('-l', dest='pshape', type=int,
                nargs=3, default=(1, 1, 1),
                help='Pebble lattice shape')
ap.add_argument('-t', dest='tshape', type=int,
                nargs=3, default=(20, 20, 20),
                help='TRISO lattice shape')
ap.add_argument('-f', dest='radial_reflector_thickness', type=float,
                default=0.0, help='Include surrounding reflector on outer periphery')
ap.add_argument('-g', dest='axial_reflector_thickness', type=float,
                default=0.0, help='Include surrounding reflector on top/bottom of the bed')
args = ap.parse_args()

# warn user about pebble center expetation
scaling_warning = (
    "************************************************************\n"
    "This script expects pebble centers scaled to radius = 1.5 cm\n"
    "************************************************************"
    )
print("{}{}{}".format(bcolors.WARNING, scaling_warning, bcolors.ENDC))

# transfer argument values to script variables
random_distribution = args.random_trisos
verbose = args.verbose

# Pebble centers coordinates (x,y,z)
print("Reading pseudo-random pebble centers "
      "from file pebble_centers_rescaled.txt...")
pebble_centers = np.loadtxt('pebble_centers_rescaled.txt', delimiter=' ')
print("File pebble_centers_rescaled.txt reading completed")

if not random_distribution:
    pitch_triso_lattice  = radius_pyc_outer
    pitch_triso_lattice *= (4*pi/3/packing_fraction)**0.33333333333333333333333

# Vessel Dimensions
vessel_x, vessel_y = (0.0, 0.0)
vessel_z_min = np.min(pebble_centers[:, 2]) - radius_pebble_outer
vessel_z_max = np.max(pebble_centers[:, 2]) + radius_pebble_outer
vessel_outer_radius = np.max(np.linalg.norm(pebble_centers[:, :-1], axis=1)) + radius_pebble_outer
vessel_inner_radius = np.min(np.linalg.norm(pebble_centers[:, :-1], axis=1)) - radius_pebble_outer
vessel_inner_radius = max(0.0, vessel_inner_radius)

if args.verbose:
    print("\tMin pebble (center) z coordinate = {}".format(vessel_z_min))
    print("\tMax pebble (center) z coordinate = {}".format(vessel_z_max))
    print("\tMax pebble (center) r coordinate = {}".format(vessel_outer_radius))
    print("\tMin pebble (center) r coordinate = {}\n".format(vessel_inner_radius))

# Add additional moderation surrounding the pebble bed to reduce the
# artificially high k-eff due to reflecting boundary conditions
extra_thickness =  args.extra_flibe_layer  # cm
vessel_z_min -= extra_thickness
vessel_z_max += extra_thickness
vessel_outer_radius += extra_thickness
vessel_height = vessel_z_max - vessel_z_min

# Add outer reflector of thickness 40 cm, UCBTH-14-002. We assume that there is then also a reflector
# of the same thickness on the top and bottom (an arbitrary selection). This reflector surrounds the pebble bed
# around its outer radius as well as forms the center reflector column.
reflector_z_min = vessel_z_min - args.axial_reflector_thickness
reflector_z_max = vessel_z_max + args.axial_reflector_thickness
reflector_outer_radius = vessel_outer_radius + args.radial_reflector_thickness
radial_reflector_is_present = args.radial_reflector_thickness > 0.0
axial_reflector_is_present = args.axial_reflector_thickness > 0.0


# -------------- Printing Parameters ---------
if verbose:
    print("GEOMETRY PARAMETERS")
    print("TRISO particles radius_fuel           [cm] = {}".format(radius_fuel))
    print("TRISO particles radius_c_buffer       [cm] = {}".format(radius_c_buffer))
    print("TRISO particles radius_pyc_inner      [cm] = {}".format(radius_pyc_inner))
    print("TRISO particles radius_sic            [cm] = {}".format(radius_sic))
    print("TRISO particles radius_pyc_outer      [cm] = {}".format(radius_pyc_outer))
    print("TRISO particles packing fraction           = {}".format(packing_fraction))
    if not random_distribution:
        print("TRISO particles regular lattice pitch_triso_lattice [cm] = {}".format(pitch_triso_lattice))
    print("Pebble radius_pebble_inner   [cm] = {}".format(radius_pebble_inner))
    print("Pebble radius_pebble_outer   [cm] = {}".format(radius_pebble_outer))
    print("Pebble radius_pebble_central [cm] = {}".format(radius_pebble_central))
    print("Pebble radius_pebble_flibe   [cm] = {}".format(radius_pebble_flibe))
    print("Vessel radius outer          [cm] = {}".format(vessel_outer_radius))
    print("Vessel radius inner          [cm] = {}".format(vessel_inner_radius))
    print("Vessel z1_vessel (min z)     [cm] = {}".format(vessel_z_min))
    print("Vessel height_vessel         [cm] = {}".format(vessel_z_max - vessel_z_min))
    print("Radial reflector present     [--] = {}".format(radial_reflector_is_present))
    print("Axial reflector present      [--] = {}".format(axial_reflector_is_present))
    if radial_reflector_is_present or axial_reflector_is_present:
        print("Reflector radius outer       [cm] = {}".format(reflector_outer_radius))
        print("Reflector height             [cm] = {}".format(reflector_z_max - reflector_z_min))

    print("Pebbles centers [cm] :")
    print(pebble_centers)

# -------------- Material Definitions --------------
# TRISO particle
# Fuel from Nagley et al. Fabrication of Uranium Oxycarbide Kernels for HTR
# fuel https://inldigitallibrary.inl.gov/sites/sti/sti/4886646.pdf Table 2
m_fuel = openmc.Material(name='m_fuel - uranium oxycarbide - triso partciles')
m_fuel.add_nuclide('U234', 89.58*enrichment_uranium*fraction_u234, percent_type='wo')
m_fuel.add_nuclide('U235', 89.58*enrichment_uranium*(1 - fraction_u234), percent_type='wo')
m_fuel.add_nuclide('U238', 89.58*(1 - enrichment_uranium), percent_type='wo')
m_fuel.add_nuclide('C0', 1.80, percent_type='wo')
m_fuel.add_element('O', 8.62, percent_type='wo')
m_fuel.set_density(*density_fuel)

m_graphite_c_buffer = openmc.Material(name='m_graphite_c_buffer - triso partciles')
m_graphite_c_buffer.set_density(*density_c_buffer)
m_graphite_c_buffer.add_nuclide('C0', 1.0)
m_graphite_c_buffer.add_s_alpha_beta('c_Graphite')

m_graphite_pyc = openmc.Material(name='m_graphite_pyc - triso particles')
m_graphite_pyc.set_density(*density_pyc)
m_graphite_pyc.add_nuclide('C0', 1.0)
m_graphite_pyc.add_s_alpha_beta('c_Graphite')

m_sic = openmc.Material(name='sic - triso partciles')
m_sic.add_nuclide('C0', 1.0)
m_sic.add_element('Si', 1.0)
m_sic.set_density(*density_sic)
# TODO: Add SiC S(alpha, beta) ; ENDF/B-VIIIb4 has data for both Si and C in SiC
m_graphite_matrix = openmc.Material(name='m_graphite_matrix - triso particles')
m_graphite_matrix.set_density('g/cm3', 1.6)
m_graphite_matrix.add_nuclide('C0', 1.0)
m_graphite_matrix.add_s_alpha_beta('c_Graphite')
# TODO: Need real specifications for the carbon filler

# Pebble graphite
m_graphite_inner = openmc.Material(name='m_graphite_inner - pebble inner zone')
m_graphite_inner.set_density(*density_graphite_inner)
m_graphite_inner.add_nuclide('C0', 1.0)
m_graphite_inner.add_s_alpha_beta('c_Graphite')

m_graphite_outer = openmc.Material(name='m_graphite_outer - pebble outer zone')
m_graphite_outer.set_density(*density_graphite_outer)
m_graphite_outer.add_nuclide('C0', 1.0)
m_graphite_outer.add_s_alpha_beta('c_Graphite')

# FLiBe coolant - From Cisneros, appendix B, material 24
m_flibe = openmc.Material(name='m_flibe - 2LiF-BeF2')
m_flibe.set_density(*density_flibe)
m_flibe.add_nuclide('Li7', 2.0*enrichment_li7)
m_flibe.add_nuclide('Li6', 2.0*(1 - enrichment_li7))
m_flibe.add_element('Be', 1.0)
m_flibe.add_element('F', 4.0)
# TODO: FLiBe coolant - no S(alpha, beta) data available up to ENDF/B-VIIIb4
# Graphite-flibe homogenized reflector
flibe_molar_mass = m_flibe.average_molar_mass * 7
graphite_molar_mass = m_graphite_outer.average_molar_mass
# moles of flibe in 1 m^3
mols_flibe = reflector_porosity * (rho_flibe * 1000.0) / flibe_molar_mass
# moles of graphite in 1 m^3:
mols_graphite = (1.0 - reflector_porosity) * (rho_graphite * 1000.0) / graphite_molar_mass

# Then, the absolute number of moles is irrelevant since we're adding nuclides based on
# the atomic representation
m_reflector = openmc.Material(name='m_reflector')
m_reflector.add_nuclide('Li7', 2.0 * enrichment_li7 * mols_flibe)
m_reflector.add_nuclide('Li6', 2.0 * (1 - enrichment_li7) * mols_flibe)
m_reflector.add_element('Be' , 1.0 * mols_flibe)
m_reflector.add_element('F'  , 4.0 * mols_flibe)
m_reflector.add_nuclide('C0', 1.0 * mols_graphite)
m_reflector.add_s_alpha_beta('c_Graphite')
m_reflector.set_density(*density_reflector)

# -------------- Geometry Definition --------------
# TRISO particle universe
s_fuel             = openmc.Sphere(r=radius_fuel)
s_c_buffer         = openmc.Sphere(r=radius_c_buffer)
s_pyc_inner        = openmc.Sphere(r=radius_pyc_inner)
s_sic              = openmc.Sphere(r=radius_sic)
s_pyc_outer        = openmc.Sphere(r=radius_pyc_outer)
c_triso_fuel       = openmc.Cell(name='TRICO Fuel', fill=m_fuel, region=-s_fuel)
c_triso_c_buffer   = openmc.Cell(name='TRISO Graphite Buffer', fill=m_graphite_c_buffer, region=+s_fuel & -s_c_buffer)
c_triso_pyc_inner  = openmc.Cell(name='TRISO Pyrolitic Graphite Inner', fill=m_graphite_pyc, region=+s_c_buffer  & -s_pyc_inner)
c_triso_sic        = openmc.Cell(name='TIRSO Silicone Carbide', fill=m_sic, region=+s_pyc_inner & -s_sic)
c_triso_pyc_outer  = openmc.Cell(name='TRISO Pyrolitic Graphite Outer', fill=m_graphite_pyc, region=+s_sic & -s_pyc_outer)
c_triso_matrix     = openmc.Cell(name='TRISO Graphite Matrix', fill=m_graphite_matrix, region=+s_pyc_outer)

triso_cells = [c_triso_fuel, c_triso_c_buffer, c_triso_pyc_inner,
               c_triso_sic, c_triso_pyc_outer, c_triso_matrix]
u_triso = openmc.Universe(cells=triso_cells)

# Pebble Geometry
s_pebble_inner = openmc.Sphere(r=radius_pebble_inner)
s_pebble_central = openmc.Sphere(r=radius_pebble_central)
s_pebble_outer = openmc.Sphere(r=radius_pebble_outer)
c_pebble_inner = openmc.Cell(name='Pebble graphite inner region', fill=m_graphite_inner, region=-s_pebble_inner)
c_pebble_central = openmc.Cell(name='Pebble central region (TRISOs)', region=+s_pebble_inner & -s_pebble_central)
c_pebble_outer = openmc.Cell(name='Pebble graphite outer region', fill=m_graphite_outer, region=+s_pebble_central & -s_pebble_outer)
c_pebble_flibe = openmc.Cell(name='Pebble exterior (FLiBe)', fill=m_flibe, region=+s_pebble_outer)

l_triso = None
l_triso_shape = args.tshape
if verbose:
    print("TRISO lattice shape   [cm] = {}".format(l_triso_shape))

# Fill c_pebble_central with TRISO particles
if not random_distribution:
    print("Using regular TRISO distribution")
    # TRISO particles regular lattice using 'pitch_triso_lattice'
    l_triso = openmc.RectLattice(name='Pebble TRISO Lattice')
    l_triso.lower_left = (-pitch_triso_lattice/2, -pitch_triso_lattice/2, -pitch_triso_lattice/2)
    l_triso.pitch = (pitch_triso_lattice, pitch_triso_lattice, pitch_triso_lattice)
    l_triso.outer = u_triso
    l_triso.universes = np.tile(u_triso, l_triso_shape)
else:
    # TRISO particles random distribution using 'packing_fraction'
    print("Using random TRISO distribution")
    c_triso_pyc_outer_random = openmc.Cell(name='c_triso_pyc_outer_random',
                                           fill=m_graphite_pyc,
                                           region=+s_sic)
    triso_rand_cells = [c_triso_fuel,
                        c_triso_c_buffer,
                        c_triso_pyc_inner,
                        c_triso_sic,
                        c_triso_pyc_outer_random]
    u_triso_random = openmc.Universe(cells=triso_rand_cells)
    # cache the triso centers to avoid recomputation later
    if not os.path.exists('triso_centers.npy'):
        print("Writing random TRISO centers to file triso_centers.npy")
        r_triso_random = +s_pebble_inner & -s_pebble_central
        spheres_random = openmc.model.pack_spheres(radius=radius_pyc_outer,
                                                   region=r_triso_random,
                                                   pf=packing_fraction,
                                                   initial_pf=0.15)
        triso_random = [openmc.model.TRISO(radius_pyc_outer, u_triso_random, i) for i in spheres_random]
        triso_centers = np.vstack([i.center for i in triso_random])
    # report packing fraction if requested
    if verbose:
        packing_fraction = len(triso_random)*radius_pyc_outer**3/(radius_pebble_central**3-radius_pebble_inner**3)
        print("Calculated packing fraction of the TRISO "
              "particles random distribution = {}".format(packing_fraction))
        np.save('triso_centers.npy', triso_random)

    print("Reading random TRISO centers from file triso_centers.npy")
    triso_random = np.load('triso_centers.npy', allow_pickle=True)
    lower_left, upper_right = c_pebble_central.region.bounding_box
    shape = l_triso_shape
    pitch = (upper_right - lower_left)/shape
    l_triso = openmc.model.create_triso_lattice(triso_random,
                                                lower_left,
                                                pitch,
                                                shape,
                                                m_graphite_matrix)
    print("File triso_centers.npy reading completed")

if not l_triso:
    raise RuntimeError("TIRSO lattice is undefined.")

# Fil central pebble cell with the TRISO lattice
c_pebble_central.fill = l_triso

pebble_univ_cells = [c_pebble_inner,
                     c_pebble_central,
                     c_pebble_outer,
                     c_pebble_flibe]
u_pebble = openmc.Universe(cells=pebble_univ_cells)

# figure out the reactor boundary conditions
reactor_axial_bc = args.bc

if radial_reflector_is_present:
    vessel_radial_bc = 'transmission'
else:
    vessel_radial_bc = 'vacuum'

if axial_reflector_is_present:
    vessel_axial_bc = 'transmission'
else:
    vessel_axial_bc = reactor_axial_bc

# Reactor cells
vessel_outer = openmc.ZCylinder(x0=vessel_x,
                                y0=vessel_y,
                                r=vessel_outer_radius,
                                boundary_type=vessel_radial_bc)

vessel_bottom = openmc.ZPlane(z0=vessel_z_min,
                              boundary_type=vessel_axial_bc)
vessel_top = openmc.ZPlane(z0=vessel_z_max,
                           boundary_type=vessel_axial_bc)
vessel_region = -vessel_outer & +vessel_bottom & -vessel_top
vessel_cell = openmc.Cell(name='Pebble Vessel', region=vessel_region)

reflector_cells = []

# only a radial reflector
if radial_reflector_is_present and (not axial_reflector_is_present):
    vessel_inner = openmc.ZCylinder(x0=vessel_x,
                                    y0=vessel_y,
                                    r=vessel_inner_radius,
                                    boundary_type='transmission')

    vessel_cell.region = vessel_cell.region & +vessel_inner

    axial_plane = +vessel_bottom & -vessel_top

    # Reflector cell - for the axial BCs, this takes the same as the vessel axial BCs
    reflector_outer = openmc.ZCylinder(x0=vessel_x, y0=vessel_y, r=reflector_outer_radius, boundary_type='vacuum')
    outer_reflector_region = -reflector_outer & axial_plane & +vessel_outer
    inner_reflector_region = -vessel_inner & axial_plane
    outer_reflector_cell = openmc.Cell(name='Outer Reflector', region=outer_reflector_region, fill=m_reflector)
    inner_reflector_cell = openmc.Cell(name='Inner Reflector', region=inner_reflector_region, fill=m_reflector)
    reflector_cells = [outer_reflector_cell, inner_reflector_cell]

# only an axial reflector
if (not radial_reflector_is_present) and axial_reflector_is_present:

    reflector_bottom = openmc.ZPlane(z0=reflector_z_min, boundary_type=reactor_axial_bc)
    reflector_top = openmc.ZPlane(z0=reflector_z_max, boundary_type=reactor_axial_bc)
    top_reflector_region = +vessel_top & -reflector_top & -vessel_outer
    bottom_reflector_region = -vessel_bottom & +reflector_bottom & -vessel_outer
    top_reflector_cell = openmc.Cell(name='Top Reflector', region=top_reflector_region, fill=m_reflector)
    bottom_reflector_cell = openmc.Cell(name='Bottom Reflector', region=bottom_reflector_region, fill=m_reflector)
    reflector_cells = [top_reflector_cell, bottom_reflector_cell]

if radial_reflector_is_present and axial_reflector_is_present:
    vessel_inner = openmc.ZCylinder(x0=vessel_x,
                                    y0=vessel_y,
                                    r=vessel_inner_radius,
                                    boundary_type=vessel_radial_bc) # transmission

    vessel_cell.region = vessel_cell.region & +vessel_inner

    # Reflector cell
    reflector_outer = openmc.ZCylinder(x0=vessel_x, y0=vessel_y, r=reflector_outer_radius, boundary_type='vacuum')
    reflector_bottom = openmc.ZPlane(z0=reflector_z_min, boundary_type=reactor_axial_bc)
    reflector_top = openmc.ZPlane(z0=reflector_z_max, boundary_type=reactor_axial_bc)

    axial_plane = +vessel_bottom & -vessel_top

    # the outer reflector has an axial height from vessel_bottom to vessel_top, and the axial reflector
    # covers the remaining space
    outer_reflector_region = -reflector_outer & axial_plane & +vessel_inner
    inner_reflector_region = -vessel_inner & axial_plane
    top_reflector_region = +vessel_top & -reflector_top & -reflector_outer
    bottom_reflector_region = -vessel_bottom & +reflector_bottom & -reflector_outer
    outer_reflector_cell = openmc.Cell(name='Outer Reflector', region=outer_reflector_region, fill=m_reflector)
    inner_reflector_cell = openmc.Cell(name='Inner Reflector', region=inner_reflector_region, fill=m_reflector)
    top_reflector_cell = openmc.Cell(name='Top Reflector', region=top_reflector_region, fill=m_reflector)
    bottom_reflector_cell = openmc.Cell(name='Bottom Reflector', region=bottom_reflector_region, fill=m_reflector)
    reflector_cells = [outer_reflector_cell, inner_reflector_cell, top_reflector_cell, bottom_reflector_cell]

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
l_pebble_shape = np.asarray(args.pshape)
l_pebble_pitch = (urc_vessel - llc_vessel)/l_pebble_shape
l_pebble = openmc.model.create_triso_lattice(pebble_trisos,
                                             llc_vessel,
                                             l_pebble_pitch,
                                             l_pebble_shape,
                                             m_flibe)
l_pebble.name = "Pebble cell lattice"

# fill vessel with pebble lattice
vessel_cell.fill = l_pebble

# create geometry
geom_cells = [vessel_cell] + reflector_cells
geom = openmc.Geometry(geom_cells)

# assumes the pebble cells are all at the same level of the geometry
report_pebble_cell_level(geom, pebble_cells)

if verbose:
    print("Cell of the vessel:")
    print(vessel_cell)

# -------------- Settings --------------------
settings = openmc.Settings()
source_space = openmc.stats.Box(*vessel_cell.bounding_box,
                                only_fissionable=True)
settings.source    = openmc.Source(space=source_space)
settings.particles = args.particles
settings.inactive  = args.inactive
settings.batches   = args.inactive + args.active
settings.temperature = {'default': 573.0,
                        'method': 'interpolation',
                        'multipole': True,
                        'range': (300.0, 1500.0),
                        'tolerance': 1000.0}

# Fuel volume calculation
volume_fuel = openmc.VolumeCalculation([m_fuel], 10000000, *c_pebble_central.bounding_box)
settings.volume_calculations = [volume_fuel]
settings.output = {'summary': False}

# -------------- Plots --------------
# Plot parameters
reactor_diameter = 2.0 * reflector_outer_radius
reactor_height = reflector_z_max - reflector_z_min
pebble_diameter = 2.0 * radius_pebble_outer

m_colors = {m_fuel: 'brown',
            m_graphite_c_buffer: 'LightSteelBlue',
            m_graphite_pyc: 'blue',
            m_sic: 'orange',
            m_graphite_matrix: 'cyan',
            m_graphite_inner: 'DeepSkyBlue',
            m_graphite_outer: 'Navy',
            m_flibe: 'yellow'}

if radial_reflector_is_present or axial_reflector_is_present:
    m_colors[m_reflector] = 'green'

plot1          = openmc.Plot()
plot1.filename = 'plot1'
plot1.width    = (reactor_diameter, reactor_diameter)
plot1.basis    = 'xy'
plot1.origin   = (0, 0, 0)
plot1.pixels   = (1000, 1000)
plot1.color_by = 'material'
plot1.colors   = m_colors

plot2          = openmc.Plot()
plot2.filename = 'plot2'
plot2.width    = (reactor_diameter, reactor_diameter)
plot2.basis    = 'xy'
plot2.origin   = (0, 0, 1)
plot2.pixels   = (1000, 1000)
plot2.color_by = 'cell'
plot2.colors   = m_colors

plot3          = openmc.Plot()
plot3.filename = 'plot3'
plot_width   = max(reactor_diameter, reactor_height)
plot3.width    = (plot_width, plot_width)
plot3.basis    = 'xz'
plot_zcenter = reflector_z_min + reactor_height/2.0
plot3.origin   = (0, 0, plot_zcenter)
plot3.pixels   = (1000, 1000)
plot3.color_by = 'material'
plot3.colors   = m_colors

plotv1 = openmc.Plot()
plotv1.filename = 'plotv1'
plotv1.width    = (pebble_diameter, pebble_diameter, pebble_diameter)
plotv1.origin   = (0, 0, 0)
plotv1.pixels   = (300, 300, 300)
plotv1.color_by = 'material'
plotv1.colors   = m_colors
plotv1.type     = 'voxel'

plots = openmc.Plots([plot1, plot2, plot3, plotv1])

# Create model and export
model = openmc.model.Model(geometry=geom, settings=settings)
model.plots = plots
model.export_to_xml()
