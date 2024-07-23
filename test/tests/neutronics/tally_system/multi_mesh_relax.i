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
  [fluid]
    type = FileMeshGenerator
    file = ../heat_source/stoplight.exo
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '200'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
  []

  allow_renumbering = false
[]

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = temp
    function = axial
    execute_on = initial
  []
[]

[Functions]
  [axial]
    type = ParsedFunction
    expression = '500 + z / 0.10 * 100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100 200'
  cell_level = 0
  initial_properties = xml

  source_rate_normalization = 'kappa_fission'

  relaxation = constant
  relaxation_factor = 0.5

  # The global tally check is disabled because we have a loosely fitting unstructured mesh tally.
  normalize_by_global_tally = false

  [Tallies]
    [Mesh_1]
      type = MeshTally
      tally_score = kappa_fission
      mesh_translations = '0 0 0
                           0 0 4
                           0 0 8'
      mesh_template = ../meshes/sphere.e
    []
    [Mesh_2]
      type = MeshTally
      tally_score = flux
      mesh_translations = '0 0 0
                           0 0 4
                           0 0 8'
      mesh_template = ../meshes/sphere.e
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
