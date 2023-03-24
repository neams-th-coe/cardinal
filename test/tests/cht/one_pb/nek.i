[Mesh]
  type = NekRSMesh
  boundary = 4
[]

[Problem]
  type = NekRSProblem
  casename = 'pbb_cht'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  # This is the heat flux in the nekRS solution, i.e. it is not an integral
  # of nrs->usrwrk, instead this is directly an integral of k*grad(T)*hat(n).
  # So this should closely match 'flux_integral'

  [volume]
    type = NekVolumeIntegral
    field = unity
  []

  [flux_in_nek]
    type = NekHeatFluxIntegral
    boundary = '4'
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
  [average_nek_pebble_T]
    type = NekSideAverage
    boundary = '4'
    field = temperature
  []
[]