h = 20.32e-2                 # height to extrude
flat_to_flat = 14.922e-2     # flat-to-flat inside the duct
bundle_pitch = 16.142e-2     # bundle pitch
corner_radius = 1e-2         # radius of curvature of duct corners
thickness = 0.394e-2         # duct thickness
vessel_inner_diameter = 0.75 # vessel inner diameter
n_bundles = 7                # number of fuel bundles

nl = 1                       # number of axial layers
e_per_side = 2               # number of elements along each duct wall
e_per_bl = 1                 # number of elements in each boundary layer
e_per_peripheral = 1         # number of elements to put in the peripheral region
growth_factor = 1.7          # growth factor for the boundary layers
bl_height = 0.0001           # height of the first boundary layer

# Smoothing factors to apply to the corner movement; must match the length
# of the e_per_bl/
corner_smoothing = '1'
