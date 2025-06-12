[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 48
  ny = 5
  nz = 5
  xmin = -0.5
  xmax = 0.0
  ymin = -0.025
  ymax = 0.025
  zmin = -0.025
  zmax = 0.025
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 100
  dt = 0.1
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    sub_cycling = true
    execute_on = timestep_end
  []
[]

[Transfers]
  [synchronize]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = transfer_in
    from_postprocessor = synchronize_in
    to_multi_app = nek
  []
[]

[Postprocessors]
  [synchronize_in]
    type = Receiver
  []
[]

