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

ap = ArgumentParser()
ap.add_argument('-g', '--generate', action='store_true',
                help='Whether to generate the mesh')

args = ap.parse_args()

flat_to_flat = 14.922e-2      # flat-to-flat inside the duct
bundle_pitch = 16.142e-2      # bundle pitch
corner_radius = 1e-2          # radius of curvature of duct corners
thickness = 0.394e-2          # duct thickness
vessel_inner_diameter = 0.75  # vessel inner diameter
wire_pitch = 20.32e-2         # wire axial pitch
n_bundles = 7                 # number of fuel bundles

pad1 = 10.16e-2               # starting height (above top of active region)
pad2 = 10.16e-2               # top height (below exit)
pad_height = 10.16e-2         # height of a load pad
pad_thickness = 1.5e-3        # pad thickness
lower_shield_height = 0.1016  # height of lower shield
active_height = 0.2065        # active height
gas_plenum_height = 0.4225    # gas plenum height
standoff_height = 0.3         # standoff heigt
plenum_height = 0.1016        # plenum height

e_per_side = 2                # elements per side of the duct
e_per_gap_span = 1            # elements across non-boundary layer part of gap
e_per_load_pad_span = 1       # elements across the non-boundary layer part of load pad
e_per_peripheral = 1          # elements across the peripheral region
e_per_interior_bl = 1         # elements in each boundary layer in duct interior (high Re)
e_per_duct_span = 1           # elements across the duct region
bl_interior_height = 0.0001   # height of first boundary layer in duct interior (high Re)
e_per_exterior_bl = 1         # elements in each bounddary layer in duct exterior (low Re)
bl_exterior_height = 0.0001   # height of first boundary layer in duct exterior (low Re)
e_per_pad_bl = 1              # elements in the boundary layers on the bottom/top faces of the load pads
bl_pad_height = 0.0001        # height of first boundary layer on bottom/top faces of load pads
growth_factor = 1.8           # boundary layer growth factor

num_layers_per_dz = 1.0       # layers per axial pitch

# Smoothing factors to apply to the corner movement; must match the length
# of the e_per_bl
corner_smoothing = [1.0]

###########################################################################

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

cs = ""
for i in range(len(corner_smoothing)):
  cs += " " + str(corner_smoothing[i])

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

# Get the origins of the bundles
bundle_origins = lattice_centers(n_rings, bundle_pitch)
bundle_pattern = pat(n_rings)

# Get the "radii" of the "ducts" which we use to create boundary layers on
# the inner surfaces of a duct. After flipping, these are listed in asscending order (smallest to largest),
# and do not include the actual inner boundary of the duct (which is instead created by
# other syntax in the mesh generator)
duct_radii = []
previous_r = flat_to_flat / 2.0
for i in range(e_per_interior_bl):
  duct_radii.append(str(previous_r - math.pow(growth_factor, i) * bl_interior_height))
  previous_r -=  math.pow(growth_factor, i - 1) * bl_interior_height
duct_radii.reverse()

# Get the "radii" of the "ducts" which we use to create boundary layers on
# the outside of the duct, as well as the load pads inside the core region.
# These are listed in ascending order (smallest to largest), and do not include the
# actual outer boundary of the bundle unit cell.
bl = []
previous_dx = 0.0
starting_pt = flat_to_flat / 2.0 + thickness
bl_outer_dx = 0.0
for i in range(e_per_exterior_bl):
  bl_outer_dx += math.pow(growth_factor, i) * bl_exterior_height
  layer = math.pow(growth_factor, i) * bl_exterior_height + starting_pt + previous_dx
  bl.append(layer)
  previous_dx += math.pow(growth_factor, i) * bl_exterior_height

# Get the boundary layer thickness on bottom/top of the load pads
bl_pad_ascending = []
load_pad_bl_dx = 0.0
for i in range(e_per_pad_bl):
  bl_pad_ascending.append(math.pow(growth_factor, i) * bl_pad_height)
  load_pad_bl_dx += bl_pad_ascending[-1]

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
copy_bl = bl.copy()
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
for i in range(len(copy_bl)):
  oobl_radii += " " + str(copy_bl[i] + pad_thickness)

gi = ""
for i in range(e_per_exterior_bl):
  gi += " " + str(gap_id)

ds = ""; di = ""; dbi = ""
for i in range(e_per_interior_bl):
  ds += " " + str(duct_radii[i])
  di += " " + str(1)
  dbi += " " + str(fluid_id)

dbi_lp = ""; di_lp = ""
for i in range(e_per_exterior_bl):
  dbi_lp += " " + str(load_pad_id)
  di_lp += " " + str(1)

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
  f.write(str.format('e_per_exterior_bl={0}\n',e_per_exterior_bl))
  f.write(str.format('e_per_interior_bl={0}\n', e_per_interior_bl))
  f.write(str.format('e_per_pad_bl={0}\n',e_per_pad_bl))
  f.write(str.format('e_per_duct_span={0}\n',e_per_duct_span))
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
  f.write("ds='" + ds + "'\n")
  f.write("di='" + di + "'\n")
  f.write("di_lp='" + di_lp + "'\n")
  f.write("dbi='" + dbi + "'\n")
  f.write("dbi_lp='" + dbi_lp + "'\n")
  f.write("gi='" + gi + "'\n")
  f.write("oobl_height='" + oobl_radii + "'\n")
  f.write("bl_layers='" + bl_layers + "'\n")
  f.write("lower_bl='" + lower_bl + "'\n")
  f.write("upper_bl='" + upper_bl + "'\n")
  f.write("s_swaps='" + s_swaps + "'\n")
  f.write("s_swaps_extra='" + s_swaps_extra + "'\n")
  f.write("polygon_origins=" + str(bundle_origins) + "\n")
  f.write("cs='" + cs + "'\n")

if (args.generate):
  import os
  var = os.system("/home/anovak/cardinal/cardinal-opt -i mesh_info.i bundle.i " + \
    " --mesh-only --n-threads=10")
  if (var):
    raise ValueError('Failed to run the bundle.i mesh script!')

  var = os.system("/home/anovak/cardinal/cardinal-opt -i mesh_info.i convert.i " + \
    " --mesh-only --n-threads=10")
  if (var):
    raise ValueError('Failed to run the convert.i mesh script!')

  os.system("mv convert_in.e fluid.exo")

  print('Generated mesh with a characteristic length of: ', hydraulic_diameter)
  print('Inlet flow area:                                ', flow_area / (hydraulic_diameter**2))
  print('Height:                                         ', h / hydraulic_diameter)
