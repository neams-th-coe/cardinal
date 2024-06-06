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

e_per_side = 2                # elements per side of the duct (i.e. on each hex face)
e_per_gap_span = 1            # elements across non-boundary layer part of gap
e_per_load_pad_span = 1       # elements across the non-boundary layer part of load pad
e_per_peripheral = 1          # elements across the peripheral region
e_per_bl = 1                  # elements in each boundary layer on the duct wall
e_per_pad_bl = 1              # elements in the boundary layers on the bottom/top faces of the load pads
e_per_gap_bl = 1              # elements in the boundary layer between the pads and ring
bl_height = 0.0001            # height of first boundary layer
bl_pad_height = 0.0001        # height of first boundary layer on bottom/top faces of load pads
growth_factor = 1.8           # boundary layer growth factor

num_layers_per_dz = 1.0       # layers per axial pitch

# Smoothing factors to apply to the corner movement; must match the length
# of the e_per_bl.
corner_smoothing = '1'

