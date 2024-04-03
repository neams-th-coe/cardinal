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

# The TRISO particles in this problem are just solid fuel, to allow us to
# get better thread reproducible behavior (less surfaces)

channel_diameter = 0.016                 # diameter of the coolant channels (m)
compact_diameter = 0.0127                # diameter of fuel compacts (m)
fuel_to_coolant_distance = 0.0188        # distance between center of fuel compact and coolant channel (m)
height = 0.1                             # height of the unit cell (m)
triso_pf = 0.05                          # TRISO packing fraction
kernel_radius = 214.85e-6                # fissile kernel outer radius (m)
oPyC_radius = 429.85e-6                  # outer PyC outer radius (m)
enrichment = 0.155                       # fuel enrichment (weight percent)
kernel_density = 10820                   # fissile kernel density (kg/m3)
matrix_density = 1700                    # graphite matrix density (kg/m3)

temp = 300.0
helium_temp = 500.0

m = 100.0

# geometry
coolant_channel_diam = channel_diameter * m
reactor_bottom = 0.0
reactor_height = height * m
reactor_top = reactor_bottom + reactor_height

cell_pitch = fuel_to_coolant_distance * m
fuel_channel_diam = compact_diameter * m

hex_orientation = 'x'

def unit_cell(n_ax_zones, n_inactive, n_active):
    axial_section_height = reactor_height / n_ax_zones

    # superimposed search lattice
    triso_lattice_shape = (4, 4, int(axial_section_height / 0.125))

    lattice_orientation = 'x'
    cell_edge_length = cell_pitch

    model = openmc.model.Model()

    ### Materials ###
    enrichment_234        = 2e-3

    # TRISO Materials

    m_fuel = openmc.Material(name='fuel')
    enrichment_uranium = enrichment
    mass_234 = openmc.data.atomic_mass('U234')
    mass_235 = openmc.data.atomic_mass('U235')
    mass_238 = openmc.data.atomic_mass('U238')

    # number of atoms in one gram of uranium mixture
    n_234 = enrichment_234 / mass_234
    n_235 = enrichment_uranium / mass_235
    n_238 = (1.0 - enrichment_uranium - enrichment_234) / mass_238
    total_n = n_234 + n_235 + n_238

    m_fuel.add_nuclide('U234', n_234 / total_n)
    m_fuel.add_nuclide('U235', n_235 / total_n)
    m_fuel.add_nuclide('U238', n_238 / total_n)
    m_fuel.add_element('C'   , 1.50)
    m_fuel.add_element('O'   , 0.50)
    m_fuel.set_density('kg/m3', kernel_density)
    m_fuel.temperature = temp

    # Water moderator - we avoid graphite for testing because we can get differences
    # that depend on the number of threads because the high scattering nature of graphite
    # results in many situations where the collision distance is almost the same as the
    # distance to a surface (and this TRISO geometry has TONS of surfaces)
    m_water_matrix = openmc.Material(name='water')
    m_water_matrix.add_element('H', 2.0)
    m_water_matrix.add_element('O', 1.0)
    m_water_matrix.set_density('kg/m3', 100.0)
    m_water_matrix.temperature = temp

    # Coolant
    m_coolant = openmc.Material(name='Helium coolant')
    m_coolant.add_element('He', 1.0, 'ao')
    m_coolant.set_density('kg/m3', 1.0)
    m_coolant.temperature = helium_temp

    ### Geometry ###

    # TRISO particle
    radius_pyc_outer       = oPyC_radius * m

    s_fuel             = openmc.Sphere(r=radius_pyc_outer*m)
    c_triso_fuel       = openmc.Cell(name='c_triso_fuel'     , fill=m_fuel,              region=-s_fuel)
    c_triso_matrix     = openmc.Cell(name='c_triso_matrix'   , fill=m_water_matrix,   region=+s_fuel)
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

    rand_spheres = openmc.model.pack_spheres(radius=radius_pyc_outer, region=r_triso, pf=triso_pf)
    random_trisos = [openmc.model.TRISO(radius_pyc_outer, u_triso, i) for i in rand_spheres]

    llc, urc = r_triso.bounding_box
    pitch = (urc - llc) / triso_lattice_shape
    # insert TRISOs into a lattice to accelerate point location queries
    triso_lattice = openmc.model.create_triso_lattice(random_trisos, llc, pitch, triso_lattice_shape, m_water_matrix)

    # create a hexagonal lattice for the coolant and fuel channels
    fuel_univ = openmc.Universe(cells=[openmc.Cell(region=-fuel_cyl, fill=triso_lattice),
                                    openmc.Cell(region=+fuel_cyl, fill=m_water_matrix)])

    # extract the coolant cell and set temperatures based on the axial profile
    coolant_cell = openmc.Cell(region=-coolant_cyl, fill=m_coolant)
    # set the coolant temperature on the cell to approximately match the expected
    # temperature profile
    axial_coords = np.linspace(reactor_bottom, reactor_top, n_ax_zones + 1)
    lattice_univs = []

    fuel_ch_cells = []

    i = 0
    for z_min, z_max in zip(axial_coords[0:-1], axial_coords[1:]):
        # create a new coolant universe for each axial zone in the coolant channel;
        # this generates a new material as well (we only need to do this for all
        # cells except the first cell)
        if (i == 0):
          c_cell = coolant_cell
        else:
          c_cell = coolant_cell.clone()

        i += 1

        # set the solid cells and their temperatures
        graphite_cell = openmc.Cell(region=+coolant_cyl, fill=m_water_matrix)
        fuel_ch_cell = openmc.Cell(region=-fuel_cyl, fill=triso_lattice)
        fuel_ch_matrix_cell = openmc.Cell(region=+fuel_cyl, fill=m_water_matrix)

        fuel_ch_cells.append(fuel_ch_cell)
        fuel_u = openmc.Universe(cells=[fuel_ch_cell, fuel_ch_matrix_cell])
        coolant_u = openmc.Universe(cells=[c_cell, graphite_cell])
        lattice_univs.append([[fuel_u] * 6, [coolant_u]])

    # create a hexagonal lattice used in each axial zone to represent the cell
    hex_lattice = openmc.HexLattice(name="Unit cell lattice")
    hex_lattice.orientation = lattice_orientation
    hex_lattice.center = (0.0, 0.0, 0.5 * (reactor_bottom + reactor_top))
    hex_lattice.pitch = (cell_pitch, axial_section_height)
    hex_lattice.universes = lattice_univs

    graphite_outer_cell = openmc.Cell(fill=m_water_matrix)
    inf_graphite_univ = openmc.Universe(cells=[graphite_outer_cell])
    hex_lattice.outer = inf_graphite_univ

    # hexagonal bounding cell
    hex = openmc.model.HexagonalPrism(cell_edge_length, hex_orientation, boundary_type='periodic')

    hex_cell_vol = 6.0 * (math.sqrt(3) / 4.0) * cell_edge_length**2 * reactor_height

    # create additional axial regions
    axial_planes = [openmc.ZPlane(z0=coord) for coord in axial_coords]
    # axial planes
    min_z = axial_planes[0]
    min_z.boundary_type = 'reflective'
    max_z = axial_planes[-1]
    max_z.boundary_type = 'reflective'

    # fill the unit cell with the hex lattice
    hex_cell = openmc.Cell(region=-hex & +min_z & -max_z, fill=hex_lattice)

    model.geometry = openmc.Geometry([hex_cell])

    ### Settings ###
    settings = openmc.Settings()

    settings.particles = 1000
    settings.inactive = n_inactive
    settings.batches = settings.inactive + n_active
    settings.temperature['method'] = 'interpolation'
    settings.temperature['range'] = (294.0, 1500.0)
    settings.temperature['default'] = 450.0

    hexagon_half_flat = math.sqrt(3.0) / 2.0 * cell_edge_length
    lower_left = (-cell_edge_length, -hexagon_half_flat, reactor_bottom)
    upper_right = (cell_edge_length, hexagon_half_flat, reactor_top)
    source_dist = openmc.stats.Box(lower_left, upper_right, only_fissionable=True)
    source = openmc.IndependentSource(space=source_dist)
    settings.source = source

    model.settings = settings

    return model


def main():

    ap = ArgumentParser()
    ap.add_argument('-n', dest='n_axial', type=int, default=3,
                    help='Number of axial cell divisions (defaults to value in common_input.i)')
    ap.add_argument('-i', dest='n_inactive', type=int, default=5,
                    help='Number of inactive cycles')
    ap.add_argument('-a', dest='n_active', type=int, default=20,
                    help='Number of active cycles')

    args = ap.parse_args()

    model = unit_cell(args.n_axial, args.n_inactive, args.n_active)
    model.export_to_xml()

if __name__ == "__main__":
    main()
