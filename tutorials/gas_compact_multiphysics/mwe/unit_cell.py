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
import materials as mats

m = 100.0
coolant_channel_diam = specs.channel_diameter * m
reactor_bottom = 0.0
reactor_height = specs.unit_cell_height * m
reactor_top = reactor_bottom + reactor_height
cell_pitch = specs.fuel_to_coolant_distance * m
fuel_channel_diam = specs.compact_diameter * m

def unit_cell():
    axial_section_height = reactor_height / specs.nl

    # superimposed search lattice
    triso_lattice_shape = (4, 4, int(axial_section_height))

    lattice_orientation = 'x'
    cell_edge_length = cell_pitch

    model = openmc.model.Model()

    ### Geometry ###

    # TRISO particle - keep it simple, use just one layer (just the kernel)
    radius_kernel = specs.kernel_radius*m
    s_fuel             = openmc.Sphere(r=radius_kernel)
    c_triso_fuel       = openmc.Cell(fill=mats.m_fuel,              region=-s_fuel)
    c_triso_matrix     = openmc.Cell(fill=mats.m_graphite_matrix,   region=+s_fuel)
    u_triso            = openmc.Universe(cells=[c_triso_fuel, c_triso_matrix])

    # Channel surfaces
    fuel_cyl = openmc.ZCylinder(r=0.5 * fuel_channel_diam)
    coolant_cyl = openmc.ZCylinder(r=0.5 * coolant_channel_diam)

    # create a TRISO lattice for one axial section (to be used in the rest of the axial zones)
    # center the TRISO region on the origin so it fills lattice cells appropriately
    min_z = openmc.ZPlane(z0=-0.5 * axial_section_height)
    max_z = openmc.ZPlane(z0=0.5 * axial_section_height)

    # region in which TRISOs are generated
    r_triso = -fuel_cyl & +min_z & -max_z

    rand_spheres = openmc.model.pack_spheres(radius=radius_kernel, region=r_triso, pf=specs.triso_pf, seed=1.0)
    random_trisos = [openmc.model.TRISO(radius_kernel, u_triso, i) for i in rand_spheres]

    llc, urc = r_triso.bounding_box
    pitch = (urc - llc) / triso_lattice_shape
    # insert TRISOs into a lattice to accelerate point location queries
    triso_lattice = openmc.model.create_triso_lattice(random_trisos, llc, pitch, triso_lattice_shape, mats.m_graphite_matrix)

    # create a hexagonal lattice for the coolant and fuel channels
    fuel_univ = openmc.Universe(cells=[openmc.Cell(region=-fuel_cyl, fill=triso_lattice),
                                    openmc.Cell(region=+fuel_cyl, fill=mats.m_graphite_matrix)])

    # extract the coolant cell and set temperatures based on the axial profile
    coolant_cell = openmc.Cell(region=-coolant_cyl, fill=mats.m_coolant)
    axial_coords = np.linspace(reactor_bottom, reactor_top, specs.nl + 1)
    lattice_univs = []

    fuel_ch_cells = []

    i = 0
    for z_min, z_max in zip(axial_coords[0:-1], axial_coords[1:]):
        i += 1

        # use the middle of the axial section to compute the temperature and density
        ax_pos = 0.5 * (z_min + z_max)

        # set the solid cells and their temperatures
        graphite_cell = openmc.Cell(region=+coolant_cyl, fill=mats.m_graphite_matrix)
        fuel_ch_cell = openmc.Cell(region=-fuel_cyl, fill=triso_lattice)
        fuel_ch_matrix_cell = openmc.Cell(region=+fuel_cyl, fill=mats.m_graphite_matrix)

        fuel_ch_cells.append(fuel_ch_cell)
        fuel_u = openmc.Universe(cells=[fuel_ch_cell, fuel_ch_matrix_cell])
        coolant_u = openmc.Universe(cells=[coolant_cell, graphite_cell])
        lattice_univs.append([[fuel_u] * 6, [coolant_u]])

    # create a hexagonal lattice used in each axial zone to represent the cell
    hex_lattice = openmc.HexLattice(name="Unit cell lattice")
    hex_lattice.orientation = lattice_orientation
    hex_lattice.center = (0.0, 0.0, 0.5 * (reactor_bottom + reactor_top))
    hex_lattice.pitch = (cell_pitch, axial_section_height)
    hex_lattice.universes = lattice_univs

    graphite_outer_cell = openmc.Cell(fill=mats.m_graphite_matrix)
    inf_graphite_univ = openmc.Universe(cells=[graphite_outer_cell])
    hex_lattice.outer = inf_graphite_univ

    # hexagonal bounding cell
    hex = openmc.model.HexagonalPrism(cell_edge_length, lattice_orientation, boundary_type='periodic')

    hex_cell_vol = 6.0 * (math.sqrt(3) / 4.0) * cell_edge_length**2 * reactor_height

    # create additional axial regions
    axial_planes = [openmc.ZPlane(z0=coord) for coord in axial_coords]
    # axial planes
    min_z = axial_planes[0]
    min_z.boundary_type = 'vacuum'
    max_z = axial_planes[-1]
    max_z.boundary_type = 'vacuum'

    # fill the unit cell with the hex lattice
    hex_cell = openmc.Cell(region=-hex & +min_z & -max_z, fill=hex_lattice)

    model.geometry = openmc.Geometry([hex_cell])

    ### Settings ###
    settings = openmc.Settings()

    settings.particles = 1500
    settings.inactive = 5
    settings.batches = 10

    # the only reason we use 'nearest' here is to be sure we have a robust test for CI;
    # otherwise, 1e-16 differences in temperature (due to numerical roundoff when using
    # different MPI ranks) do change the tracking do to the stochastic interpolation
    settings.temperature['method'] = 'nearest'
    settings.temperature['range'] = (294.0, 1500.0)
    settings.temperature['tolerance'] = 200.0
    settings.temperature['default'] = 500

    hexagon_half_flat = math.sqrt(3.0) / 2.0 * cell_edge_length
    lower_left = (-cell_edge_length, -hexagon_half_flat, reactor_bottom)
    upper_right = (cell_edge_length, hexagon_half_flat, reactor_top)
    source_dist = openmc.stats.Box(lower_left, upper_right)
    source = openmc.IndependentSource(space=source_dist)
    settings.source = source

    model.settings = settings

    m_colors = {}
    m_colors[mats.m_coolant] = 'royalblue'
    m_colors[mats.m_fuel] = 'red'
    m_colors[mats.m_graphite_c_buffer] = 'black'
    m_colors[mats.m_graphite_pyc] = 'orange'
    m_colors[mats.m_sic] = 'yellow'
    m_colors[mats.m_graphite_matrix] = 'silver'

    plot1          = openmc.Plot()
    plot1.filename = 'plot1'
    plot1.width    = (2 * cell_pitch, 4 * axial_section_height)
    plot1.basis    = 'xz'
    plot1.origin   = (0.0, 0.0, reactor_height/2.0)
    plot1.pixels   = (int(800 * 2 * cell_pitch), int(800 * 4 * axial_section_height))
    plot1.color_by = 'cell'

    plot2          = openmc.Plot()
    plot2.filename = 'plot2'
    plot2.width    = (3 * cell_pitch, 3 * cell_pitch)
    plot2.basis    = 'xy'
    plot2.origin   = (0.0, 0.0, axial_section_height / 2.0)
    plot2.pixels   = (int(800 * cell_pitch), int(800 * cell_pitch))
    plot2.color_by = 'material'
    plot2.colors   = m_colors

    plot3          = openmc.Plot()
    plot3.filename = 'plot3'
    plot3.width    = plot2.width
    plot3.basis    = plot2.basis
    plot3.origin   = plot2.origin
    plot3.pixels   = plot2.pixels
    plot3.color_by = 'cell'

    model.plots = openmc.Plots([plot1, plot2, plot3])

    return model


def main():
    model = unit_cell()
    model.export_to_xml()

if __name__ == "__main__":
    main()

