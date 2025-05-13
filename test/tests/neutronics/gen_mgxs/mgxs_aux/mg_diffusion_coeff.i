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
  [diff_g1]
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
  [comp_diff_g1]
    type = ComputeDiffusionCoeffMGAux
    variable = diff_g1
    total_rxn_rate = 'total_g1'
    p1_scatter_rxn_rates = 'nu_scatter_g1_gp1_l1 nu_scatter_g2_gp1_l1'
    scalar_flux = 'flux_g1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  initial_properties = xml
  verbose = true
  cell_level = 0
  normalize_by_global_tally = false

  power = 1.0
  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Cell_KF]
      type = CellTally
      score = 'kappa_fission'
      block = '100'
    []
    [Cell_Total_Flux]
      type = CellTally
      score = 'total flux'
      block = '100'
      filters = 'Energy'
    []
    [Cell_Scatter]
      type = CellTally
      score = 'nu_scatter'
      block = '100'
      filters = 'Energy EnergyOut Legendre'
      estimator = 'analog'
    []
  []
  [Filters]
    [Energy]
      type = EnergyFilter
      energy_boundaries = '0.0 6.25e-1 2.0e7'
      reverse_bins = true
    []
    [EnergyOut]
      type = EnergyOutFilter
      energy_boundaries = '0.0 6.25e-1 2.0e7'
      reverse_bins = true
    []
    [Legendre]
      type = AngularLegendreFilter
      order = 1
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
    variable = diff_g1
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
