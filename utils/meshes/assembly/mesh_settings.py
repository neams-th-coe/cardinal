pin_diameter = 9.54236e-3     # pin diameter
pin_pitch = 11.2769e-3        # pin pitch
flat_to_flat = 0.035          # duct inner flat-to-flat
wire_diameter = 1.7062e-3     # wire diameter
wire_pitch = 0.28499          # wire axial pitch
h = 0.28499                   # height
n_pins = 7                    # number of pins

e_per_side = 2                # number of elements along each 1/6 of the pin
e_per_bl = 3                  # number of elements in each boundary layer
e_per_pin_background = 1      # number of background elements in pincell
e_per_assembly_background = 3 # number of background elements in assembly
growth_factor = 1.8           # boundary layer growth factor
nl = 1                        # number of axial layers
bl_height = 0.00006           # height of first boundary layer

# If you would like to apply a radius of curvature to the corners of the
# duct, set the following parameters.
#
# corner_radius: radius of curvature of duct corners
# corner_smoothing: Smoothing factors to apply to the corner movement; must match
#                   the length of the e_per_bl + e_per_assembly_background. You may
#                   need to adjust this parameter if the default doesn't work nicely.
corner_radius = 6.858e-3
corner_smoothing = '1.0 1.0 1.0 0.75 0.25 0.25'
