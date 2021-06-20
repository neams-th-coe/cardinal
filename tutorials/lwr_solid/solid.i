r_pin = 0.39218
clad_ir = 0.40005
clad_or = 0.45720
L = 300.0

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
    initial_condition = 300
  []
[]

[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = CONSTANT
  []
  [density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [rho]
    type = ParsedAux
    variable = density
    function = '-0.4884*temp+2413.0'
    args = 'temp'
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
    block = '1'
  []
  [sink]
    type = BodyForce
    variable = temp
    value = -20.0
    block = '2'
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = temp
    boundary = 'top'
    value = 1000.0
  []
  [bottom]
    type = DirichletBC
    variable = temp
    boundary = 'bottom'
    value = 400.0
  []
  [surface]
    type = FunctionDirichletBC
    variable = temp
    boundary = 'surface'
    function = axial
  []
[]

[Functions]
  [axial]
    type = ParsedFunction
    value = '400+z*(1000-400)/10.0'
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_values = '0.5'
    prop_names = 'thermal_conductivity'
    block = '1'
  []
  [k2]
    type = GenericConstantMaterial
    prop_values = '0.05'
    prop_names = 'thermal_conductivity'
    block = '2'
  []
  [k3]
    type = GenericConstantMaterial
    prop_values = '2.0'
    prop_names = 'thermal_conductivity'
    block = '3'
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 2
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
    input_files = 'openmc.i'
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
  [density_to_openmc]
    type = MultiAppMeshFunctionTransfer
    direction = to_multiapp
    multi_app = openmc
    variable = density
    source_variable = density
  []
[]
