#!/bin/python
import os
import math
import sys
import numpy as np
from argparse import ArgumentParser

# Create a mesh of the gap region between hexagonal ducts
# enclosed in a cylindrical barrel, without any structural components
# (i.e. load pads, restraint rings, etc). The sideset IDs are:

# 1: inlet
# 2: outlet
# 3: duct outer walls
# 4: vessel inner diameter

# All of the settings for the mesh (geometry, refinements) are
# defined in the mesh_settings.py file. You should not need to
# edit anything in this script except optionally:

####################################################################

ap = ArgumentParser()
ap.add_argument('-g', '--generate', action='store_true',
                help='Whether to generate the mesh')

args = ap.parse_args()

script_dir = os.path.dirname(__file__)
sys.path.append(script_dir)
import mesh_settings as ms

h = ms.h
flat_to_flat = ms.flat_to_flat
bundle_pitch = ms.bundle_pitch
corner_radius = ms.corner_radius
thickness = ms.thickness
vessel_inner_diameter = ms.vessel_inner_diameter
n_bundles = ms.n_bundles

nl = ms.nl
e_per_side = ms.e_per_side
e_per_bl = ms.e_per_bl
e_per_peripheral = ms.e_per_peripheral
growth_factor = ms.growth_factor
bl_height = ms.bl_height
corner_smoothing = ms.corner_smoothing

# dummy block IDs just for mesh creation purposes
gap_id = 5
load_pad_id = 25
periph_id = 50

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
first_row = int(elements_in_ring(n_rings) / 6) + 1
last_row = first_row + n_rings - 1

pattern = "'"
for i in range(n_rings):
  for j in range(first_row + i):
    pattern += " 0"
  pattern += ";"
for i in range(n_rings - 1):
  for j in range(last_row - 1 - i):
    pattern += " 0"
  pattern += ";"
pattern += "'"

# This is the half-thickness of the gap
outer_flat_to_flat = flat_to_flat + 2.0 * thickness
gap_dx = (bundle_pitch - outer_flat_to_flat) / 2.0

# Get the origins of the bundles
bundle_origins = lattice_centers(n_rings, bundle_pitch)

bl = []
previous_dx = 0.0
starting_pt = flat_to_flat / 2.0 + thickness
for i in range(e_per_bl):
  layer = math.pow(growth_factor, i) * bl_height + starting_pt + previous_dx
  bl.append(layer)
  previous_dx += math.pow(growth_factor, i) * bl_height

duct_ids = "'"
pad_ids = "'"
bl_radii = "'"
for i in range(len(bl)):
  bl_radii += " "+ str(bl[i])
  pad_ids += " " + str(gap_id)
  duct_ids += " 1"

# determine height of first element
t = 0.0
for i in range(e_per_bl):
  t += math.pow(growth_factor, i)

l0 = gap_dx / (t + math.pow(growth_factor, e_per_bl))

# then, determine the total height of the boundary layer
t = 0.0
for i in range(e_per_bl):
  t += math.pow(growth_factor, i) * l0

# for the transition layers on the outermost ring of ducts, continue the growth factor pattern
largest_mid_duct_dx = math.pow(growth_factor, e_per_bl + 1) * l0

t_outer = 0.0
for i in range(e_per_bl):
  t_outer += math.pow(growth_factor, i) * l0

# non-dimensionalize to the hydraulic diameter
hex_area = math.sqrt(3.0) / 2.0 * outer_flat_to_flat**2
hex_perimeter = outer_flat_to_flat / math.sqrt(3.0) * 6

flow_area = (math.pi * vessel_inner_diameter**2 / 4.0 - n_bundles * hex_area)
wetted_perimeter = (math.pi * vessel_inner_diameter + n_bundles * hex_perimeter)
hydraulic_diameter = 4.0 * flow_area / wetted_perimeter

# Write a file that contains all the essential meshing pre-processor definitions
with open('mesh_info.i', 'w') as f:
  f.write(str.format('flat_to_flat={0}\n', flat_to_flat))
  f.write(str.format('outer_flat_to_flat={0}\n', outer_flat_to_flat))
  f.write(str.format('corner_radius={0}\n', corner_radius))
  f.write(str.format("thickness={0}\n", thickness))
  f.write(str.format("e_per_side={0}\n", e_per_side))
  f.write(str.format("e_per_bl={0}\n", e_per_bl))
  f.write(str.format("e_per_peripheral={0}\n", e_per_peripheral))
  f.write(str.format("growth_factor={0}\n", growth_factor))
  f.write(str.format("bl_outer_dx={0}\n", t_outer))
  f.write(str.format("gap_id={0}\n", gap_id))
  f.write(str.format("periph_id={0}\n", periph_id))
  f.write(str.format("load_pad_id={0}\n", load_pad_id))
  f.write(str.format("bundle_pitch={0}\n", bundle_pitch))
  f.write(str.format("gap_dx={0}\n", gap_dx))
  f.write(str.format("vessel_inner_diameter={0}\n", vessel_inner_diameter))
  f.write(str.format("height={0}\n", h))
  f.write(str.format("hydraulic_diameter={0}\n", hydraulic_diameter))
  f.write(str.format("nl={0}\n", nl))
  f.write("bl_height=" + str(bl_radii) + "'\n")
  f.write("pad_ids=" + str(pad_ids) + "'\n")
  f.write("duct_ids=" + str(duct_ids) + "'\n")
  f.write("pattern=" + str(pattern) + "\n")
  f.write("polygon_origins=" + str(bundle_origins) + "\n")
  f.write("corner_smoothing='" + corner_smoothing + "'\n")

if (args.generate):
  home = os.getenv('HOME')
  var = os.system(home + "/cardinal/cardinal-opt -i mesh_info.i plane.i " + \
    " --mesh-only --n-threads=10")
  if (var):
    raise ValueError('Failed to run the plane.i mesh script!')

  var = os.system(home + "/cardinal/cardinal-opt -i mesh_info.i convert.i " + \
    " --mesh-only --n-threads=10")
  if (var):
    raise ValueError('Failed to run the convert.i mesh script!')

  os.system("mv convert_in.e fluid.exo")

  print('Generated mesh with a characteristic length of: ', hydraulic_diameter)
  print('Inlet flow area:                                ', flow_area / (hydraulic_diameter**2))
  print('Height:                                         ', h / hydraulic_diameter)
