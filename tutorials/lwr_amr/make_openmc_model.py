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
# This is a modified version of the C5G7 reactor physics benchmark problem extension case as described in:                 #
# "Benchmark on Deterministic Transport Calculations Without Spatial Homogenisation: MOX Fuel Assembly 3-D Extension Case" #
# [NEA/NSC/DOC(2005)16]                                                                                                    #
# https://www.oecd-nea.org/upload/docs/application/pdf/2019-12/nsc-doc2005-16.pdf                                          #
#                                                                                                                          #
# The original C5G7 benchmark is defined with multi-group cross sections. To account for                                   #
# continuous energy spectral effects, we chose to use the material properties provided in:                                 #
# "Proposal for a Second Stage of the Benchmark on Power Distributions Within Assemblies"                                  #
# [NEA/NSC/DOC(96)2]                                                                                                       #
# https://www.oecd-nea.org/upload/docs/application/pdf/2020-01/nsc-doc96-02-rev2.pdf                                       #
#--------------------------------------------------------------------------------------------------------------------------#

import openmc
import numpy as np
import openmc.universe
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
## First fuel region: 4.3% MOX.
mox_4_3_comp = 1.0e24 * np.array([5.00e-5, 2.21e-2, 1.50e-5, 5.80e-4, 2.40e-4, 9.80e-5, 5.40e-5, 1.30e-5, 4.63e-2])
mox_4_3_frac = mox_4_3_comp / np.sum(mox_4_3_comp)
mox_4_3 = openmc.Material(name = '4.3% MOX Fuel', temperature = 293.15)
mox_4_3.add_nuclide('U235', mox_4_3_frac[0], percent_type = 'ao')
mox_4_3.add_nuclide('U238', mox_4_3_frac[1], percent_type = 'ao')
mox_4_3.add_nuclide('Pu238', mox_4_3_frac[2], percent_type = 'ao')
mox_4_3.add_nuclide('Pu239', mox_4_3_frac[3], percent_type = 'ao')
mox_4_3.add_nuclide('Pu240', mox_4_3_frac[4], percent_type = 'ao')
mox_4_3.add_nuclide('Pu241', mox_4_3_frac[5], percent_type = 'ao')
mox_4_3.add_nuclide('Pu242', mox_4_3_frac[6], percent_type = 'ao')
mox_4_3.add_nuclide('Am241', mox_4_3_frac[7], percent_type = 'ao')
mox_4_3.add_element('O', mox_4_3_frac[8], percent_type = 'ao')
mox_4_3.set_density('atom/cm3', np.sum(mox_4_3_comp))

## Second fuel region: 7.0% MOX.
mox_7_0_comp = 1.0e24 * np.array([5.00e-5, 2.21e-2, 2.40e-5, 9.30e-4, 3.90e-4, 1.52e-4, 8.40e-5, 2.00e-5, 4.63e-2])
mox_7_0_frac = mox_7_0_comp / np.sum(mox_7_0_comp)
mox_7_0 = openmc.Material(name = '7.0% MOX Fuel', temperature = 293.15)
mox_7_0.add_nuclide('U235', mox_7_0_frac[0], percent_type = 'ao')
mox_7_0.add_nuclide('U238', mox_7_0_frac[1], percent_type = 'ao')
mox_7_0.add_nuclide('Pu238', mox_7_0_frac[2], percent_type = 'ao')
mox_7_0.add_nuclide('Pu239', mox_7_0_frac[3], percent_type = 'ao')
mox_7_0.add_nuclide('Pu240', mox_7_0_frac[4], percent_type = 'ao')
mox_7_0.add_nuclide('Pu241', mox_7_0_frac[5], percent_type = 'ao')
mox_7_0.add_nuclide('Pu242', mox_7_0_frac[6], percent_type = 'ao')
mox_7_0.add_nuclide('Am241', mox_7_0_frac[7], percent_type = 'ao')
mox_7_0.add_element('O', mox_7_0_frac[8], percent_type = 'ao')
mox_7_0.set_density('atom/cm3', np.sum(mox_7_0_comp))

## Third fuel region: 8.7% MOX.
mox_8_7_comp = 1.0e24 * np.array([5.00e-5, 2.21e-2, 3.00e-5, 1.16e-3, 4.90e-4, 1.90e-4, 1.05e-4, 2.50e-5, 4.63e-2])
mox_8_7_frac = mox_8_7_comp / np.sum(mox_8_7_comp)
mox_8_7 = openmc.Material(name = '8.7% MOX Fuel', temperature = 293.15)
mox_8_7.add_nuclide('U235', mox_8_7_frac[0], percent_type = 'ao')
mox_8_7.add_nuclide('U238', mox_8_7_frac[1], percent_type = 'ao')
mox_8_7.add_nuclide('Pu238', mox_8_7_frac[2], percent_type = 'ao')
mox_8_7.add_nuclide('Pu239', mox_8_7_frac[3], percent_type = 'ao')
mox_8_7.add_nuclide('Pu240', mox_8_7_frac[4], percent_type = 'ao')
mox_8_7.add_nuclide('Pu241', mox_8_7_frac[5], percent_type = 'ao')
mox_8_7.add_nuclide('Pu242', mox_8_7_frac[6], percent_type = 'ao')
mox_8_7.add_nuclide('Am241', mox_8_7_frac[7], percent_type = 'ao')
mox_8_7.add_element('O', mox_8_7_frac[8], percent_type = 'ao')
mox_8_7.set_density('atom/cm3', np.sum(mox_8_7_comp))

## Fourth fuel region: UO2 at ~1% enriched.
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
## Fuels first
### Common primitives for defining the different fuel regions.
fuel_pin_or = openmc.ZCylinder(r = r_fuel)
fuel_gap_1_or = openmc.ZCylinder(r = r_fuel + t_f_c_gap)
fuel_zr_or = openmc.ZCylinder(r = r_fuel + t_f_c_gap + t_zr_clad)
fuel_bb = openmc.model.RectangularPrism(width = pitch, height = pitch)

### The entire 4.3% MOX pincell.
gap_1_cell_1 = openmc.Cell(name = '4.3% MOX Pin Gap 1')
gap_1_cell_1.region = +fuel_pin_or & -fuel_gap_1_or
zr_clad_cell_1 = openmc.Cell(name = '4.3% MOX Pin Zr Clad')
zr_clad_cell_1.region = +fuel_gap_1_or & -fuel_zr_or
zr_clad_cell_1.fill = zr
h2o_bb_cell_1 = openmc.Cell(name = '4.3% MOX Pin Water Bounding Box')
h2o_bb_cell_1.region = +fuel_zr_or & -fuel_bb
h2o_bb_cell_1.fill = h2o

mox_4_3_fuel_cell = openmc.Cell(name = '4.3% MOX Fuel Pin')
mox_4_3_fuel_cell.region = -fuel_pin_or
mox_4_3_fuel_cell.fill = mox_4_3
mox43_u = openmc.Universe(cells=[mox_4_3_fuel_cell, gap_1_cell_1, zr_clad_cell_1, h2o_bb_cell_1])

### The entire 7.0% MOX pincell.
gap_1_cell_2 = openmc.Cell(name = '7.0% MOX Pin Gap 1')
gap_1_cell_2.region = +fuel_pin_or & -fuel_gap_1_or
zr_clad_cell_2 = openmc.Cell(name = '7.0% MOX Pin Zr Clad')
zr_clad_cell_2.region = +fuel_gap_1_or & -fuel_zr_or
zr_clad_cell_2.fill = zr
h2o_bb_cell_2 = openmc.Cell(name = '7.0% MOX Pin Water Bounding Box')
h2o_bb_cell_2.region = +fuel_zr_or & -fuel_bb
h2o_bb_cell_2.fill = h2o

mox_7_0_fuel_cell = openmc.Cell(name = '7.0% MOX Fuel Pin')
mox_7_0_fuel_cell.region = -fuel_pin_or
mox_7_0_fuel_cell.fill = mox_7_0
mox70_u = openmc.Universe(cells=[mox_7_0_fuel_cell, gap_1_cell_2, zr_clad_cell_2, h2o_bb_cell_2])

### The entire 8.7% MOX pincell.
gap_1_cell_3 = openmc.Cell(name = '8.7% MOX Pin Gap 1')
gap_1_cell_3.region = +fuel_pin_or & -fuel_gap_1_or
zr_clad_cell_3 = openmc.Cell(name = '8.7% MOX Pin Zr Clad')
zr_clad_cell_3.region = +fuel_gap_1_or & -fuel_zr_or
zr_clad_cell_3.fill = zr
h2o_bb_cell_3 = openmc.Cell(name = '8.7% MOX Pin Water Bounding Box')
h2o_bb_cell_3.region = +fuel_zr_or & -fuel_bb
h2o_bb_cell_3.fill = h2o

mox_8_7_fuel_cell = openmc.Cell(name = '8.7% MOX Fuel Pin')
mox_8_7_fuel_cell.region = -fuel_pin_or
mox_8_7_fuel_cell.fill = mox_8_7
mox87_u = openmc.Universe(cells=[mox_8_7_fuel_cell, gap_1_cell_3, zr_clad_cell_3, h2o_bb_cell_3])

### The entire UO2 pincell.
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

## Guide tube, control rod, and fission chamber next.
### Common primitives for defining both.
tube_fill_or = openmc.ZCylinder(r = r_guide)
tube_clad_or = openmc.ZCylinder(r = r_guide + t_al_clad)

tube_clad_cell_1 = openmc.Cell(name = 'Guide Tube Cladding')
tube_clad_cell_1.region = +tube_fill_or & -tube_clad_or
tube_clad_cell_1.fill = al

tube_clad_cell_2 = openmc.Cell(name = 'Control Rod Cladding')
tube_clad_cell_2.region = +tube_fill_or & -tube_clad_or
tube_clad_cell_2.fill = al

tube_clad_cell_3 = openmc.Cell(name = 'Fission Chamber Cladding')
tube_clad_cell_3.region = +tube_fill_or & -tube_clad_or
tube_clad_cell_3.fill = al

guide_tube_h2o_bb_cell_1 = openmc.Cell(name = 'Guide Tube Water Bounding Box')
guide_tube_h2o_bb_cell_1.region = +tube_clad_or & -fuel_bb
guide_tube_h2o_bb_cell_1.fill = h2o

guide_tube_h2o_bb_cell_2 = openmc.Cell(name = 'Control Rod Water Bounding Box')
guide_tube_h2o_bb_cell_2.region = +tube_clad_or & -fuel_bb
guide_tube_h2o_bb_cell_2.fill = h2o

guide_tube_h2o_bb_cell_3 = openmc.Cell(name = 'Fission Chamber Water Bounding Box')
guide_tube_h2o_bb_cell_3.region = +tube_clad_or & -fuel_bb
guide_tube_h2o_bb_cell_3.fill = h2o

### The guide tube.
tube_fill_cell = openmc.Cell(name = 'Guide Tube Water')
tube_fill_cell.region = -tube_fill_or
tube_fill_cell.fill = h2o
tub_u = openmc.Universe(cells=[tube_fill_cell, tube_clad_cell_1, guide_tube_h2o_bb_cell_1])

### The control rod.
rod_fill_cell = openmc.Cell(name = 'Control Rod Meat')
rod_fill_cell.region = -tube_fill_or
rod_fill_cell.fill = bc4
rod_u = openmc.Universe(cells=[rod_fill_cell, tube_clad_cell_2, guide_tube_h2o_bb_cell_2])

### The fission chamber.
fission_chamber_cell = openmc.Cell(name = 'Fission Chamber')
fission_chamber_cell.region = -tube_fill_or
fission_chamber_cell.fill = fiss
fis_u = openmc.Universe(cells=[fission_chamber_cell, tube_clad_cell_3, guide_tube_h2o_bb_cell_3])

### An empty water block.
water_cell = openmc.Cell(name = 'Moderator')
water_cell.region = -fuel_bb
water_cell.fill = h2o
h2o_u = openmc.Universe(cells=[water_cell])

## The assemblies.
assembly_bb = openmc.model.RectangularPrism(width = 17.0 * pitch, height = 17.0 * pitch)

### UO2 fueled assembly.
uo2_assembly_cells = [
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 1
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 2
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 3
  [uo2_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, uo2_u], # 4
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 5
  [uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u], # 6
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 7
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 8
  [uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, fis_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u], # 9
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 10
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 11
  [uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u], # 12
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 13
  [uo2_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, uo2_u], # 14
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, tub_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 15
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u], # 16
  [uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u, uo2_u]  # 17
]# 1      2      3      4      5      6      7      8      9      10     11     12     13     14     15     16     17

uo2_assembly = openmc.RectLattice(name = 'UO2 Assembly')
uo2_assembly.pitch = (pitch, pitch)
uo2_assembly.lower_left = (-17.0 * pitch / 2.0, -17.0 * pitch / 2.0)
uo2_assembly.universes = uo2_assembly_cells
uo2_assembly_uni = openmc.Universe(cells = [openmc.Cell(name = 'UO2 Assembly Cell', region = -assembly_bb, fill = uo2_assembly)])

### UO2 fueled assembly with inserted control rods.
uo2_rodded_assembly_cells = [
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

uo2_rodded_assembly = openmc.RectLattice(name = 'Rodded UO2 Assembly')
uo2_rodded_assembly.pitch = (pitch, pitch)
uo2_rodded_assembly.lower_left = (-17.0 * pitch / 2.0, -17.0 * pitch / 2.0)
uo2_rodded_assembly.universes = uo2_rodded_assembly_cells
uo2_rodded_assembly_uni = openmc.Universe(cells = [openmc.Cell(name = 'Rodded UO2 Assembly Cell', region = -assembly_bb, fill = uo2_rodded_assembly)])

### MOX fueled assembly.
mox_assembly_cells = [
  [mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u], # 1
  [mox43_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox43_u], # 2
  [mox43_u, mox70_u, mox70_u, mox70_u, mox70_u, tub_u,   mox70_u, mox70_u, tub_u,   mox70_u, mox70_u, tub_u,   mox70_u, mox70_u, mox70_u, mox70_u, mox43_u], # 3
  [mox43_u, mox70_u, mox70_u, tub_u,   mox70_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox70_u, tub_u,   mox70_u, mox70_u, mox43_u], # 4
  [mox43_u, mox70_u, mox70_u, mox70_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox70_u, mox70_u, mox70_u, mox43_u], # 5
  [mox43_u, mox70_u, tub_u,   mox87_u, mox87_u, tub_u,   mox87_u, mox87_u, tub_u,   mox87_u, mox87_u, tub_u,   mox87_u, mox87_u, tub_u,   mox70_u, mox43_u], # 6
  [mox43_u, mox70_u, mox70_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox70_u, mox70_u, mox43_u], # 7
  [mox43_u, mox70_u, mox70_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox70_u, mox70_u, mox43_u], # 8
  [mox43_u, mox70_u, tub_u,   mox87_u, mox87_u, tub_u,   mox87_u, mox87_u, fis_u,   mox87_u, mox87_u, tub_u,   mox87_u, mox87_u, tub_u,   mox70_u, mox43_u], # 9
  [mox43_u, mox70_u, mox70_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox70_u, mox70_u, mox43_u], # 10
  [mox43_u, mox70_u, mox70_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox70_u, mox70_u, mox43_u], # 11
  [mox43_u, mox70_u, tub_u,   mox87_u, mox87_u, tub_u,   mox87_u, mox87_u, tub_u,   mox87_u, mox87_u, tub_u,   mox87_u, mox87_u, tub_u,   mox70_u, mox43_u], # 12
  [mox43_u, mox70_u, mox70_u, mox70_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox70_u, mox70_u, mox70_u, mox43_u], # 13
  [mox43_u, mox70_u, mox70_u, tub_u,   mox70_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox87_u, mox70_u, tub_u,   mox70_u, mox70_u, mox43_u], # 14
  [mox43_u, mox70_u, mox70_u, mox70_u, mox70_u, tub_u,   mox70_u, mox70_u, tub_u,   mox70_u, mox70_u, tub_u,   mox70_u, mox70_u, mox70_u, mox70_u, mox43_u], # 15
  [mox43_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox70_u, mox43_u], # 16
  [mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u, mox43_u]  # 17
]# 1        2        3        4        5        6        7        8        9        10       11       12       13       14       15       16       17

mox_assembly = openmc.RectLattice(name = 'MOX Assembly')
mox_assembly.pitch = (pitch, pitch)
mox_assembly.lower_left = (-17.0 * pitch / 2.0, -17.0 * pitch / 2.0)
mox_assembly.universes = mox_assembly_cells
mox_assembly_uni = openmc.Universe(cells = [openmc.Cell(name = 'MOX Assembly Cell', region = -assembly_bb, fill = mox_assembly)])

### The portion of the upper reflector containing control rods.
rodded_refl_cells = [
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 1
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 2
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 3
  [h2o_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, h2o_u], # 4
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 5
  [h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u], # 6
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 7
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 8
  [h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u], # 9
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 10
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 11
  [h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u], # 12
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 13
  [h2o_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, h2o_u], # 14
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, rod_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 15
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 16
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u]  # 17
]# 1      2      3      4      5      6      7      8      9      10     11     12     13     14     15     16     17

rodded_rel_assembly = openmc.RectLattice(name = 'Rodded Reflector Assembly')
rodded_rel_assembly.pitch = (pitch, pitch)
rodded_rel_assembly.lower_left = (-17.0 * pitch / 2.0, -17.0 * pitch / 2.0)
rodded_rel_assembly.universes = rodded_refl_cells
rodded_rel_assembly_uni = openmc.Universe(cells = [openmc.Cell(name = 'Rodded Reflector Assembly Cell', region = -assembly_bb, fill = rodded_rel_assembly)])

### The portion of the upper reflector containing guide tubes.
unrodded_refl_cells = [
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 1
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 2
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 3
  [h2o_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, h2o_u], # 4
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 5
  [h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u], # 6
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 7
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 8
  [h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u], # 9
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 10
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 11
  [h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u], # 12
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 13
  [h2o_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, h2o_u], # 14
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, tub_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 15
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u], # 16
  [h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u, h2o_u]  # 17
]# 1      2      3      4      5      6      7      8      9      10     11     12     13     14     15     16     17

unrodded_rel_assembly = openmc.RectLattice(name = 'Unrodded Reflector Assembly')
unrodded_rel_assembly.pitch = (pitch, pitch)
unrodded_rel_assembly.lower_left = (-17.0 * pitch / 2.0, -17.0 * pitch / 2.0)
unrodded_rel_assembly.universes = unrodded_refl_cells
unrodded_rel_assembly_uni = openmc.Universe(cells = [openmc.Cell(name = 'Unrodded Reflector Assembly Cell', region = -assembly_bb, fill = unrodded_rel_assembly)])

## The core region.
core_front = openmc.YPlane(y0 = 17.0 * pitch, boundary_type = 'reflective')
core_left = openmc.XPlane(x0 = -17.0 * pitch, boundary_type = 'reflective')
core_right = openmc.XPlane(x0 = 17.0 * pitch)
core_back = openmc.YPlane(y0 = -17.0 * pitch)
core_bb_xy = -core_front & +core_back & +core_left & -core_right

core_cells = [
  [uo2_rodded_assembly_uni, mox_assembly_uni],
  [mox_assembly_uni, uo2_assembly_uni]
]
core_assembly = openmc.RectLattice(name = 'Core Assembly')
core_assembly.pitch = (17.0 * pitch, 17.0 * pitch)
core_assembly.lower_left = (-17.0 * pitch, -17.0 * pitch)
core_assembly.universes = core_cells

core_z_planes = []
for z in np.linspace(0.0, core_height, core_axial_slices + 1):
  core_z_planes.append(openmc.ZPlane(z0 = z))
core_z_planes[0].boundary_type = 'reflective'

core_assembly_cells = []
for i in range(core_axial_slices):
  core_assembly_cells.append(openmc.Cell(name = 'Core Assembly Cell ' + str(i), region = core_bb_xy & +core_z_planes[i] & -core_z_planes[i + 1], fill = core_assembly))

## The reflector region.
refl_right = openmc.XPlane(x0 = 17.0 * pitch + reflector_t, boundary_type = 'vacuum')
refl_back = openmc.YPlane(y0 = -17.0 * pitch - reflector_t, boundary_type = 'vacuum')
refl_top = openmc.ZPlane(z0 = core_height + reflector_t, boundary_type = 'vacuum')

### The portion of the reflector above the core (penetrated by control rods and guide tubes).
upper_reflector_cells = [
  [rodded_rel_assembly_uni,   unrodded_rel_assembly_uni],
  [unrodded_rel_assembly_uni, unrodded_rel_assembly_uni]
]
upper_refl_assembly = openmc.RectLattice(name = 'Upper Reflector Assembly')
upper_refl_assembly.pitch = (17.0 * pitch, 17.0 * pitch)
upper_refl_assembly.lower_left = (-17.0 * pitch, -17.0 * pitch)
upper_refl_assembly.universes = upper_reflector_cells
upper_refl_cell = openmc.Cell(name = 'Upper Reflector Cell', region = +core_z_planes[-1] & -refl_top & core_bb_xy, fill = upper_refl_assembly)

### The remainder of the reflector.
refl_region = -core_front & +refl_back & +core_left & -refl_right & -refl_top & +core_z_planes[0] & ~(core_bb_xy & +core_z_planes[0] & -refl_top)
refl_cell = openmc.Cell(name = 'Water Reflector')
refl_cell.region = refl_region
refl_cell.fill = h2o

## The entire geometry.
all_cells = [refl_cell, upper_refl_cell]
for c in core_assembly_cells:
  all_cells.append(c)
model_uni = openmc.Universe(cells = all_cells)
#--------------------------------------------------------------------------------------------------------------------------#

#--------------------------------------------------------------------------------------------------------------------------#
# Setup the model.
c5g7_model = openmc.Model(geometry = openmc.Geometry(model_uni), materials = openmc.Materials([mox_4_3, mox_7_0, mox_8_7, uo2, h2o, bc4, fiss, zr, al]))

## The simulation settings.
c5g7_model.settings.source = [openmc.IndependentSource(space = openmc.stats.Box(lower_left = (-17.0 * pitch, -17.0 * pitch, 0.0), upper_right = (17.0 * pitch, 17.0 * pitch, 192.78)))]
c5g7_model.settings.batches = 100
c5g7_model.settings.generations_per_batch = 10
c5g7_model.settings.inactive = 10
c5g7_model.settings.particles = 1000

c5g7_model.export_to_model_xml()
#--------------------------------------------------------------------------------------------------------------------------#
