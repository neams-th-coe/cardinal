#!/bin/python
import os
import math
import sys
import numpy as np
from argparse import ArgumentParser

# Create a mesh of the gap region between hexagonal ducts enclosed in a cylindrical
# barrel, with the load pad and restraint ring structural components. The sideset IDs are:

# 1: inlet
# 2: outlet
# 3: duct outer walls
# 4: vessel cylindrical wall
# 5: top/bottom of restraint rings on vessel
# 6: pad outer boundary

# In the axial direction, this mesh contains five main regions - a lower shield, an
# active region, a gas plenum, a standoff height, and a plenum. In this model, we
# have the duct extend up into the plenum. We then have two load pads, one 'pad1'
# above the top of the active region, and the second 'pad2' below the exit.

# The hydraulic diameter is computed based on the same geometry, but without the
# load pads or restraint rings.

# All of the settings for the mesh (geometry, refinements) are
# defined in the mesh_settings.py file. You should not need to
# edit anything in this script except for:

################################################################################

ap = ArgumentParser()
ap.add_argument('-g', '--generate', action='store_true',
                help='Whether to generate the mesh')

args = ap.parse_args()

script_dir = os.path.dirname(__file__)
sys.path.append(script_dir)
import mesh_settings as ms

flat_to_flat = ms.flat_to_flat
bundle_pitch = ms.bundle_pitch
corner_radius = ms.corner_radius
thickness = ms.thickness
vessel_inner_diameter = ms.vessel_inner_diameter
wire_pitch = ms.wire_pitch
n_bundles = ms.n_bundles

pad1 = ms.pad1
pad2 = ms.pad2
pad_height = ms.pad_height
pad_thickness = ms.pad_thickness
lower_shield_height = ms.lower_shield_height
active_height = ms.active_height
gas_plenum_height = ms.gas_plenum_height
standoff_height = ms.standoff_height
plenum_height = ms.plenum_height

e_per_side = ms.e_per_side
e_per_gap_span = ms.e_per_gap_span
e_per_load_pad_span = ms.e_per_load_pad_span
e_per_peripheral = ms.e_per_peripheral
e_per_bl = ms.e_per_bl
e_per_pad_bl = ms.e_per_pad_bl
e_per_gap_bl = ms.e_per_gap_bl
bl_height = ms.bl_height
bl_pad_height = ms.bl_pad_height
growth_factor = ms.growth_factor
corner_smoothing = ms.corner_smoothing

num_layers_per_dz = ms.num_layers_per_dz

###########################################################################

# If not specified by user, set a do-nothing default
if (len(corner_smoothing) == 0):
  for i in range(e_per_bl):
    corner_smoothing.append(1.0)

# block and sideset IDs for various regions in the mesh (arbitrary, but this
# lets us refer to them consistently across the scripts)
inlet = 1
outlet = 2
duct_outer = 55
vessel_inner = 66
vessel_inner_cut = 77
duct_pad_outer = 88
load_pad_id = 7
periph_id = 8
fluid_id = 6
gap_id = 51
duct_id = 50
pin_id = 14
garbage_id2 = 998
garbage_id3 = 997
garbage_id4 = 996
garbage_id5 = 995

# constants for hexagonal lattices
cos60 = 0.5
sin60 = math.sqrt(3.0) / 2.0
corner_shiftx = [cos60, -cos60, -1, -cos60, cos60, 1]
corner_shifty = [sin60, sin60, 0, -sin60, -sin60, 0]
edge_shiftx = [-1, -cos60, cos60, 1, cos60, -cos60]
edge_shifty = [0, -sin60, -sin60, 0, sin60, sin60]

def rings(elems):
  """Get the number of rings for a specified number of elements."""
  remaining = elems
  i = 0

  while (remaining > 0):
    i += 1
    remaining -= elements_in_ring(i)

  if (elems != elements(i)):
    raise ValueError("Number of elements not evenly divisible in a hexagonal lattice")

  return i

def elements(nrings):
  """Get the number of elements in a hexagonal lattice according to the number
     of rings of elements."""

  n = 0
  for i in range(nrings, 0, -1):
    n += elements_in_ring(i)

  return n

def elements_in_ring(ring):
  """Get the number of elements in specified ring of a hexagonal lattice."""
  if (ring == 1):
    return 1
  else:
    return 6 * (ring - 1)

def lattice_centers(nrings, pitch):
  """Get the center coordinates of the hex lattice elements in a specified number of rings
     with the given pitch."""

  n_pins = elements(nrings)
  xy = np.zeros((n_pins, 2))
  lattices = "'"

  c = 1
  for i in range(2, nrings + 1):
     n_total = elements_in_ring(i)
     n_corner = 6
     n_edge_per_side = int((n_total - n_corner) / 6)
     increment = n_edge_per_side + 1

     d = 0
     side = 0

     for j in range(n_total):
       if (d == increment):
         d = 0
         side += 1

       if (i == 2):
         side = j

       xy[c, 0] = corner_shiftx[side] * pitch * (i - 1)
       xy[c, 1] = corner_shifty[side] * pitch * (i - 1)

       # additional shift for the edge sides
       if (d != 0):
         xy[c, 0] += edge_shiftx[side] * pitch * d
         xy[c, 1] += edge_shifty[side] * pitch * d

       c += 1
       d += 1

  for i in range(n_pins):
    # rotate by 30 degrees
    rotated_x = xy[i, 0] * sin60 - xy[i, 1] * cos60
    rotated_y = xy[i, 0] * cos60 + xy[i, 1] * sin60
    lattices += str(rotated_x) + " " + str(rotated_y) + " 0.0;"

  return lattices[:-1] + "'"

# Get the 'pattern' needed for the core by assuming a simple hex grid of bundles
n_rings = rings(n_bundles)

def pat(nr):
  pattern = "'"
  first_row = int(elements_in_ring(nr) / 6) + 1
  last_row = first_row + n_rings - 1
  for i in range(n_rings):
    for j in range(first_row + i):
      pattern += " 0"
    pattern += ";"
  for i in range(n_rings - 1):
    for j in range(last_row - 1 - i):
      pattern += " 0"
    pattern += ";"
  return pattern + "'"

def layer_dx(first, growth, n):
  """Get the dx of a series of boundary layers"""
  l = []

  for i in range(n):
    l.append(math.pow(growth, i) * first)

  return l

def layer_thicknesses(first, growth, n):
  """Get the thickness of a series of boundary layers"""
  l = []

  previous_dx = 0.0
  for i in range(n):
    next_dx = math.pow(growth, i) * first
    layer = next_dx + previous_dx
    l.append(layer)

    previous_dx += next_dx

  return l

def bl_points(first, growth, n, start):
  """Get the positions of each boundary layer"""
  b = layer_thicknesses(first, growth, n)
  thickness = sum(b)
  for i in range(n):
    b[i] += start

  return b, thickness

# Get the origins of the bundles
bundle_origins = lattice_centers(n_rings, bundle_pitch)
bundle_pattern = pat(n_rings)

# Get the boundary layers on the duct surface
bl, bl_outer_dx = bl_points(bl_height, growth_factor, e_per_bl, flat_to_flat / 2.0 + thickness)

# Get the boundary layers on the pad surface
gap_bl, bl_gap_outer_dx = bl_points(bl_height, growth_factor, e_per_gap_bl, \
                          flat_to_flat / 2.0 + thickness + pad_thickness)

D = (bundle_pitch - flat_to_flat - 2.0 * thickness - 2.0 * pad_thickness) / 2.0

gap_dx = layer_dx(bl_height, growth_factor, e_per_gap_bl)
equi_space = (D - 2.0 * sum(gap_dx)) / e_per_gap_span
for i in range(e_per_gap_span):
  gap_bl.append(gap_bl[-1] + equi_space)
gap_dx.reverse()
for i in range(e_per_gap_bl - 1):
  gap_bl.append(gap_bl[-1] + gap_dx[i])

# Get the boundary layer thickness on bottom/top of the load pads
bl_pad_ascending, load_pad_bl_dx = bl_points(bl_pad_height, growth_factor, e_per_pad_bl, 0.0)

delete_duct = str(duct_id) + " " + str(garbage_id4) + " "

lower_bl = ""; upper_bl = ""; bl_layers = ""; s_swaps = ""; s_swaps_extra = ""; do_nothing = ""
for i in reversed(range(e_per_pad_bl)):
  lower_bl += " " + str(bl_pad_ascending[i])
  bl_layers += " 1"
  s_swaps_extra += " " + delete_duct + " " + str(periph_id) + " " + str(garbage_id3) + " " + \
    str(load_pad_id) + " " + str(garbage_id2) +"; "
  s_swaps += " " + delete_duct + "; "
  do_nothing += " " + delete_duct + "; "

for i in range(e_per_pad_bl):
  upper_bl += " " + str(bl_pad_ascending[i])

insertion = flat_to_flat / 2.0 + thickness + pad_thickness
bl.append(insertion)
bl.sort()

# height of the domain, which is split into 4 regions:
#   (1): below first load pad
#   (2): first load pad
#   (3): between load pads
#   (4): second load pad
#   (5): above second load pad

h = (lower_shield_height + active_height + gas_plenum_height + \
     standoff_height + plenum_height)
h1 = pad1 + (lower_shield_height + active_height) - load_pad_bl_dx
h2 = pad_height - 2.0 * load_pad_bl_dx
h4 = pad_height - 2.0 * load_pad_bl_dx
h5 = plenum_height + pad2 - load_pad_bl_dx
h3 = h - (h1 + h2 + h4 + h5) - 8.0 * load_pad_bl_dx

# number of layers in each major section of the axial height
nl1 = max(int(math.ceil(num_layers_per_dz * h1 / wire_pitch) + 1), 1)
nl2 = max(int(math.ceil(num_layers_per_dz * h2 / wire_pitch) + 1), 1)
nl3 = max(int(math.ceil(num_layers_per_dz * h3 / wire_pitch) + 1), 1)
nl4 = max(int(math.ceil(num_layers_per_dz * h4 / wire_pitch) + 1), 1)
nl5 = max(int(math.ceil(num_layers_per_dz * h5 / wire_pitch) + 1), 1)

gap_dx = (bundle_pitch - flat_to_flat - 2.0 * thickness) / 2.0
if (pad_thickness > gap_dx):
  print('Load pad thickness: ', pad_thickness)
  print('Duct-duct gap thickness: ', gap_dx)
  raise ValueError("Load pad thickness cannot exceed the duct-duct gap thickness!")

# Create arrays of various parameters for programmatic purposes
obl_radii = ""
for i in range(len(bl)):
  obl_radii += " " + str(bl[i])

oobl_radii = ""
for i in range(len(gap_bl)):
  oobl_radii += " " + str(gap_bl[i])

gi = ""
for i in range(e_per_gap_bl * 2 + e_per_gap_span - 1):
  gi += " " + str(gap_id)

dbi_lp = ""; di_lp = ""; di_lp_gap = ""
for i in range(e_per_bl):
  dbi_lp += " " + str(load_pad_id)
  di_lp += " " + str(1)
for i in range(e_per_gap_bl * 2 + e_per_gap_span - 1):
  di_lp_gap += " " + str(1)

dbi_lp += " " + str(load_pad_id)

# non-dimensionalize to the hydraulic diameter
outer_flat_to_flat = flat_to_flat + 2.0 * thickness
hex_area = math.sqrt(3.0) / 2.0 * outer_flat_to_flat**2
hex_perimeter = outer_flat_to_flat / math.sqrt(3.0) * 6

flow_area = (math.pi * vessel_inner_diameter**2 / 4.0 - n_bundles * hex_area)
wetted_perimeter = (math.pi * vessel_inner_diameter + n_bundles * hex_perimeter)
hydraulic_diameter = 4.0 * flow_area / wetted_perimeter

with open('mesh_info.i', 'w') as f:
  f.write(str.format('inv_Dh={0}\n', 1.0 / hydraulic_diameter))
  f.write(str.format('corner_radius={0}\n', corner_radius))
  f.write(str.format('outer_flat_to_flat={0}\n', outer_flat_to_flat))
  f.write(str.format('e_per_side={0}\n', e_per_side))
  f.write(str.format('e_per_gap_span={0}\n', e_per_gap_span))
  f.write(str.format('e_per_load_pad_span={0}\n',e_per_load_pad_span))
  f.write(str.format('e_per_peripheral={0}\n',e_per_peripheral))
  f.write(str.format('e_per_bl={0}\n', e_per_bl))
  f.write(str.format('e_per_pad_bl={0}\n',e_per_pad_bl))
  f.write(str.format('fluid_id={0}\n',fluid_id))
  f.write(str.format('load_pad_id={0}\n',load_pad_id))
  f.write(str.format('gap_id={0}\n',gap_id))
  f.write(str.format('duct_id={0}\n',duct_id))
  f.write(str.format('periph_id={0}\n',periph_id))
  f.write(str.format('pin_id={0}\n',pin_id))
  f.write(str.format('garbage_id2={0}\n',garbage_id2))
  f.write(str.format('garbage_id3={0}\n',garbage_id3))
  f.write(str.format('garbage_id4={0}\n',garbage_id4))
  f.write(str.format('garbage_id5={0}\n',garbage_id5))
  f.write(str.format('flat_to_flat={0}\n', flat_to_flat))
  f.write(str.format('wire_pitch={0}\n', wire_pitch))
  f.write(str.format('bundle_pitch={0}\n', bundle_pitch))
  f.write(str.format('thickness={0}\n', thickness))
  f.write(str.format('pad_thickness={0}\n', pad_thickness))
  f.write(str.format('vessel_inner_diameter={0}\n', vessel_inner_diameter))
  f.write(str.format('growth_factor={0}\n', growth_factor))
  f.write(str.format('bl_outer_dx={0}\n', bl_outer_dx))
  f.write(str.format('nl1={0}\n', nl1))
  f.write(str.format('nl2={0}\n', nl2))
  f.write(str.format('nl3={0}\n', nl3))
  f.write(str.format('nl4={0}\n', nl4))
  f.write(str.format('nl5={0}\n', nl5))
  f.write(str.format('h1={0}\n', h1))
  f.write(str.format('h2={0}\n', h2))
  f.write(str.format('h3={0}\n', h3))
  f.write(str.format('h4={0}\n', h4))
  f.write(str.format('h5={0}\n', h5))
  f.write(str.format('inlet={0}\n', inlet))
  f.write(str.format('outlet={0}\n', outlet))
  f.write(str.format('duct_outer={0}\n', duct_outer))
  f.write(str.format('duct_pad_outer={0}\n', duct_pad_outer))
  f.write(str.format('vessel_inner={0}\n', vessel_inner))
  f.write(str.format('vessel_inner_cut={0}\n', vessel_inner_cut))
  f.write("delete_duct='" + delete_duct + "'\n")
  f.write("bundle_pattern=" + str(bundle_pattern) + "\n")
  f.write("do_nothing='" + do_nothing + "'\n")
  f.write("obl_height='" + obl_radii + "'\n")
  f.write("di_lp='" + di_lp + "'\n")
  f.write("di_lp_gap='" + di_lp_gap + "'\n")
  f.write("dbi_lp='" + dbi_lp + "'\n")
  f.write("gi='" + gi + "'\n")
  f.write("oobl_height='" + oobl_radii + "'\n")
  f.write("bl_layers='" + bl_layers + "'\n")
  f.write("lower_bl='" + lower_bl + "'\n")
  f.write("upper_bl='" + upper_bl + "'\n")
  f.write("s_swaps='" + s_swaps + "'\n")
  f.write("s_swaps_extra='" + s_swaps_extra + "'\n")
  f.write("polygon_origins=" + str(bundle_origins) + "\n")
  f.write("corner_smoothing='" + corner_smoothing + "'\n")

if (args.generate):
  home = os.getenv('HOME')
  var = os.system(home + "/cardinal/cardinal-opt -i mesh_info.i bundle.i " + \
    " --mesh-only --n-threads=10")
  if (var):
    raise ValueError('Failed to run the bundle.i mesh script!')

  var = os.system(home + "/cardinal/cardinal-opt -i mesh_info.i convert.i " + \
    " --mesh-only --n-threads=10")
  if (var):
    raise ValueError('Failed to run the convert.i mesh script!')

  os.system("mv convert_in.e fluid.exo")

  print('Generated mesh with a characteristic length of: ', hydraulic_diameter)
  print('Inlet flow area:                                ', flow_area / (hydraulic_diameter**2))
  print('Height:                                         ', h / hydraulic_diameter)
