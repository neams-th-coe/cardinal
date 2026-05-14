[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid1]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0

  power = 1.0

  [Tallies]
    [heating]
      type = CellTally
      output = 'unrelaxed_tally_rel_error'
      normalize_by_global_tally = false
      check_tally_sum = false
    []
  []
[]

[AuxVariables]
  [AMR_fom]
    order = CONSTANT
    family = MONOMIAL
  []
  [kappa_fission_init]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 1.0
  []
  [ll]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [AMR_fom]
    type = FoMAux
    variable = 'AMR_fom'
    fom_type = 'AMR'
    tally_value = 'kappa_fission'
    tally_value_init = 'kappa_fission_init'
    tally_rel_error = 'kappa_fission_rel_error'
    sim_time = 2.0
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [AMR_fom]
    type = PointValue
    point = '0 0 0'
    variable = AMR_fom
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
