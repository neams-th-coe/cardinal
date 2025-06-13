[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'
[]

[AuxVariables]
  [axial_bins]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [bins2]
    type = SpatialUserObjectAux
    variable = axial_bins
    user_object = axial_binning
  []
[]

[UserObjects]
  [axial_binning]
    type = LayeredBin
    direction = z
    num_layers = 6
  []
  [vol_avg]
    type = NekBinnedPlaneIntegral
    bins = 'axial_binning'
    field = unity
    gap_thickness = 0.01
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
