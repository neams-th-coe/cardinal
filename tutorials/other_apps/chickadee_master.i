[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = SoluteDiffusion
    variable = u
    coefficient = 1.4
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    value = 1.0
    boundary = 'left'
  []
  [right]
    type = DirichletBC
    variable = u
    value = 2.0
    boundary = 'right'
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = cardinal_sub.i
    execute_on = timestep_end

    # change this for wherever you have Cardinal located
    library_path = '/home/anovak/cardinal/lib'
  []
[]

[Transfers]
  # fill this out with whatever data transfers you want to/from Cardinal
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-6
[]

[Outputs]
  exodus = true
[]
