[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 4
  ny = 4
  nz = 4
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = temperature
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    execute_on = 'timestep_end'

    # This input file is run with 2 ranks, so one of these ranks is going to try
    # to set up two of these inputs. NekRS cant support that yet, so we throw an error.
    positions = '-0.5 0.0 0.0
                  0.5 0.0 0.0
                  1.0 0.0 0.0'
    output_in_position = true
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-8
  num_steps = 2
[]
