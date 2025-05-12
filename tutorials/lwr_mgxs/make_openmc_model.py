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

#--------------------------------------------------------------------------------------------------------------------------#
# This is based on the C5G7 reactor physics benchmark problem extension case as described in:                              #
# "Benchmark on Deterministic Transport Calculations Without Spatial Homogenisation: MOX Fuel Assembly 3-D Extension Case" #
# [NEA/NSC/DOC(2005)16]                                                                                                    #
# https://www.oecd-nea.org/upload/docs/application/pdf/2019-12/nsc-doc2005-16.pdf                                          #                                                                                                                     #
#                                                                                                                          #
# The original C5G7 benchmark is defined with multi-group cross sections. To account for                                   #
# continuous energy spectral effects, we chose to use the material properties provided in:                                 #
# "Proposal for a Second Stage of the Benchmark on Power Distributions Within Assemblies"                                  #
# [NEA/NSC/DOC(96)2]                                                                                                       #
# https://www.oecd-nea.org/upload/docs/application/pdf/2020-01/nsc-doc96-02-rev2.pdf                                       #
#--------------------------------------------------------------------------------------------------------------------------#

import openmc
import numpy as np
from argparse import ArgumentParser

ap = ArgumentParser()
ap.add_argument('-n', dest='n_axial', type=int, default=1,
                help='Number of axial core divisions')
args = ap.parse_args()

#--------------------------------------------------------------------------------------------------------------------------#
# Some geometric properties that can be modified to change the model.
## The radius of a fuel pin (same for all pin types).
r_fuel = 0.4095

## The thickness of the fuel-clad gap.
t_f_c_gap = 0.0085

## The thickness of the Zr fuel pin cladding.
t_zr_clad = 0.057

## The radius of the control rod guide tubes and the fission chambers.
r_guide = 0.3400

## The thickness of the guide tube / fission chamber Al cladding.
t_al_clad = 0.2

## The pitch of a single lattice element.
pitch = 1.26

## The height of the fuel assemblies from the axial midplane.
core_height = 192.78

## The thickness of the water reflector around the fuel assemblies, both axial and radial.
reflector_t = 21.42

# Some discretization parameters.
core_axial_slices = args.n_axial
#--------------------------------------------------------------------------------------------------------------------------#

#--------------------------------------------------------------------------------------------------------------------------#
# Material definitions.

## Fuel region: UO2 at ~1% enriched.
uo2_comp = 1.0e24 * np.array([8.65e-4, 2.225e-2, 4.622e-2])
uo2_frac = uo2_comp / np.sum(uo2_comp)
uo2 = openmc.Material(name = 'UO2 Fuel', temperature = 293.15)
uo2.add_nuclide('U235', uo2_frac[0], percent_type = 'ao')
uo2.add_nuclide('U238', uo2_frac[1], percent_type = 'ao')
uo2.add_element('O', uo2_frac[2], percent_type = 'ao')
uo2.set_density('atom/cm3', np.sum(uo2_comp))

## Control rod meat: assumed to be B-10 carbide (B4C).
bc4 = openmc.Material(name = 'Control Rod Meat', temperature = 293.15)
bc4.add_element('B', 4.0, percent_type = 'ao')
bc4.add_element('C', 1.0, percent_type = 'ao')
bc4.set_density('atom/cm3', 2.75e23)

## Moderator and coolant, boronated water.
h2o_comp = 1.0e24 * np.array([3.35e-2, 2.78e-5])
h2o_frac = h2o_comp / np.sum(h2o_comp)
h2o = openmc.Material(name = 'H2O Moderator', temperature = 293.15)
h2o.add_element('H', 2.0 * h2o_frac[0], percent_type = 'ao')
h2o.add_element('O', h2o_frac[0], percent_type = 'ao')
h2o.add_element('B', h2o_frac[1], percent_type = 'ao')
h2o.set_density('atom/cm3', np.sum(h2o_comp))
h2o.add_s_alpha_beta('c_H_in_H2O')

## Fission chamber.
fiss_comp = 1.0e24 * np.array([3.35e-2, 2.78e-5, 1.0e-8])
fiss_frac = fiss_comp / np.sum(fiss_comp)
fiss = openmc.Material(name = 'Fission Chamber', temperature = 293.15)
fiss.add_element('H', 2.0 * fiss_frac[0], percent_type = 'ao')
fiss.add_element('O', fiss_frac[0], percent_type = 'ao')
fiss.add_element('B', fiss_frac[1], percent_type = 'ao')
fiss.add_nuclide('U235', fiss_frac[2], percent_type = 'ao')
fiss.set_density('atom/cm3', np.sum(fiss_comp))
fiss.add_s_alpha_beta('c_H_in_H2O')

## Zr clad.
zr = openmc.Material(name = 'Zr Cladding', temperature = 293.15)
zr.add_element('Zr', 1.0, percent_type = 'ao')
zr.set_density('atom/cm3', 1.0e24 * 4.30e-2)

## Al clad.
al = openmc.Material(name = 'Al Cladding', temperature = 293.15)
al.add_element('Al', 1.0, percent_type = 'ao')
al.set_density('atom/cm3', 1.0e24 * 6.0e-2)
#--------------------------------------------------------------------------------------------------------------------------#

#--------------------------------------------------------------------------------------------------------------------------#
# Geometry definitions.
## Fuel first
### The entire UO2 pincell.
fuel_pin_or = openmc.ZCylinder(r = r_fuel)
fuel_gap_1_or = openmc.ZCylinder(r = r_fuel + t_f_c_gap)
fuel_zr_or = openmc.ZCylinder(r = r_fuel + t_f_c_gap + t_zr_clad)
fuel_bb = openmc.model.RectangularPrism(width = pitch, height = pitch)

gap_1_cell_4 = openmc.Cell(name = 'UO2 Pin Gap 1')
gap_1_cell_4.region = +fuel_pin_or & -fuel_gap_1_or
zr_clad_cell_4 = openmc.Cell(name = 'UO2 Pin Zr Clad')
zr_clad_cell_4.region = +fuel_gap_1_or & -fuel_zr_or
zr_clad_cell_4.fill = zr
h2o_bb_cell_4 = openmc.Cell(name = 'UO2 Pin Water Bounding Box')
h2o_bb_cell_4.region = +fuel_zr_or & -fuel_bb
h2o_bb_cell_4.fill = h2o

uo2_fuel_cell = openmc.Cell(name = 'UO2 Fuel Pin')
uo2_fuel_cell.region = -fuel_pin_or
uo2_fuel_cell.fill = uo2
uo2_u = openmc.Universe(cells=[uo2_fuel_cell, gap_1_cell_4, zr_clad_cell_4, h2o_bb_cell_4])

## Guide tube and fission chamber next.
### Common primitives for defining both.
tube_fill_or = openmc.ZCylinder(r = r_guide)
tube_clad_or = openmc.ZCylinder(r = r_guide + t_al_clad)

tube_clad_cell_1 = openmc.Cell(name = 'Control Rod Cladding')
tube_clad_cell_1.region = +tube_fill_or & -tube_clad_or
tube_clad_cell_1.fill = al

tube_clad_cell_2 = openmc.Cell(name = 'Fission Chamber Cladding')
tube_clad_cell_2.region = +tube_fill_or & -tube_clad_or
tube_clad_cell_2.fill = al

guide_tube_h2o_bb_cell_1 = openmc.Cell(name = 'Guide Tube Water Bounding Box')
guide_tube_h2o_bb_cell_1.region = +tube_clad_or & -fuel_bb
guide_tube_h2o_bb_cell_1.fill = h2o

guide_tube_h2o_bb_cell_2 = openmc.Cell(name = 'Fission Chamber Water Bounding Box')
guide_tube_h2o_bb_cell_2.region = +tube_clad_or & -fuel_bb
guide_tube_h2o_bb_cell_2.fill = h2o

### The guide tube.
rod_fill_cell = openmc.Cell(name = 'Control Rod Meat')
rod_fill_cell.region = -tube_fill_or
rod_fill_cell.fill = bc4
rod_u = openmc.Universe(cells=[rod_fill_cell, tube_clad_cell_1, guide_tube_h2o_bb_cell_1])

### The fission chamber.
fission_chamber_cell = openmc.Cell(name = 'Fission Chamber')
fission_chamber_cell.region = -tube_fill_or
fission_chamber_cell.fill = fiss
fis_u = openmc.Universe(cells=[fission_chamber_cell, tube_clad_cell_2, guide_tube_h2o_bb_cell_2])

### An empty water cell to build the upper reflector.
water_cell = openmc.Cell(name = 'Reflector Water')
water_cell.region = -fuel_bb
water_cell.fill = h2o
wat_u = openmc.Universe(cells=[water_cell])

## The assembly.
assembly_bb = openmc.model.RectangularPrism(origin = (0.0, 0.0), width = 16.9 * pitch, height = 16.9 * pitch, boundary_type = 'reflective')

### UO2 fueled assembly.
uo2_assembly_cells = [
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 1
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 2
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 3
  [uo2_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, uo2_u], # 4
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 5
  [uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u], # 6
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 7
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 8
  [uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, fis_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u], # 9
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 10
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 11
  [uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u], # 12
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 13
  [uo2_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, uo2_u], # 14
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, rod_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 15
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 16
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u]  # 17
]# 1      2      3      4      5      6      7      8      9      10     11     12     13     14     15     16     17

uo2_assembly = openmc.RectLattice(name = 'UO2 Assembly')
uo2_assembly.pitch = (pitch, pitch, core_height / core_axial_slices)
uo2_assembly.lower_left = (-17.0 * pitch / 2.0, -17.0 * pitch / 2.0, 0.0)
uo2_assembly.universes = [uo2_assembly_cells for k in range(core_axial_slices)]

core_z_plane_center = openmc.ZPlane(z0 = 0.0, boundary_type = 'reflective')
core_z_plane_top    = openmc.ZPlane(z0 = core_height)

all_cells = []
all_cells.append(openmc.Cell(name = 'Fueled Assembly', fill = uo2_assembly, region = +core_z_plane_center & -core_z_plane_top & -assembly_bb))

### The upper reflector.
ref_assembly_cells = [
  [wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 1
  [wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 2
  [wat_u, wat_u, wat_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 3
  [wat_u, wat_u, wat_u, rod_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, rod_u, wat_u, wat_u, wat_u], # 4
  [wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 5
  [wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u], # 6
  [wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 7
  [wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 8
  [wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, fis_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u], # 9
  [wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 10
  [wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 11
  [wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u], # 12
  [wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 13
  [wat_u, wat_u, wat_u, rod_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, rod_u, wat_u, wat_u, wat_u], # 14
  [wat_u, wat_u, wat_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, rod_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 15
  [wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u], # 16
  [wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u, wat_u]  # 17
]# 1      2      3      4      5      6      7      8      9      10     11     12     13     14     15     16     17

ref_assembly = openmc.RectLattice(name = 'Reflector Assembly')
ref_assembly.pitch = (pitch, pitch)
ref_assembly.lower_left = (-17.0 * pitch / 2.0, -17.0 * pitch / 2.0)
ref_assembly.universes = ref_assembly_cells

refl_top = openmc.ZPlane(z0 = core_height + reflector_t, boundary_type = 'vacuum')
all_cells.append(openmc.Cell(name='Axial Reflector Cell', fill = ref_assembly, region=-assembly_bb & -refl_top & +core_z_plane_top))

#--------------------------------------------------------------------------------------------------------------------------#
# Setup the model.
c5g7_model = openmc.Model(geometry = openmc.Geometry(openmc.Universe(cells = all_cells)), materials = openmc.Materials([uo2, h2o, fiss, zr, al, bc4]))

## The simulation settings.
c5g7_model.settings.source = [openmc.IndependentSource(space = openmc.stats.Box(lower_left = (-17.0 * pitch / 2.0, -17.0 * pitch / 2.0, 0.0),
                                                                                upper_right = (17.0 * pitch / 2.0, 17.0 * pitch / 2.0, 192.78)))]
c5g7_model.settings.batches = 100
c5g7_model.settings.generations_per_batch = 10
c5g7_model.settings.inactive = 10
c5g7_model.settings.particles = 1000

c5g7_model.export_to_model_xml()
#--------------------------------------------------------------------------------------------------------------------------#
