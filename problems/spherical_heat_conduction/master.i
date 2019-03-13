[Mesh]
  type = FileMesh
  file = sphere.e
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = BodyForce
    value = 1e9
    variable = temp
  []
[]

[BCs]
  inactive = 'outside'
  [outside]
    type = DirichletBC
    variable = temp
    boundary = '1'
    value = 300
  []
  [match_nek]
    type = MatchedValueBC
    variable = temp
    boundary = '1'
    v = 'nek_temp'
  []
[]

[Materials]
  [hc]
    type = GenericConstantMaterial
    prop_values = '150' # W/mK
    prop_names = 'thermal_conductivity'
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 5
  petsc_options_value = 'hypre boomeramg'
  dt = 1e-4
[]

[Variables]
  [temp]
  []
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = NekApp
    input_files = 'nek.i'
  []
[]

[Transfers]
  [nek_temp]
    type = MultiAppNearestNodeTransfer
    source_variable = temp
    direction = from_multiapp
    multi_app = nek
    variable = nek_temp
  []
  [avg_flux]
    type = MultiAppNearestNodeTransfer
    source_variable = avg_flux
    direction = to_multiapp
    multi_app = nek
    variable = avg_flux
  []
[]

[AuxVariables]
  [nek_temp]
  []
  [avg_flux]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [avg_flux]
    type = FluxAverageAux
    coupled = 'temp'
    diffusivity = thermal_conductivity
    variable = avg_flux
    boundary = '1'
  []
[]
