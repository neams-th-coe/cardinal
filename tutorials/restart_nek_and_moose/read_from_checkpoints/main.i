[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Problem]
   type = FEProblem
   restart_file_base = ../cardinal_sub_checkpoint_cp/LATEST # restart from LATEST time
   force_restart = true # force restart sub app only
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [trans]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Postprocessors]
  [midValue]
    type = PointValue
    variable = u
    point = '0.5 0.5 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient

  start_time = 0 # force start time to zero, same as nek restarted time
  num_steps = 10
  dt = 5e-4

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
[]
