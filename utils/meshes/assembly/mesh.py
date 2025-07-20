#!/bin/python
import os
import math
import sys
import numpy as np
from argparse import ArgumentParser

# Common meshing utilities
this_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from mesh_utils import build_pattern

# Create a mesh for the fluid region inside a hexagonal duct.
# The sideset IDs are:

# 1: pin surface
# 2: inlet
# 3: outlet
# 4: duct inner surface

# All of the settings for the mesh (geometry, refinements) are
# defined in the mesh_settings.py file. The hydraulic diameter and
# flow area are evaluated accounting for the wire, even though the
# wire is not explicitly meshed.

####################################################################

ap = ArgumentParser()
ap.add_argument('-g', '--generate', action='store_true',
                help='Whether to generate the mesh')

args = ap.parse_args()

script_dir = os.path.dirname(__file__)
sys.path.append(script_dir)
import mesh_settings as ms

nondimensional = ms.nondimensional
pin_diameter = ms.pin_diameter
pin_pitch = ms.pin_pitch
flat_to_flat = ms.flat_to_flat
corner_radius = ms.corner_radius
wire_diameter = ms.wire_diameter
wire_pitch = ms.wire_pitch
h = ms.h
n_pins = ms.n_pins

e_per_side = ms.e_per_side
e_per_bl = ms.e_per_bl
e_per_pin_background = ms.e_per_pin_background
e_per_assembly_background = ms.e_per_assembly_background
growth_factor = ms.growth_factor
nl = ms.nl
bl_height = ms.bl_height
corner_smoothing = ms.corner_smoothing

# dummy block IDs just for mesh creation purposes
fluid_id = 5

# constants for hexagonal lattices
cos60 = 0.5
sin60 = math.sqrt(3.0) / 2.0
corner_shiftx = [cos60, -cos60, -1, -cos60, cos60, 1]
corner_shifty = [sin60, sin60, 0, -sin60, -sin60, 0]
edge_shiftx = [-1, -cos60, cos60, 1, cos60, -cos60]
edge_shifty = [0, -sin60, -sin60, 0, sin60, sin60]

def side_length(pitch):
  """Get the side length of a hexagon with given flat-to-flat distance (a.k.a pitch)"""
  return pitch / math.sqrt(3.0)

def area(pitch):
  """Get the area of a hexagon with a given flat-to-flat distance (a.k.a. pitch)"""
  side = side_length(pitch)
  return 3 * math.sqrt(3.0) / 2.0 * (side**2)

def perimeter(pitch):
  """Get the perimeter of a hexagon with a given flat-to-flat distance (a.k.a. pitch)"""
  return 6 * side_length(pitch)

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
    lattices += str(rotated_x) + " " + str(rotated_y) + " 0.0 "

  return lattices[:-1] + "'"

def flow_area(nrings, bundle_pitch, pin_diameter):
  """Compute the flow area for a hexagonal bundle consisting of nrings of
     pins with a given bundle pitch and outer diameter."""

  num_pins = elements(nrings)
  pin_area = math.pi * (pin_diameter**2) / 4.0
  return area(bundle_pitch) - num_pins * pin_area

def hydraulic_diameter(nrings, bundle_pitch, pin_diameter, wire_diameter = 0, wire_pitch = 1.0):
  """Function to compute the hydraulic diameter of a hexagonal lattice formed
     of 'nrings' rings of pins with specified bundle pitch and pin outer diameter.
     Because the wire lead pitch affects how much of the bundle is available for
     fluid flow, we compute the hydraulic diameter as 4 * flow volume / wetted area.
     For bare bundles, the wire_diameter and wire_pitch parameters are optional."""

  # everything should scale with height, so use a dummy height here for easier readability
  height = 1.0

  bare_volume = flow_area(nrings, bundle_pitch, pin_diameter) * height

  num_pins = elements(nrings)
  pin_perimeter = num_pins * math.pi * pin_diameter
  duct_perimeter = perimeter(bundle_pitch)
  bare_area = (pin_perimeter + duct_perimeter) * height

  n_wire_coils = height / wire_pitch;
  wire_length_per_coil = math.sqrt(wire_pitch**2 + math.pow(math.pi * (pin_diameter + wire_diameter), 2))
  wire_length = wire_length_per_coil * n_wire_coils
  wire_volume = num_pins * math.pi * (wire_diameter**2) / 4.0 * wire_length

  wire_area = num_pins * math.pi * wire_diameter * wire_length

  volume = bare_volume - wire_volume
  area = bare_area + wire_area

  return 4 * volume / area

# Get the 'pattern' needed for the pins
n_rings = rings(n_pins)
pattern = build_pattern(n_rings)

# Get the coordinates of the pin centers
pin_centers = lattice_centers(n_rings, pin_pitch)

bl_radii = ""
previous_dx = 0.0
for i in range(e_per_bl):
  bl_radii += " " + str(math.pow(growth_factor, i) * bl_height + pin_diameter / 2.0 + previous_dx)
  previous_dx += math.pow(growth_factor, i) * bl_height

total_bl_thickness = 0.0
for i in range(e_per_bl):
  total_bl_thickness += math.pow(growth_factor, i) * bl_height

duct_radii = ""
dr = []
previous_r = flat_to_flat / 2.0
for i in range(e_per_bl):
  dr.append(previous_r - math.pow(growth_factor, i) * bl_height)
  previous_r -=  math.pow(growth_factor, i - 1) * bl_height

dr.reverse()
for i in range(len(dr)):
  duct_radii += " " + str(dr[i])

fluid_ids = "'"
fluid_elems = "'"
for i in range(e_per_bl):
  fluid_ids += " " + str(fluid_id)
  fluid_elems += " 1"

# calculate hydraulic diameter for a wire-wrap bundle (no curved corners)
inlet_area = flow_area(n_rings, flat_to_flat, pin_diameter) - n_pins * math.pi * (wire_diameter ** 2) / 4.0

if (nondimensional):
  hydraulic_d = hydraulic_diameter(n_rings, flat_to_flat, pin_diameter, wire_diameter, wire_pitch)
else:
  hydraulic_d = 1.0

# Write a file that contains all the essential meshing pre-processor definitions
with open('mesh_info.i', 'w') as f:
  f.write(str.format('pin_diameter={0}\n', pin_diameter))
  f.write(str.format('e_per_bl={0}\n', e_per_bl))
  f.write(str.format('hydraulic_diameter={0}\n', hydraulic_d))
  f.write(str.format('duct_corner_radius_of_curvature={0}\n', corner_radius))
  f.write(str.format('flat_to_flat={0}\n', flat_to_flat))
  f.write(str.format('e_per_side={0}\n', e_per_side))
  f.write(str.format('e_per_pin_background={0}\n', e_per_pin_background))
  f.write(str.format('e_per_assembly_background={0}\n', e_per_assembly_background))
  f.write(str.format('nl={0}\n', nl))
  f.write(str.format('fluid_id={0}\n', fluid_id))
  f.write(str.format('pin_pitch={0}\n', pin_pitch))
  f.write(str.format("bl_radii='" + bl_radii + "'\n"))
  f.write(str.format("duct_radii='" + duct_radii + "'\n"))
  f.write(str.format('total_bl_thickness={0}\n', total_bl_thickness))
  f.write(str.format('h={0}\n', h))
  f.write("fluid_ids=" + fluid_ids + "'\n")
  f.write("fluid_elems=" + fluid_elems + "'\n")
  f.write("pattern=" + str(pattern) + "\n")
  f.write("pin_centers=" + str(pin_centers) + "\n")
  f.write("corner_smoothing='" + corner_smoothing + "'\n")
  f.write("corners=" + str(corner_radius > 0) + "\n")

if (args.generate):
  home = os.getenv('HOME')
  var = os.system(home + "/cardinal/cardinal-opt -i mesh_info.i fluid.i " + \
    " --mesh-only --n-threads=10")
  if (var):
    raise ValueError('Failed to run the fluid.i mesh script!')

  var = os.system(home + "/cardinal/cardinal-opt -i mesh_info.i convert.i " + \
    " --mesh-only --n-threads=10")
  if (var):
    raise ValueError('Failed to run the convert.i mesh script!')

  os.system("mv convert_in.e fluid.exo")

  print('Generated mesh with a characteristic length of: ', hydraulic_d)
  print('Inlet flow area:                                ', inlet_area / (hydraulic_d**2))
  print('Height:                                         ', h / hydraulic_d)
