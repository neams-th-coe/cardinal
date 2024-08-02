#!/bin/env python

import math
import numpy as np
import openmc
import sys
import os

# options: generic_lattice, material.
# FAILS for: generic_lattice
fill_by = 'generic_lattice'

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

    lattice_orientation = 'x'
    cell_edge_length = cell_pitch

    model = openmc.model.Model()

    fuel_cyl = openmc.ZCylinder(r=0.5 * fuel_channel_diam)
    coolant_cyl = openmc.ZCylinder(r=0.5 * coolant_channel_diam)

    # alternatively, lets try a generic lattice to see if the nature of the issue is TRISO
    # or any old LATTICE
    unis = openmc.Universe(cells=[openmc.Cell(fill=mats.m_fuel)])
    generic_lattice = openmc.HexLattice()
    generic_lattice.center = (0.0, 0.0)
    generic_lattice.pitch = (cell_pitch,)
    generic_lattice.universes = [[unis] * 6, [unis]]

    # move axially through layers, create the universes for each layer
    lattice_univs = []
    axial_coords = np.linspace(reactor_bottom, reactor_top, specs.nl + 1)
    for z_min, z_max in zip(axial_coords[0:-1], axial_coords[1:]):
        graphite_cell = openmc.Cell(region=+coolant_cyl, fill=mats.m_graphite_matrix)

        if (fill_by == 'generic_lattice'):
            fuel_ch_cell = openmc.Cell(region=-fuel_cyl, fill=generic_lattice)
        elif (fill_by == 'material'):
            fuel_ch_cell = openmc.Cell(region=-fuel_cyl, fill=mats.m_fuel)

        fuel_ch_matrix_cell = openmc.Cell(region=+fuel_cyl, fill=mats.m_graphite_matrix)
        coolant_cell = openmc.Cell(region=-coolant_cyl, fill=mats.m_coolant)

        fuel_u = openmc.Universe(cells=[fuel_ch_cell, fuel_ch_matrix_cell])
        coolant_u = openmc.Universe(cells=[coolant_cell, graphite_cell])
        lattice_univs.append([[fuel_u] * 6, [coolant_u]])

    # create a hexagonal lattice used in each axial zone to represent the cell
    hex_lattice = openmc.HexLattice(name="Unit cell lattice")
    hex_lattice.orientation = lattice_orientation
    hex_lattice.center = (0.0, 0.0, 0.5 * (reactor_bottom + reactor_top))
    hex_lattice.pitch = (cell_pitch, axial_section_height)
    hex_lattice.universes = lattice_univs

    # hexagonal bounding cell
    hex = openmc.model.HexagonalPrism(cell_edge_length, lattice_orientation, boundary_type='periodic')

    # create additional axial regions
    axial_planes = [openmc.ZPlane(z0=coord) for coord in axial_coords]
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

