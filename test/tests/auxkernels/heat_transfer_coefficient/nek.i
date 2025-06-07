[Problem]
  type = NekRSProblem
  casename = 'sfr_pin'
  synchronization_interval = constant
  constant_interval = 100

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
      initial_flux_integral = 1000
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
[]

[UserObjects]
  [axial]
    type = LayeredBin
    num_layers = 5
    direction = z
  []
  [q]
    type = NekBinnedSideAverage
    field = usrwrk00
    bins = 'axial'
    boundary = '1'
    interval = 100
  []
  [Tw]
    type = NekBinnedSideAverage
    field = temperature
    bins = 'axial'
    boundary = '1'
    interval = 100
  []
  [Tinf]
    type = NekBinnedVolumeAverage
    field = temperature
    bins = 'axial'
    interval = 100
  []
[]

[AuxVariables]
  [h]
  []
[]

[AuxKernels]
  [h]
    type = HeatTransferCoefficientAux
    variable = h
    heat_flux = q
    wall_T = Tw
    bulk_T = Tinf
  []
[]

[ICs]
  [avg_flux]
    type = ConstantIC
    variable = avg_flux
    value = 10.0
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  # for the boundary mesh mirror, this is a boundary average integral
  [avg_h_uo]
    type = ElementAverageValue
    variable = h
  []

  [avg_wall_flux]
    type = NekSideAverage
    boundary = '1'
    field = usrwrk00
  []
  [avg_wall_T]
    type = NekSideAverage
    boundary = '1'
    field = temperature
  []
  [avg_bulk_T]
    type = NekVolumeAverage
    field = temperature
  []
  [avg_h]
    type = ParsedPostprocessor
    expression = 'avg_wall_flux/(avg_wall_T-avg_bulk_T)'
    pp_names = 'avg_wall_flux avg_wall_T avg_bulk_T'
  []
[]

[Outputs]
  csv = true
  time_step_interval = 100
  hide = 'avg_flux_integral'
[]
