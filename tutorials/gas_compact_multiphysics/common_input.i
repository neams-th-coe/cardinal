# This "input" file has various common parameters to be shared across
# MOOSE input files and OpenMC python scripts. To run any of the MOOSE
# input files, use
#
# cardinal-opt -i common_input.i <input file you want to run>
#
# All parameters in this input file should be in SI units! All
# specifications come from the following report except where noted:
#
# [1] https://inldigitallibrary.inl.gov/sites/sti/sti/6899493.pdf

# geometric parameters

channel_diameter = 0.016                 # diameter of the coolant channels (m)
compact_diameter = 0.0127                # diameter of fuel compacts (m)
fuel_to_coolant_distance = 0.0188        # distance between center of fuel compact and coolant channel (m)
bundle_flat_to_flat = 0.35996            # bundle flat-to-flat distance (m)
bundle_gap_width = 4e-3                  # gap width between bundles (m)
height = 6.343                           # height of the full core (m)
vessel_inner_diameter = 3.3              # inner diameter of reactor vessel (m)
vessel_thickness = 0.05                  # vessel thickness (m) - no value provided in [1], this is selected arbitrarily
top_reflector_thickness = 1.585          # top graphite reflector thickness (m)
bottom_reflector_thickness = 1.585       # bottom graphite reflector thickness (m)
triso_pf = 0.15                          # TRISO packing fraction (%)
kernel_radius = 214.85e-6                # fissile kernel outer radius (m)
buffer_radius = 314.85e-6                # buffer outer radius (m)
iPyC_radius = 354.85e-6                  # inner PyC outer radius (m)
SiC_radius = 389.85e-6                   # SiC outer radius (m)
oPyC_radius = 429.85e-6                  # outer PyC outer radius (m)

# material parameters

B4C_density = 2380                       # B4C density (kg/m3)
fluid_density = 5.5508                   # fluid density (kg/m3)
fluid_viscosity = 3.22639e-5             # fluid dynamic viscosity (Pa-s)
fluid_Cp = 5189.0                        # fluid isobaric specific heat (J/kg/K)
fluid_k = 0.2556                         # fluid conductivity (W/m/K)
B10_enrichment = 0.30                    # B-10 enrichment in boron in the poison (weight percent)
total_B10_wt_percent = 0.001             # total weight percent of B-10 in the poison material (B_nC)
buffer_k = 0.5                           # buffer thermal conductivity (W/m/K)
PyC_k = 4.0                              # PyC thermal conductivity (W/m/K)
SiC_k = 13.9                             # SiC thermal conductivity (W/m/K)
kernel_k = 3.5                           # fissil kernel thermal conductivity (W/m/K)
matrix_k = 15.0                          # graphite matrix thermal conductivity (W/m/K)

# operating conditions for the full core

inlet_T = 598.0                          # inlet fluid temperature (K)
power = 200e6                            # full core power (W)
mdot = 117.3                             # fluid mass flowrate (kg/s)
outlet_P = 7.1e6                         # fluid outlet pressure (Pa)

# other full core specifications used to construct the unit cell model

n_bundles = 12                           # number of bundles in the full core
n_coolant_channels_per_block = 108       # number of coolant channels per assembly
n_fuel_compacts_per_block = 210          # number of fuel compacts per assembly
unit_cell_height = 1.6                   # unit cell height - arbitrarily selected

# other settings for generating plots

num_layers_for_plots = 50                # number of layers to average fields over for plotting
