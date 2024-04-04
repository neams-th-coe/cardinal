#!/bin/env python
#********************************************************************/
#*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
#*                             Cardinal                             */
#*                                                                  */
#*                  (c) 2021 UChicago Argonne, LLC                  */
#*                        ALL RIGHTS RESERVED                       */
#*                                                                  */
#*                 Prepared by UChicago Argonne, LLC                */
#*               Under Contract No. DE-AC02-06CH11357               */
#*                With the U. S. Department of Energy               */
#*                                                                  */
#*             Prepared by Battelle Energy Alliance, LLC            */
#*               Under Contract No. DE-AC07-05ID14517               */
#*                With the U. S. Department of Energy               */
#*                                                                  */
#*                 See LICENSE for full restrictions                */
#********************************************************************/

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

def coolant_temp(t_in, t_out, l, z):
    """
    Computes the coolant temperature based on an expected cosine power distribution
    for a specified temperature rise. The total core temperature rise is governed
    by energy conservation as dT = Q / m / Cp, where dT is the total core temperature
    rise, Q is the total core power, m is the mass flowrate, and Cp is the fluid
    isobaric specific heat. If you neglect axial heat conduction and assume steady
    state, then the temperature rise in a layer of fluid i can be related to the
    ratio of the power in that layer to the total power,
    dT_i / dT = Q_i / Q. We assume here a sinusoidal power distribution to get
    a reasonable estimate of an initial coolant temperature distribution.

    Parameters
    ----------

    t_in : float
        Inlet temperature of the channel
    t_out : float
        Outlet temperature of the channel
    l : float
        Length of the channel
    z : float or 1-D numpy.array
        Axial position where the temperature will be computed

    Returns
    -------
        float or 1-D numpy array of float depending on z
    """
    dT = t_out - t_in
    Q = 2 * l / math.pi
    Qi = (l - l * np.cos(math.pi * z / l)) / math.pi

    t = t_in + Qi / Q * dT

    return t

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

def assembly(n_ax_zones, n_inactive, n_active, add_entropy_mesh=False):
    axial_section_height = reactor_height / n_ax_zones

    model = openmc.model.Model()

    enrichment = 0.155        # U-235 enrichment (weight percent)
    enrichment_234 = 2e-3     # U-234 enrichment (weight percent)
    kernel_density = 10820    # fissile kernel density (kg/m3)
    buffer_density = 1050     # buffer density (kg/m3)
    PyC_density = 1900        # PyC density (kg/m3)
    SiC_density = 3203        # SiC density (kg/m3)
    matrix_density = 1700     # graphite matrix density (kg/m3)

    # ----- uranium oxycarbide fuel ----- #
    m_fuel = openmc.Material(name='fuel')
    mass_234 = openmc.data.atomic_mass('U234')
    mass_235 = openmc.data.atomic_mass('U235')
    mass_238 = openmc.data.atomic_mass('U238')

    n_234 = enrichment_234 / mass_234
    n_235 = enrichment / mass_235
    n_238 = (1.0 - enrichment - enrichment_234) / mass_238
    total_n = n_234 + n_235 + n_238

    m_fuel.add_nuclide('U234', n_234 / total_n)
    m_fuel.add_nuclide('U235', n_235 / total_n)
    m_fuel.add_nuclide('U238', n_238 / total_n)
    m_fuel.add_element('C'   , 1.50)
    m_fuel.add_element('O'   , 0.50)
    m_fuel.set_density('kg/m3', kernel_density)

    # ----- graphite buffer ----- #
    m_graphite_c_buffer = openmc.Material(name='buffer')
    m_graphite_c_buffer.add_element('C', 1.0)
    m_graphite_c_buffer.add_s_alpha_beta('c_Graphite')
    m_graphite_c_buffer.set_density('kg/m3', buffer_density)

    # ----- pyrolitic carbon ----- #
    m_graphite_pyc = openmc.Material(name='pyc')
    m_graphite_pyc.add_element('C', 1.0)
    m_graphite_pyc.add_s_alpha_beta('c_Graphite')
    m_graphite_pyc.set_density('kg/m3', PyC_density)

    # ----- silicon carbide ----- #
    m_sic = openmc.Material(name='sic')
    m_sic.add_element('C' , 1.0)
    m_sic.add_element('Si', 1.0)
    m_sic.set_density('kg/m3', SiC_density)

    # ----- matrix graphite ----- #
    m_graphite_matrix = openmc.Material(name='graphite moderator')
    m_graphite_matrix.add_element('C', 1.0)
    m_graphite_matrix.add_s_alpha_beta('c_Graphite')
    m_graphite_matrix.set_density('kg/m3', matrix_density)

    # ----- helium coolant ----- #
    m_coolant = openmc.Material(name='Helium coolant')
    m_coolant.add_element('He', 1.0, 'ao')
    # we don't set density here because we'll set it as a function of temperature

    # Channel surfaces
    fuel_cyl = openmc.ZCylinder(r=0.5 * fuel_channel_diam)
    coolant_cyl = openmc.ZCylinder(r=0.5 * coolant_channel_diam)
    poison_cyl = openmc.ZCylinder(r=0.5 * fuel_channel_diam)
    graphite_cyl = openmc.ZCylinder(r=0.5 * fuel_channel_diam)

    # create a TRISO lattice for one axial section (to be used in the rest of the axial zones)
    # center the TRISO region on the origin so it fills lattice cells appropriately

    axial_coords = np.linspace(reactor_bottom, reactor_top, n_ax_zones + 1)
    lattice_univs = []
    bundle_univs = []

    m_colors = {}

    for z_min, z_max in zip(axial_coords[0:-1], axial_coords[1:]):
        # use the middle of the axial section to compute the temperature and density
        ax_pos = 0.5 * (z_min + z_max)
        T = coolant_temp(specs.inlet_T, coolant_outlet_temp, reactor_height, ax_pos)

        # create solid cells, which don't require us to clone materials in order to set temperatures
        fuel_ch_cell = openmc.Cell(region=-fuel_cyl, fill=m_fuel)
        fuel_ch_cell.temperature = T

        fuel_ch_matrix_cell = openmc.Cell(region=+fuel_cyl, fill=m_graphite_matrix)
        fuel_ch_matrix_cell.temperature = T

        poison_matrix_cell = openmc.Cell(region=+poison_cyl, fill=m_graphite_matrix)
        poison_matrix_cell.temperature = T

        graphite_cell = openmc.Cell(fill=m_graphite_matrix)
        graphite_cell.temperature = T

        coolant_matrix_cell = openmc.Cell(region=+coolant_cyl, fill=m_graphite_matrix)
        coolant_matrix_cell.temperature = T

        # create fluid cells, which require us to clone the material in order to be able to
        # set unique densities
        coolant_cell = openmc.Cell(region=-coolant_cyl, fill=m_coolant)
        coolant_cell.fill = [m_coolant.clone() for i in range(specs.n_coolant_channels_per_block - 3 * 6 * 2)]

        for mat in range(len(coolant_cell.fill)):
          m_colors[coolant_cell.fill[mat]] = 'red'

        # Define a universe for each type of solid-only pin (fuel, poison, and graphite)
        f = openmc.Universe(cells=[fuel_ch_cell, fuel_ch_matrix_cell])
        c = openmc.Universe(cells=[coolant_cell, coolant_matrix_cell])
        g = openmc.Universe(cells=[graphite_cell])

        d = [f] * 2

        ring2 = ([f] + [c]) * 6
        ring3 = ([c] + d) * 6
        ring4 = (d + [c] + [f]) * 6
        ring5 = ([f] + [c] + d + [c]) * 6
        ring6 = ([c] + d + [c] + d) * 6
        ring7 = (d + [c] + d + [c] + [f]) * 6
        ring8 = ([f] + [c] + d + [c] + d + [c]) * 6
        ring9 = [g] * 54

        # inner two rings where there aren't any fuel/compact/poison pins
        ring1 = [g] * 6
        ring0 = [g]

        lattice_univs.append([ring9, ring8, ring7, ring6, ring5, ring4, ring3, ring2, ring1, ring0])

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

    graphite_outer_cell = openmc.Cell(fill=m_graphite_matrix)
    graphite_outer_cell.temperature = T
    inf_graphite_univ = openmc.Universe(cells=[graphite_outer_cell])
    hex_lattice.outer = inf_graphite_univ

    # create additional axial regions
    axial_planes = [openmc.ZPlane(z0=coord) for coord in axial_coords]

    # axial planes
    min_z = openmc.ZPlane(z0=0.0, boundary_type = 'vacuum')
    max_z = openmc.ZPlane(z0=reactor_height, boundary_type = 'vacuum')

    # fill the unit cell with the hex lattice
    symmetry_plane = openmc.Plane(a=-math.sqrt(3.0)/2.0, b=0.5, c=0.0, d=0.0, boundary_type='reflective')
    symmetry_plane_2 = openmc.YPlane(boundary_type='reflective')
    hex_prism = openmc.model.HexagonalPrism(bundle_pitch / math.sqrt(3.0), 'x', boundary_type='vacuum')
    outer_cell = openmc.Cell(region=-hex_prism & +min_z & -max_z & -symmetry_plane & +symmetry_plane_2, fill=hex_lattice)

    model.geometry = openmc.Geometry([outer_cell])

    ### Settings ###
    settings = openmc.Settings()

    settings.particles = 100
    settings.inactive = n_inactive
    settings.batches = settings.inactive + n_active
    settings.temperature['method'] = 'interpolation'
    settings.temperature['range'] = (294.0, 1500.0)

    l = bundle_pitch / math.sqrt(3.0)
    lower_left = (-l, -l, reactor_bottom)
    upper_right = (l, l, reactor_top)
    source_dist = openmc.stats.Box(lower_left, upper_right, only_fissionable=True)
    source = openmc.IndependentSource(space=source_dist)
    settings.source = source

    if (add_entropy_mesh):
        entropy_mesh = openmc.RegularMesh()
        entropy_mesh.lower_left = lower_left
        entropy_mesh.upper_right = upper_right
        entropy_mesh.dimension = (6, 6, 20)
        settings.entropy_mesh = entropy_mesh

    vol_calc = openmc.VolumeCalculation([outer_cell],
                                        100_000_000, lower_left, upper_right)
    settings.volume_calculations = [vol_calc]

    model.settings = settings

    m_colors[m_fuel] = 'palegreen'
    m_colors[m_graphite_matrix] = 'darkblue'

    bundle_p_rounded = int(bundle_pitch)

    plot1          = openmc.Plot()
    plot1.filename = 'plot1'
    plot1.width    = (2 * bundle_pitch, 2 * axial_section_height)
    plot1.basis    = 'xz'
    plot1.origin   = (0.0, 0.0, reactor_height / 2.0)
    plot1.pixels   = (100 * 2 * bundle_p_rounded, int(100 * 3 * axial_section_height))
    plot1.color_by = 'cell'

    plot2          = openmc.Plot()
    plot2.filename = 'plot2'
    plot2.width    = (1.5 * bundle_pitch, 1.5 * bundle_pitch)
    plot2.basis    = 'xy'
    plot2.origin   = (0.0, 0.0, axial_section_height / 4.0)
    plot2.pixels   = (500 * bundle_p_rounded, 500 * bundle_p_rounded)
    plot2.color_by = 'material'
    plot2.colors   = m_colors

    plot3          = openmc.Plot()
    plot3.filename = 'plot3'
    plot3.width    = plot2.width
    plot3.basis    = plot2.basis
    plot3.origin   = plot2.origin
    plot3.pixels   = plot2.pixels
    plot3.color_by = 'cell'

    #model.plots = openmc.Plots([plot1, plot2, plot3])
    model.plots = openmc.Plots([plot2])

    return model


def main():

    ap = ArgumentParser()
    ap.add_argument('-n', dest='n_axial', type=int, default=1,
                    help='Number of axial cell divisions')
    ap.add_argument('-s', '--entropy', action='store_true',
                    help='Whether to add a Shannon entropy mesh')
    ap.add_argument('-i', dest='n_inactive', type=int, default=5,
                    help='Number of inactive cycles')
    ap.add_argument('-a', dest='n_active', type=int, default=5,
                    help='Number of active cycles')

    args = ap.parse_args()

    model = assembly(args.n_axial, args.n_inactive, args.n_active, args.entropy)
    model.export_to_xml()

if __name__ == "__main__":
    main()

