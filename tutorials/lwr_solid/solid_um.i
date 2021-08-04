r_pin = 0.39218
clad_ir = 0.40005
clad_or = 0.45720
L = 300.0

T_fluid = ${fparse 280.0 + 273.15}

[Mesh]
  [clad] # This makes a circular annulus that will represent the clad
    type = AnnularMeshGenerator
    nr = 3
    nt = 20
    rmin = ${clad_ir}
    rmax = ${clad_or}
    quad_subdomain_id = 1
    tri_subdomain_id = 0
  []
  [extrude_clad] # this extrudes the circular annulus in the axial direction
    type = FancyExtruderGenerator
    input = clad
    heights = '${L}'
    num_layers = '40'
    direction = '0 0 1'
  []
  [rename_clad] # this renames some sidesets on the clad to avoid name clashes
    type = RenameBoundaryGenerator
    input = extrude_clad
    old_boundary = '1 0' # outer surface, inner surface
    new_boundary = '5 4'
  []
  [fuel] # this makes a circle that will represent the fuel
    type = AnnularMeshGenerator
    nr = 10
    nt = 20
    rmin = 0
    rmax = ${r_pin}
    quad_subdomain_id = 2
    tri_subdomain_id = 3
    growth_r = -1.2
  []
  [extrude] # this extrudes the circle in the axial direction
    type = FancyExtruderGenerator
    input = fuel
    heights = '${L}'
    num_layers = '40'
    direction = '0 0 1'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'rename_clad extrude'
  []
[]

[Variables]
  [temp]
    initial_condition = ${T_fluid}
  []
[]

[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = CoupledForce
    variable = temp
    v = heat_source
  []
[]

[BCs]
  [surface]
    type = ConvectiveFluxFunction
    T_infinity = '${T_fluid}'

    # convert from W/m2/K to W/cm2/K
    coefficient = ${fparse 1000.0/100.0/100.0}
    variable = temp
    boundary = '5'
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
    variable = temp
    primary = '1'
    secondary = '4'

    # we will use a quadrature-based approach to find the gap width and cross-side temperature
    quadrature = true

    # emissivity of the fuel
    emissivity_primary = 0.8

    # emissivity of the clad
    emissivity_secondary = 0.8

    # thermal conductivity of the gap material
    gap_conductivity = 1.0

    # geometric terms related to the gap
    gap_geometry_type = CYLINDER
    cylinder_axis_point_1 = '0 0 0'
    cylinder_axis_point_2 = '0 0 ${L}'
  []
[]

[Materials]
  [k_clad]
    type = GenericConstantMaterial
    prop_values = '0.5'
    prop_names = 'thermal_conductivity'
    block = '1'
  []
  [k_fuel]
    type = GenericConstantMaterial
    prop_values = '0.05'
    prop_names = 'thermal_conductivity'
    block = '2 3'
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 10
  petsc_options_value = 'hypre boomeramg'
  dt = 1.0
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [openmc]
    type = TransientMultiApp
    app_type = OpenMCApp
    input_files = 'openmc_um.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [heat_source_from_openmc]
    type = MultiAppCopyTransfer
    direction = from_multiapp
    multi_app = openmc
    variable = heat_source
    source_variable = heat_source
  []
  [temp_to_openmc]
    type = MultiAppMeshFunctionTransfer
    direction = to_multiapp
    multi_app = openmc
    variable = temp
    source_variable = temp
  []
[]
