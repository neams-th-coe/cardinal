[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../meshes/sphere.e
  []
  [solid1]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []

  allow_renumbering = false
  parallel_type = replicated
[]

[AuxVariables]
  [grad_kappa_fission]
    type = VectorMooseVariable
    family = MONOMIAL_VEC
    order = CONSTANT
  []
[]

[AuxKernels]
  [comp_grad_kappa_fission]
    type = FDTallyGradAux
    variable = grad_kappa_fission
    score = 'kappa_fission'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0

  power = 100.0

  [Tallies]
    [Mesh]
      type = MeshTally
      score = 'kappa_fission'

      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = final
  exodus = true
  hide = 'temp cell_instance cell_id kappa_fission'
[]
