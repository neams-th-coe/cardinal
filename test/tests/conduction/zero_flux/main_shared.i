[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 1.0
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [flux]
    initial_condition = 1.0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.2
  nl_abs_tol = 1e-8
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek_vpp.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    to_multi_app = nek
    variable = avg_flux
  []
  [flux_integral]
    type = MultiAppReporterTransfer
    to_reporters = 'flux_integral/value'
    from_reporters = 'flux_sidesets/flux_sidesets'
    to_multi_app = nek
  []
[]

[VectorPostprocessors]
  [flux_sidesets]
    type = VectorOfPostprocessors
    postprocessors = 'flux_integral flux_integral flux_integral flux_integral flux_integral flux_integral flux_integral flux_integral'
  []
[]

[Postprocessors]
  [flux_integral]
    type = Receiver
    default = 1.0
  []
[]

[Outputs]
  exodus = true
[]
