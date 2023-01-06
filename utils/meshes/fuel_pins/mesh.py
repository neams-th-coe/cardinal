#!/bin/python
import os
import sys
from argparse import ArgumentParser

# Create a mesh for bare (no wire) solid fuel pins inside a hexagonal assembly.
# The sideset IDs are:

# 3: clad surface
# 4: bottom
# 5: top

# All of the settings for the mesh (geometry, refinements) are defined in
# the mesh_settings.py file.

####################################################################

ap = ArgumentParser()
ap.add_argument('-g', '--generate', action='store_true',
                help='Whether to generate the mesh')

args = ap.parse_args()

script_dir = os.path.dirname(__file__)
sys.path.append(script_dir)
import mesh_settings as ms

h = ms.h
pin_pitch = ms.pin_pitch
pin_diameter = ms.pin_diameter
pellet_diameter = ms.pellet_diameter
flat_to_flat = ms.flat_to_flat

e_per_side = ms.e_per_side
e_per_clad = ms.e_per_clad
e_per_pellet = ms.e_per_pellet
nl = ms.nl

n_pins = ms.n_pins
n_bundles = ms.n_bundles

fluid_id = 5
fuel_id = 1
clad_id = 2

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

# Get the 'pattern' needed for the pins
n_rings = rings(n_pins)
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

# Get the 'pattern' needed for the core by assuming a simple hex grid of bundles
n_rings = rings(n_bundles)
first_row = int(elements_in_ring(n_rings) / 6) + 1
last_row = first_row + n_rings - 1

bundle_pattern = "'"
for i in range(n_rings):
  for j in range(first_row + i):
    bundle_pattern += " 0"
  bundle_pattern += ";"
for i in range(n_rings - 1):
  for j in range(last_row - 1 - i):
    bundle_pattern += " 0"
  bundle_pattern += ";"
bundle_pattern += "'"

# Write a file that contains all the essential meshing pre-processor definitions
with open('mesh_info.i', 'w') as f:
  f.write(str.format('h={0}\n', h))
  f.write(str.format('pin_pitch={0}\n', pin_pitch))
  f.write(str.format('pin_diameter={0}\n', pin_diameter))
  f.write(str.format('pellet_diameter={0}\n', pellet_diameter))
  f.write(str.format('flat_to_flat={0}\n', flat_to_flat))
  f.write(str.format('nl={0}\n', nl))
  f.write(str.format('e_per_side={0}\n', e_per_side))
  f.write(str.format('e_per_clad={0}\n', e_per_clad))
  f.write(str.format('e_per_pellet={0}\n', e_per_pellet))
  f.write(str.format('fluid_id={0}\n', fluid_id))
  f.write(str.format('fuel_id={0}\n', fuel_id))
  f.write(str.format('clad_id={0}\n', clad_id))
  f.write("pattern=" + str(pattern) + "\n")
  f.write("bundle_pattern=" + str(bundle_pattern) + "\n")

if (args.generate):
  home = os.getenv('HOME')
  var = os.system(home + "/cardinal/cardinal-opt -i mesh_info.i solid.i " + \
    " --mesh-only --n-threads=10")
  if (var):
    raise ValueError('Failed to run the solid.i mesh script!')

  os.system("mv solid_in.e solid.exo")
