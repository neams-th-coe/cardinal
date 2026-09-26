!include openmc.i

[Mesh]
  uniform_refine = 1
[]

[Adaptivity/Markers/uniform]
  mark := 'COARSEN'
[]

[AuxVariables]
  [kappa_fission]
    family = MONOMIAL
    order = CONSTANT
  []
  [kappa_fission_raw]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [sum_kappa_fission]
    type = ParsedAux
    variable = kappa_fission
    expression = 'kappa_fission_g1 + kappa_fission_g2'
    coupled_variables = 'kappa_fission_g1 kappa_fission_g2'
  []
  [sum_kappa_fission_raw]
    type = ParsedAux
    variable = kappa_fission_raw
    expression = 'kappa_fission_g1_raw + kappa_fission_g2_raw'
    coupled_variables = 'kappa_fission_g1_raw kappa_fission_g2_raw'
  []
[]

[Problem]
  [Tallies/Mesh]
    filters = 'Energy'
  []
  [Filters/Energy]
    type = EnergyFilter
    energy_boundaries = '0.0 6.25e-1 2.0e8'
  []
[]

[Outputs]
  hide := 'temp uniform kappa_fission_g1_raw kappa_fission_g2_raw kappa_fission_g1 kappa_fission_g2'
[]
