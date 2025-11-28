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
[]

[AuxVariables]
  [total_xs_g1]
    type = MooseVariable
    order = CONSTANT
    family = MONOMIAL
  []
  [zero_flux]
    type = MooseVariable
    order = CONSTANT
    family = MONOMIAL
    initial_condition = '0.0'
  []
[]

[AuxKernels]
  [comp_total_g1]
    type = ComputeMGXSAux
    variable = total_xs_g1
    rxn_rates = 'total_g1'
    normalize_by = 'flux_g1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  initial_properties = xml
  verbose = true
  cell_level = 0

  power = 1.0
  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Cell_KF]
      type = CellTally
      score = 'kappa_fission'
      block = '100'

      normalize_by_global_tally = false
    []
    [Cell_Total_Flux]
      type = CellTally
      score = 'total flux'
      block = '100'
      filters = 'Energy'

      normalize_by_global_tally = false
    []
  []
  [Filters]
    [Energy]
      type = EnergyFilter
      energy_boundaries = '0.0 6.25e-1 2.0e7'
      reverse_bins = true
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [result]
    type = PointValue
    point = '0 0 0'
    variable = total_xs_g1
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
