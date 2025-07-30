# Everything in this input file is in SI units.

[Mesh]
  type = FileMesh
  file = sphere_in_m.e
[]

[Variables]
  [temp]
    initial_condition = 300.0
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
    expression = '-0.4884*temp+2413.0'
    coupled_variables = 'temp'
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

[Materials]
  [k]
    type = GenericConstantMaterial
    # convert 1 W/cm/K to W/m/K
    prop_values = '${fparse 1.0 * 100}'
    prop_names = 'thermal_conductivity'
  []
[]

[BCs]
  [surface]
    type = DirichletBC
    variable = temp
    boundary = '1'
    value = 300.0
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 2
  petsc_options_value = 'hypre boomeramg'
  dt = 1.0
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-16
[]

[Outputs]
  exodus = true
  hide = 'density heat_source'
[]

[MultiApps]
  [openmc]
    type = TransientMultiApp
    input_files = 'openmc.i'
    execute_on = timestep_end
  []
[]

[Transfers]

  # For illustration, this cases uses a copy transfer because we use exactly the
  # same mesh and quadrature order as in the coupled OpenMC wrapping
  [heat_source_from_openmc]
    type = MultiAppCopyTransfer
    from_multi_app = openmc
    variable = heat_source
    source_variable = kappa_fission
  []
  [temp_to_openmc]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = openmc
    variable = temp
    source_variable = temp
  []
  [density_to_openmc]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = openmc
    variable = density
    source_variable = density
  []
[]

[Postprocessors]
  [max_T]
    type = ElementExtremeValue
    variable = temp
  []
  [avg_T]
    type = ElementAverageValue
    variable = temp
  []
  [T_at_pt1]
    type = PointValue
    variable = temp
    point = '0 0 0'
  []
  [T_at_pt2]
    type = PointValue
    variable = temp
    point = '0.05 0.06 0.07'
  []
[]
