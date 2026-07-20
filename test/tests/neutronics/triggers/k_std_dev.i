[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  k_trigger = std_dev
  k_trigger_threshold = 1.2e-2
  max_batches = 100
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  # Switched by command-line args to either std_dev or rel_err
  [k_err]
    type = KEigenvalue
    output = 'std_dev'
  []

  [threshold]
    type = ParsedPostprocessor
    expression = 'if (k_err < th, 1, 0)'
    pp_names = 'k_err'
    constant_names = 'th'
    constant_expressions = '1.2e-2'
  []
[]

[Outputs]
  csv = true
  hide = 'k_err'
  execute_on = 'TIMESTEP_END'
[]
