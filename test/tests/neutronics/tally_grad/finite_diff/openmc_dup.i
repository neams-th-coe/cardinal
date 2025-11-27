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
  [mag_grad_kappa_fission]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [comp_grad_kappa_fission]
    type = FDTallyGradAux
    variable = grad_kappa_fission
    score = 'kappa_fission'
  []
  [mag_grad_kappa_fission]
    type = VectorVariableMagnitudeAux
    variable = mag_grad_kappa_fission
    vector_variable = grad_kappa_fission
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0

  power = 100.0
  source_rate_normalization = 'kappa_fission'
  normalization_tally = 'Cell'

  [Tallies]
    [Mesh]
      type = MeshTally
      score = 'kappa_fission'
      name = 'mesh_kf'

      normalize_by_global_tally = false
    []
    [Cell]
      type = CellTally
      score = 'kappa_fission'
      name = 'cell_kf'

      normalize_by_global_tally = false
    []
    [Cell_Flux]
      type = CellTally
      score = 'flux'
      name = 'cell_flux'

      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [max_grad]
    type = ElementExtremeValue
    variable = mag_grad_kappa_fission
    value_type = max
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
