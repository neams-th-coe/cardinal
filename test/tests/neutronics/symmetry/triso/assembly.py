#!/bin/env python

from argparse import ArgumentParser
import math
import numpy as np
import matplotlib.pyplot as plt

import openmc
import sys
import os

# Get common input parameters shared by other physics
script_dir = os.path.dirname(__file__)
sys.path.append(script_dir)
import common_input as specs
import compact_materials as compact_mats

def coolant_density(t):
  """
  Computes the helium density from temperature assuming a fixed operating pressure.

  Parameters
  ----------

  t : float
    Fluid temperature

  Returns
  _______
    float or 1-D numpy array of float depending on t
  """

  p_in_bar = specs.outlet_P * 1.0e-5;
  return 48.14 * p_in_bar / (t + 0.4446 * p_in_bar / math.pow(t, 0.2));

# -------------- Unit Conversions: OpenMC requires cm -----------
m = 100.0

# estimate the outlet temperature using bulk energy conservation for steady state
coolant_outlet_temp = specs.power / specs.mdot / specs.fluid_Cp + specs.inlet_T

# geometry parameters
coolant_channel_diam = specs.channel_diameter * m
reactor_bottom = 0.0
reactor_height = specs.height * m
reactor_top = reactor_bottom + reactor_height
bundle_pitch = specs.bundle_flat_to_flat * m + specs.bundle_gap_width * m
cell_pitch = specs.fuel_to_coolant_distance * m
fuel_channel_diam = specs.compact_diameter * m

def assembly(n_ax_zones, n_inactive, n_active):
    axial_section_height = reactor_height / n_ax_zones

    # superimposed search lattice
    triso_lattice_shape = (4, 4, int(axial_section_height / 0.5))

    model = openmc.model.Model()

    # TRISO particle
    radius_pyc_outer   = specs.oPyC_radius * m

    s_fuel             = openmc.Sphere(r=specs.kernel_radius*m)
    s_c_buffer         = openmc.Sphere(r=specs.buffer_radius*m)
    s_pyc_inner        = openmc.Sphere(r=specs.iPyC_radius*m)
    s_sic              = openmc.Sphere(r=specs.SiC_radius*m)
    s_pyc_outer        = openmc.Sphere(r=radius_pyc_outer)
    c_triso_fuel       = openmc.Cell(name='c_triso_fuel'     , fill=compact_mats.m_fuel,              region=-s_fuel)
    c_triso_c_buffer   = openmc.Cell(name='c_triso_c_buffer' , fill=compact_mats.m_graphite_c_buffer, region=+s_fuel      & -s_c_buffer)
    c_triso_pyc_inner  = openmc.Cell(name='c_triso_pyc_inner', fill=compact_mats.m_graphite_pyc,      region=+s_c_buffer  & -s_pyc_inner)
    c_triso_sic        = openmc.Cell(name='c_triso_sic'      , fill=compact_mats.m_sic,               region=+s_pyc_inner & -s_sic)
    c_triso_pyc_outer  = openmc.Cell(name='c_triso_pyc_outer', fill=compact_mats.m_graphite_pyc,      region=+s_sic       & -s_pyc_outer)
    c_triso_matrix     = openmc.Cell(name='c_triso_matrix'   , fill=compact_mats.m_graphite_matrix,   region=+s_pyc_outer)
    u_triso            = openmc.Universe(cells=[c_triso_fuel, c_triso_c_buffer, c_triso_pyc_inner, c_triso_sic, c_triso_pyc_outer, c_triso_matrix])

    # Channel surfaces
    fuel_cyl = openmc.ZCylinder(r=0.5 * fuel_channel_diam)
    coolant_cyl = openmc.ZCylinder(r=0.5 * coolant_channel_diam)
    poison_cyl = openmc.ZCylinder(r=0.5 * fuel_channel_diam)
    graphite_cyl = openmc.ZCylinder(r=0.5 * fuel_channel_diam)

    # create a TRISO lattice for one axial section (to be used in the rest of the axial zones)
    # center the TRISO region on the origin so it fills lattice cells appropriately
    min_z = openmc.ZPlane(z0=-0.5 * axial_section_height)
    max_z = openmc.ZPlane(z0=0.5 * axial_section_height)

    # region in which TRISOs are generated
    r_triso = -fuel_cyl & +min_z & -max_z

    rand_spheres = openmc.model.pack_spheres(radius=radius_pyc_outer, region=r_triso, pf=0.05, seed=1)
    random_trisos = [openmc.model.TRISO(radius_pyc_outer, u_triso, i) for i in rand_spheres]

    llc, urc = r_triso.bounding_box
    pitch = (urc - llc) / triso_lattice_shape

    # insert TRISOs into a lattice to accelerate point location queries
    triso_lattice = openmc.model.create_triso_lattice(random_trisos, llc, pitch, triso_lattice_shape, compact_mats.m_graphite_matrix)

    axial_coords = np.linspace(reactor_bottom, reactor_top, n_ax_zones + 1)
    lattice_univs = []
    bundle_univs = []

    m_colors = {}

    for z_min, z_max in zip(axial_coords[0:-1], axial_coords[1:]):
        # use the middle of the axial section to compute the temperature and density
        ax_pos = 0.5 * (z_min + z_max)

        # create solid cells, which don't require us to clone materials in order to set temperatures
        fuel_ch_cell = openmc.Cell(region=-fuel_cyl, fill=triso_lattice)

        fuel_ch_matrix_cell = openmc.Cell(region=+fuel_cyl, fill=compact_mats.m_graphite_matrix)

        poison_matrix_cell = openmc.Cell(region=+poison_cyl, fill=compact_mats.m_graphite_matrix)

        graphite_cell = openmc.Cell(fill=compact_mats.m_graphite_matrix)

        coolant_matrix_cell = openmc.Cell(region=+coolant_cyl, fill=compact_mats.m_graphite_matrix)

        # create fluid cells, which require us to clone the material in order to be able to
        # set unique densities
        coolant_cell = openmc.Cell(region=-coolant_cyl, fill=compact_mats.m_coolant)
        n_mats = 1
        coolant_cell.fill = [compact_mats.m_coolant.clone() for i in range(n_mats)]

        for mat in range(len(coolant_cell.fill)):
          m_colors[coolant_cell.fill[mat]] = 'red'

        # Define a universe for each type of solid-only pin (fuel, poison, and graphite)
        f = openmc.Universe(cells=[fuel_ch_cell, fuel_ch_matrix_cell])
        c = openmc.Universe(cells=[coolant_cell, coolant_matrix_cell])
        g = openmc.Universe(cells=[graphite_cell])

        # dummy void cell for the fuel compacts that are cut out of the domain by symmetry
        void_cell = openmc.Cell(fill=None)
        v = openmc.Universe(cells=[void_cell])

        d = [f] * 2

        side = [f] + [c]
        ring2 = side * 2 + [v] * 5 + [c] + side

        side = [c] + d
        ring3 = side + [c] + [f] + [v] * 9 + [f] + side

        side = d + [c] + [f]
        ring4 = side + [f] * 2 + [c] + [v] * 11 + [c] + [f] + side

        side = [f] + [c] + d + [c]
        ring5 = side + [f] + [c] + [f] + [v] * 15 + [f] + [c] + side

        #side = [c] + d + [c] + d
        #ring6 = side + [c] + [f] * 2 + [c] + [v] * 17 + [c] + [f] * 2 + side

        #side = d + [c] + d + [c] + [f]
        #ring7 = side + d + [c] + [f] + [v] * 21 + [f] + [c] + [f] + side

        #side = [f] + [c] + d + [c] + d + [c]
        #ring8 = side + [f] + [c] + d + [c] + [v] * 23 + [c] + d + [c] + side

        #side = [c] + d + [c] + d + [c] + d
        #ring9 = side + [c] + d + [c] + [f] + [v] * 27 + [f] + [c] + d + side

        #side = [p] + [f] + [c] + d + [c] + d + [c] + [f]
        #ring10 = side + [p] + [f] + [c] + d + [c] + [v] * 29 + [c] + d + [c] + [f] + side

        #ring11 = [g] * 66
        ring11 = [g] * 36

        # inner two rings where there aren't any fuel/compact/poison pins
        ring1 = [g] * 6
        ring0 = [g]

        lattice_univs.append([ring11, ring5, ring4, ring3, ring2, ring1, ring0])

    # create a hexagonal lattice used in each axial zone to represent the cell
    hex_lattice = openmc.HexLattice(name="Bundle cell lattice")
    hex_lattice.orientation = 'x'
    hex_lattice.center = (0.0, 0.0, 0.5 * (reactor_bottom + reactor_top))
    hex_lattice.pitch = (cell_pitch, axial_section_height)
    hex_lattice.universes = lattice_univs

    hexagon_volume = reactor_height * math.sqrt(3) / 2.0 * bundle_pitch**2
    coolant_channel_volume = math.pi * coolant_channel_diam**2 / 4.0 * reactor_height
    print('Volume of fuel bundle (m3): ', hexagon_volume / (100**3))
    print('Volume of solid regions in fuel bundle (m3): ', (hexagon_volume - 108 * coolant_channel_volume) / (100**3))

    graphite_outer_cell = openmc.Cell(fill=compact_mats.m_graphite_matrix)
    inf_graphite_univ = openmc.Universe(cells=[graphite_outer_cell])
    hex_lattice.outer = inf_graphite_univ

    # create additional axial regions
    axial_planes = [openmc.ZPlane(z0=coord) for coord in axial_coords]

    # axial planes
    min_z = openmc.ZPlane(z0=0.0, boundary_type='reflective')
    max_z = openmc.ZPlane(z0=reactor_height, boundary_type='reflective')
    symmetry_plane = openmc.XPlane(x0=0, boundary_type='reflective')

    # fill the unit cell with the hex lattice
    hex_prism = openmc.model.HexagonalPrism(bundle_pitch / math.sqrt(3.0), 'x', boundary_type='reflective')
    outer_cell = openmc.Cell(region=-hex_prism & +min_z & -max_z & +symmetry_plane, fill=hex_lattice)

    model.geometry = openmc.Geometry([outer_cell])

    ### Settings ###
    settings = openmc.Settings()

    settings.particles = 500
    settings.inactive = n_inactive
    settings.batches = settings.inactive + n_active
    settings.temperature['method'] = 'interpolation'
    settings.temperature['range'] = (294.0, 1500.0)
    settings.temperature['default'] = 600.0

    l = bundle_pitch / math.sqrt(3.0)
    lower_left = (0, -l, reactor_bottom)
    upper_right = (l, l, reactor_top)
    source_dist = openmc.stats.Box(lower_left, upper_right)
    source = openmc.IndependentSource(space=source_dist)
    settings.source = source

    model.settings = settings

    return model


def main():

    ap = ArgumentParser()
    ap.add_argument('-n', dest='n_axial', type=int, default=1,
                    help='Number of axial cell divisions')
    ap.add_argument('-i', dest='n_inactive', type=int, default=5,
                    help='Number of inactive cycles')
    ap.add_argument('-a', dest='n_active', type=int, default=5,
                    help='Number of active cycles')

    args = ap.parse_args()

    model = assembly(args.n_axial, args.n_inactive, args.n_active)
    model.export_to_xml()

if __name__ == "__main__":
    main()

