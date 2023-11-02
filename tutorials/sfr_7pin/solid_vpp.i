d_pellet = 0.603e-2                           # pellet diameter (m)
d_pin = 8.0e-3                                # pin (clad) diameter (m)
t_clad = 0.52e-3                              # clad thickness (m)
L = 20.32e-2                                  # height (m)
bundle_pitch = 0.02625                        # flat-to-flat distance inside the duct (m)
duct_thickness = 0.004                        # duct thickness (m)
pin_power = 21e3                              # bundle power (kW)

[Mesh]

# --- DUCT --- #

  # Make the duct mesh in 2-D; we first create a "solid" hexagon with two blocks
  # so that we can set the inner wall sideset as the boundary between these two blocks,
  # and then delete an inner block to get just the duct walls
  [duct]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse bundle_pitch / 2.0 + duct_thickness}
    num_sectors_per_side = '14 14 14 14 14 14'
    uniform_mesh_on_sides = true

    duct_sizes = '${fparse bundle_pitch / 2.0}'
    duct_block_ids = '10'
    duct_intervals = '4'
    duct_sizes_style = apothem
  []
  [inner_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = duct
    new_boundary = '10'
    primary_block = '10'
    paired_block = '1'
  []
  [delete_inside_block]
    type = BlockDeletionGenerator
    input = inner_sideset
    block = '1'
  []

  # rotate the duct by 30 degrees, then add names for sidesets. Delete sideset 1 because
  # we will name some other part of our mesh with sideset 1.
  [rotate]
    type = TransformGenerator
    input = delete_inside_block
    transform = rotate
    vector_value = '30.0 0.0 0.0'
  []
  [rename_sideset_names]
    type = RenameBoundaryGenerator
    input = rotate
    old_boundary = '10000 10'
    new_boundary = 'duct_outer duct_inner'
  []
  [delete_extraneous]
    type = BoundaryDeletionGenerator
    input = rename_sideset_names
    boundary_names = '1'
  []

# --- CLAD --- #

  [clad]
    type = AnnularMeshGenerator
    nr = 3
    nt = 20
    rmin = ${fparse d_pin / 2.0 - t_clad}
    rmax = ${fparse d_pin / 2.0}
    quad_subdomain_id = 1
    tri_subdomain_id = 0
  []
  [rename_clad] # this renames some sidesets on the clad to avoid name clashes
    type = RenameBoundaryGenerator
    input = clad
    old_boundary = '1 0' # outer surface, inner surface
    new_boundary = '5 4'
  []
  [rename_clad_names] # this renames some names on the clad to avoid name clashes
    type = RenameBoundaryGenerator
    input = rename_clad
    old_boundary = '5 4' # outer surface, inner surface
    new_boundary = 'clad_outer clad_inner'
  []

# --- FUEL --- #

  [fuel]
    type = AnnularMeshGenerator
    nr = 10
    nt = 20
    rmin = 0
    rmax = ${fparse d_pellet / 2.0}
    quad_subdomain_id = 2
    tri_subdomain_id = 3
    growth_r = -1.2
  []

# --- COMBINE --- #

  [combine] # this combines the fuel and clad together to make one pin
    type = CombinerGenerator
    inputs = 'rename_clad_names fuel'
  []
  [repeat] # this repeats the pincell 7 times to get the 7 pins, and adds the duct
    type = CombinerGenerator
    inputs = 'combine combine combine combine combine combine combine delete_extraneous'
    positions = '+0.00000000 +0.00000000 +0.00000000
                 +0.00452000 +0.00782887 +0.00000000
                 -0.00452000 +0.00782887 +0.00000000
                 -0.00904000 +0.00000000 +0.00000000
                 -0.00452000 -0.00782887 +0.00000000
                 +0.00452000 -0.00782887 +0.00000000
                 +0.00904000 +0.00000000 +0.00000000
                 +0.0        +0.0        +0.0'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = repeat
    direction = '0 0 1'
    num_layers = '40'
    heights = '${L}'
  []
[]

[Variables]
  [T]
    initial_condition = 500.0
  []
[]

[Kernels]
  [diffusion]
    type = HeatConduction
    variable = T
  []
  [source]
    type = BodyForce
    variable = T
    function = ${fparse pin_power / (pi * (d_pellet * d_pellet / 4.0) * L)}
    block = '2 3'
  []
[]

[ThermalContact]
  # This adds boundary conditions bewteen the fuel and the cladding, which represents
  # the heat flux in both directions as
  # q''= h * (T_1 - T_2)
  # where h is a conductance that accounts for conduction through a material and
  # radiation between two infinite parallel plate gray bodies.
  [one_to_two]
    type = GapHeatTransfer
    variable = T
    primary = '1'
    secondary = '4'

    # we will use a quadrature-based approach to find the gap width and cross-side temperature
    quadrature = true

    # emissivity of the fuel
    emissivity_primary = 0.8

    # emissivity of the clad
    emissivity_secondary = 0.8

    # thermal conductivity of the gap material
    gap_conductivity_function = k_sodium
    gap_conductivity_function_variable = T

    # geometric terms related to the gap
    gap_geometry_type = CYLINDER
    cylinder_axis_point_1 = '0 0 0'
    cylinder_axis_point_2 = '0 0 ${L}'
  []
[]

[BCs]
  [pin_outer]
    type = MatchedValueBC
    variable = T
    v = nek_temp
    boundary = '5'
  []
  [duct_inner]
    type = MatchedValueBC
    variable = T
    v = nek_temp
    boundary = '10'
  []
[]

[Functions]
  [k_sodium]
    type = ParsedFunction
    expression = '1.1045e2 + -6.5112e-2 * t + 1.5430e-5 * t * t + -2.4617e-9 * t * t * t'
  []
  [k_HT9]
    type = ParsedFunction
    expression = 'if (t < 1030, 17.622 + 2.428e-2 * t - 1.696e-5 * t * t,
                           12.027 + 1.218e-2 * t)'
  []
  [k_U]
    type = ParsedFunction
    expression = 'if (t < 255.4, 16.170,
                            if (t < 1173.2, (5.907e-6 * t * t + 1.591e-2 * t + 11.712),
                                             38.508))'
  []
[]

[Materials]
  [clad_and_duct]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = k_HT9
    temp = T
    block = '1 10'
  []
  [pellet]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = k_U
    temp = T
    block = '2 3'
  []
[]

[VectorPostprocessors]
  [flux]
    type = VectorOfPostprocessors
    postprocessors = 'pin_flux duct_flux'
  []
[]

[Postprocessors]
  [pin_flux]
    type = SideDiffusiveFluxIntegral
    diffusivity = thermal_conductivity
    variable = T
    boundary = '5'
  []
  [duct_flux]
    type = SideDiffusiveFluxIntegral
    diffusivity = thermal_conductivity
    variable = T
    boundary = '10'
  []
  [max_fuel_T]
    type = NodalExtremeValue
    variable = T
    value_type = max
    block = '2 3'
  []
  [max_clad_T]
    type = NodalExtremeValue
    variable = T
    value_type = max
    block = '1'
  []
  [max_duct_T]
    type = NodalExtremeValue
    variable = T
    value_type = max
    block = '10'
  []
  [synchronize]
    type = Receiver
    default = 1
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'nek_vpp.i'
    sub_cycling = true
    execute_on = timestep_end
  []
[]

[Transfers]
  [nek_temp] # grabs temperature from nekRS and stores it in nek_temp
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
    fixed_meshes = true
  []
  [avg_flux] # sends heat flux in avg_flux to nekRS
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = avg_flux
    to_multi_app = nek
    variable = avg_flux
    fixed_meshes = true
  []
  [flux_integral_to_nek] # sends the heat flux integral (for normalization) to nekRS
    type = MultiAppReporterTransfer
    to_reporters = 'flux_integral/value'
    from_reporters = 'flux/flux'
    to_multi_app = nek
  []
  [synchronize_in]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = transfer_in
    from_postprocessor = synchronize
    to_multi_app = nek
  []
[]

[AuxVariables]
  [nek_temp]
    initial_condition = 500.0
  []
  [avg_flux]
    family = MONOMIAL
    order = CONSTANT
  []
[]


[AuxKernels]
  [avg_flux]
    type = DiffusionFluxAux
    diffusion_variable = T
    component = normal
    diffusivity = thermal_conductivity
    variable = avg_flux
    boundary = '5 10'
  []
[]

[Executioner]
  type = Transient
  dt = 5e-3
  num_steps = 10
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-16
  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_hypre_type'
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
