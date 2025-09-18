import openmc
import openmc.lib
import numpy as np
from matplotlib import pyplot as plt
import math

import os
import sys
script_dir = os.path.dirname(__file__)
sys.path.append(script_dir)
import common as specs

# This is a simplified OpenMC model of a CEFR fuel subassembly; for this multiphysics
# tutorial, we focus only on the active region, with a height of 45 cm
# (just out of simplicity and to make the training more tractable for the time allotted).

model = openmc.Model()

# --- Create materials

# UO2 for fuel
UO2 = openmc.Material(name="UO2")
UO2.set_density('sum')
UO2.add_nuclide('U235',1.49981E-02,'ao')
UO2.add_nuclide('U238',8.26381E-03,'ao')
UO2.add_nuclide('O16', 4.69512E-02,'ao')

# 15-15Ti stainless steel for the cladding
SS = openmc.Material(name="15-15Ti")
SS.set_density('atom/b-cm',density=8.49843E-02)
SS.add_nuclide('Fe54' ,3.18736E-03 )
SS.add_nuclide('Fe56' ,5.00347E-02 )
SS.add_nuclide('Fe57' ,1.15552E-03 )
SS.add_nuclide('Fe58' ,1.53779E-04 )
SS.add_nuclide('Cr50' ,6.43727E-04 )
SS.add_nuclide('Cr52' ,1.24136E-02 )
SS.add_nuclide('Cr53' ,1.40761E-03 )
SS.add_nuclide('Cr54' ,3.50383E-04 )
SS.add_nuclide('Ni58' ,8.11014E-03 )
SS.add_nuclide('Ni60' ,3.12399E-03 )
SS.add_nuclide('Ni61' ,1.35798E-04 )
SS.add_nuclide('Ni62' ,4.32997E-04 )
SS.add_nuclide('Ni64' ,1.10257E-04 )
SS.add_nuclide('Mo92' ,1.57916E-04 )
SS.add_nuclide('Mo94' ,9.94445E-05 )
SS.add_nuclide('Mo95' ,1.72153E-04 )
SS.add_nuclide('Mo96' ,1.81174E-04 )
SS.add_nuclide('Mo97' ,1.04335E-04 )
SS.add_nuclide('Mo98' ,2.65077E-04 )
SS.add_nuclide('Mo100',1.06726E-04 )
SS.add_nuclide('Mn55' ,1.29433E-03 )
SS.add_nuclide('C0'   ,2.37026E-04 )
SS.add_nuclide('Ti46' ,2.85966E-05 )
SS.add_nuclide('Ti47' ,2.57889E-05 )
SS.add_nuclide('Ti48' ,2.55532E-04 )
SS.add_nuclide('Ti49' ,1.87524E-05 )
SS.add_nuclide('Ti50' ,1.79552E-05 )
SS.add_nuclide('Si28' ,7.00451E-04 )
SS.add_nuclide('Si29' ,3.55834E-05 )
SS.add_nuclide('Si30' ,2.34843E-05 )

# Ti316 stainless steel for the duct
Ti316 = openmc.Material(name="Ti36")
Ti316.set_density('atom/b-cm',density=8.54309E-02)
Ti316.add_nuclide('Fe54' ,3.23855E-03)
Ti316.add_nuclide('Fe56' ,5.08384E-02)
Ti316.add_nuclide('Fe57' ,1.17408E-03)
Ti316.add_nuclide('Fe58' ,1.56248E-04)
Ti316.add_nuclide('Cr50' ,6.74283E-04)
Ti316.add_nuclide('Cr52' ,1.30029E-02)
Ti316.add_nuclide('Cr53' ,1.47442E-03)
Ti316.add_nuclide('Cr54' ,3.67015E-04)
Ti316.add_nuclide('Ni58' ,6.88162E-03)
Ti316.add_nuclide('Ni60' ,2.65077E-03)
Ti316.add_nuclide('Ni61' ,1.15228E-04)
Ti316.add_nuclide('Ni62' ,3.67407E-04)
Ti316.add_nuclide('Ni64' ,9.35550E-05)
Ti316.add_nuclide('Mo92' ,1.79675E-04)
Ti316.add_nuclide('Mo94' ,1.13147E-04)
Ti316.add_nuclide('Mo95' ,1.95874E-04)
Ti316.add_nuclide('Mo96' ,2.06138E-04)
Ti316.add_nuclide('Mo97' ,1.18712E-04)
Ti316.add_nuclide('Mo98' ,3.01602E-04)
Ti316.add_nuclide('Mo100',1.21432E-04)
Ti316.add_nuclide('Mn55' ,1.51194E-03)
Ti316.add_nuclide('C0'   ,2.37324E-04)
Ti316.add_nuclide('Ti46' ,3.27229E-05)
Ti316.add_nuclide('Ti47' ,2.95101E-05)
Ti316.add_nuclide('Ti48' ,2.92403E-04)
Ti316.add_nuclide('Ti49' ,2.14583E-05)
Ti316.add_nuclide('Ti50' ,2.05460E-05)
Ti316.add_nuclide('Si28' ,9.35106E-04)
Ti316.add_nuclide('Si29' ,4.75041E-05)
Ti316.add_nuclide('Si30' ,3.13517E-05)

# sodium for the coolant
sodium = openmc.Material(name="sodium")
sodium.set_density('sum')
sodium.add_nuclide('Na23',2.33599E-02)

# helium for the central hole in the fuel pin
He = openmc.Material(name="helium")
He.set_density('sum')
He.add_nuclide('He4',1.00000E-11)

# register the materials with the model object
model.materials = openmc.Materials([UO2, SS, Ti316, He, sodium])

# --- Define the geometry

# Define the surfaces needed for a fuel pin
helium_hole_surface = openmc.ZCylinder(x0=0, y0=0, r=specs.hole_diameter/2.0)
fuel_surface        = openmc.ZCylinder(x0=0, y0=0, r=specs.pellet_diameter/2.0)
clad_inner_surface  = openmc.ZCylinder(x0=0, y0=0, r=specs.inner_clad_diameter/2.0)
clad_outer_surface  = openmc.ZCylinder(x0=0, y0=0, r=specs.outer_clad_diameter/2.0)

# Define the surfaces needed for the various hexagons enclosing the assembly
hex_WR_IN    = openmc.model.HexagonalPrism(orientation='x', origin=(0.0, 0.0), edge_length=specs.duct_inner_flat_to_flat/math.sqrt(3))
hex_WR_OU    = openmc.model.HexagonalPrism(orientation='x', origin=(0.0, 0.0), edge_length=specs.duct_outer_flat_to_flat/math.sqrt(3.))
hex_SA_PITCH = openmc.model.HexagonalPrism(orientation='x', origin=(0.0, 0.0), edge_length=specs.assembly_pitch/math.sqrt(3.), boundary_type='reflective')

# define axial surfaces which will bound the active fissile region
lower = openmc.ZPlane(z0=0.0, boundary_type='vacuum')
upper = openmc.ZPlane(z0=specs.height, boundary_type='vacuum')

# create the cells in a fuel pin
helium_hole    = openmc.Cell(fill=He,     region=-helium_hole_surface)
fuel_annulus   = openmc.Cell(fill=UO2,    region=+helium_hole_surface & -fuel_surface)
helium_gap     = openmc.Cell(fill=He,     region=+fuel_surface & -clad_inner_surface)
ss_clad        = openmc.Cell(fill=SS,     region=+clad_inner_surface & -clad_outer_surface)
sodium_annulus = openmc.Cell(fill=sodium, region=+clad_outer_surface)

fuel_pin_universe = openmc.Universe(cells=[helium_hole, fuel_annulus, helium_gap, ss_clad, sodium_annulus])

# pure sodium universe
sodium_universe = openmc.Universe(cells=[openmc.Cell(fill=sodium)])

# create a lattice for a fuel assembly, consisting of 61 fuel pins
axial_pitch = specs.height / specs.n_layers
fuel_pin_lattice = openmc.HexLattice()
fuel_pin_lattice.orientation = 'x'
fuel_pin_lattice.center = [0., 0., specs.height/2]
fuel_pin_lattice.pitch  = [specs.pin_pitch, axial_pitch]

each_layer = [[sodium_universe]*30, [fuel_pin_universe]*24, [fuel_pin_universe]*18, [fuel_pin_universe]*12, [fuel_pin_universe]*6, [fuel_pin_universe]]
axial = []
for i in range(specs.n_layers):
  axial.append(each_layer)

fuel_pin_lattice.universes = axial

# put that fuel pin lattice inside a hexagon boundary, then add additional hexagons
# to represent the duct. For each, place inside the axial extents
layer = +lower & -upper
fuel_hex_cell = openmc.Cell(region=-hex_WR_IN & layer, fill=fuel_pin_lattice)
duct_cell = openmc.Cell(region=-hex_WR_OU & +hex_WR_IN & layer, fill=Ti316)
sodium_cell = openmc.Cell(region=+hex_WR_OU & -hex_SA_PITCH & layer, fill=sodium)

root = openmc.Universe(cells=[fuel_hex_cell, duct_cell, sodium_cell])
model.geometry = openmc.Geometry(root)

###############################################################################
#                   Exporting to OpenMC settings.xml file
###############################################################################

# Instantiate a Settings object, set all runtime parameters, and export to XML. Note
# that the choices for batches and particles are VERY low, and are only selected as
# these choices to get a fast-running model
model.settings.batches   = 120
model.settings.inactive  = 20
model.settings.particles = 1000

model.settings.ptables   = True
model.settings.temperature['method']='interpolation'
model.settings.temperature['range'] = (300.0, 3000.0)
model.settings.temperature['default'] = 0.5 * (specs.inlet_temperature + specs.outlet_temperature)

# Create an initial uniform spatial source distribution over fissionable zones
l = specs.assembly_pitch
bounds = [-l, -l, 0.0, l, l, specs.height]
uniform_dist = openmc.stats.Box(bounds[:3], bounds[3:])
model.settings.source = openmc.source.IndependentSource(space=uniform_dist, constraints={'fissionable' : True})

# Create some plots for visualization
xy          = openmc.Plot()
xy.filename = 'xy'
xy.width    = (8, 8)
xy.basis    = 'xy'
xy.origin   = (0.0, 0.0, 0.5*specs.height)
xy.pixels   = (2000, 2000)
xy.color_by = 'material'

xz          = openmc.Plot()
xz.filename = 'xz'
xz.width    = (l, specs.height*1.5)
xz.basis    = 'xz'
xz.origin   = (0.0, 0.0, 0.5*specs.height)
xz.pixels   = (2000, 2000)
xz.color_by = 'material'

xy_cell          = openmc.Plot()
xy_cell.filename = 'xy_cell'
xy_cell.width    = (8, 8)
xy_cell.basis    = 'xy'
xy_cell.origin   = (0.0, 0.0, 0.5*specs.height)
xy_cell.pixels   = (2000, 2000)
xy_cell.color_by = 'cell'

xz_cell          = openmc.Plot()
xz_cell.filename = 'xz_cell'
xz_cell.width    = (l, specs.height*1.5)
xz_cell.basis    = 'xz'
xz_cell.origin   = (0.0, 0.0, 0.5*specs.height)
xz_cell.pixels   = (2000, 2000)
xz_cell.color_by = 'cell'

model.plots = openmc.Plots([xy, xz, xy_cell, xz_cell])

model.export_to_model_xml()

