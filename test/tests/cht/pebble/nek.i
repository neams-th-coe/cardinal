[Mesh]
  type = NekRSMesh
  boundary = '1'

  # nekRS solves with a length scale of meters, but nek_master.i is currently solving
  # in terms of centimeters. Therefore, just for the sake of data transfers, we need to
  # scale NekRSMesh to centimeters.
  scaling = 100.0
[]

[Problem]
  type = NekRSProblem
  casename = 'onepebble2'
  n_usrwrk_slots = 1

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      direction = to_nek
      postprocessor_to_conserve = flux_integral
      usrwrk_slot = 0
    []
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  # This is the heat flux in the nekRS solution, i.e. it is not an integral
  # of nrs->usrwrk, instead this is directly an integral of k*grad(T)*hat(n).
  # So this should closely match 'flux_integral'
  [flux_in_nek]
    type = NekHeatFluxIntegral
    boundary = '1'
  []

  [max_nek_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_nek_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
[]
