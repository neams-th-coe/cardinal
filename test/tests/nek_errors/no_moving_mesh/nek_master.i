# This is just a dummy master input file so that other errors related
# to the synchronization transfer dont error before we get to our desired
# error check that we DO want to trigger.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [temp]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = temp
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = NekApp
    input_files = 'nek.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Postprocessors]
  [synchronize]
    type = Receiver
    default = 1
  []
[]
