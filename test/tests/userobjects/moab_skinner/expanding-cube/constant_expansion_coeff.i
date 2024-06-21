[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [temp]
    initial_condition = 1000
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        incremental = true
        add_variables = true
        eigenstrain_names = eigenstrain
        generate_output = 'strain_xx strain_yy strain_zz'
      []
    []
  []
[]

[BCs]
  [x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  []
  [small_stress]
    type = ComputeFiniteStrainElasticStress
  []
  [thermal_expansion_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 298
    thermal_expansion_coeff = 1.3e-3
    temperature = temp
    eigenstrain_name = eigenstrain
  []
[]

[Executioner]
  type = Transient

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  end_time = 0.025
  dt = 0.0125
[]

[Outputs]
  exodus = true
  checkpoint = true
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = 'all_bins.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [temp_from_source]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = sub
    variable = temp
    source_variable = temp
  []
  [dispx]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = disp_x
    variable = disp_x
    to_multi_app = sub
  []
  [dispy]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = disp_y
    variable = disp_y
    to_multi_app = sub
  []
  [dispz]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = disp_z
    variable = disp_z
    to_multi_app = sub
  []
[]

[Postprocessors]
  [strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  []
  [strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  []
  [strain_zz]
    type = ElementAverageValue
    variable = strain_zz
  []
  [temperature]
    type = AverageNodalVariableValue
    variable = temp
  []
[]
