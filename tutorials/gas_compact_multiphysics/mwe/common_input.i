channel_diameter = 0.016                 # diameter of the coolant channels (m)
compact_diameter = 0.0127                # diameter of fuel compacts (m)
fuel_to_coolant_distance = 0.0188        # distance between center of fuel compact and coolant channel (m)
bundle_flat_to_flat = 0.35996            # bundle flat-to-flat distance (m)
bundle_gap_width = 4e-3                  # gap width between bundles (m)
height = 6.343                           # height of the full core (m)
triso_pf = 0.15                          # TRISO packing fraction (%)
kernel_radius = 214.85e-6                # fissile kernel outer radius (m)

# operating conditions for the full core

inlet_T = 598.0                          # inlet fluid temperature (K)
mdot = 117.3                             # fluid mass flowrate (kg/s)
outlet_P = 7.1e6                         # fluid outlet pressure (Pa)

# other full core specifications used to construct the unit cell model

n_bundles = 12                           # number of bundles in the full core
n_coolant_channels_per_block = 108       # number of coolant channels per assembly
n_fuel_compacts_per_block = 210          # number of fuel compacts per assembly
unit_cell_height = 1.6                   # unit cell height - arbitrarily selected

# other settings for generating plots

num_layers_for_plots = 50                # number of layers to average fields over for plotting
