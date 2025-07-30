[Mesh]
  [circle]
    type = AnnularMeshGenerator
    nr = 10
    nt = 25
    rmin = 0
    rmax = 0.4e-2
    growth_r = -1.3
  []
  [cylinder]
    type = AdvancedExtruderGenerator
    input = circle
    heights = '0.05 0.7 0.05'
    num_layers = '5 10 5'
    direction = '0 0 1'
  []
  [transform]
    type = TransformGenerator
    input = cylinder
    transform = translate
    vector_value = '0 0 -0.4'
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]


[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.02
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek_isolated.i'
    sub_cycling = true
    execute_on = timestep_end
  []
[]

[Transfers]
  [flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    to_multi_app = nek
    variable = avg_flux
    from_boundaries = '1'
  []
  [flux_integral]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
    from_postprocessor = flux_integral
    to_multi_app = nek
  []
[]

[AuxVariables]
  [flux]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1.0
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1.5'
  []
[]

[Postprocessors]
  [flux_integral]
    type = ConstantPostprocessor
    value = 10
  []
[]

[Outputs]
  exodus = true
[]
