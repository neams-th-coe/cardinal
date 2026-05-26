[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
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
  [vr_fom]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [vr_fom]
    type = FoMAux
    variable = 'vr_fom'
    fom_type = 'VR'
    tally_rel_error = 'kappa_fission_rel_error'
    # A constant value is used as a proxy for simulation time to
    # ensure the test results match the gold file.
    sim_time = 2.0
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [vr_fom]
    type = PointValue
    point = '0 0 0'
    variable = vr_fom
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
