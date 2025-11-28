[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
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
  temperature_blocks = '100'
  cell_level = 0
  initial_properties = xml

  source_rate_normalization = 'kappa_fission'

  relaxation = constant
  relaxation_factor = 0.5

  [Tallies]
    [Mesh_1]
      type = MeshTally
      score = kappa_fission
      mesh_translations = '0 0 0'
      mesh_template = ../meshes/sphere.e

      # The global tally check is disabled because we have a loosely fitting unstructured mesh tally.
      normalize_by_global_tally = false
    []
    [Mesh_2]
      type = MeshTally
      score = flux
      mesh_translations = '0 0 0'
      mesh_template = ../meshes/sphere.e

      # The global tally check is disabled because we have a loosely fitting unstructured mesh tally.
      normalize_by_global_tally = false
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
