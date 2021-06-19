# This input file controls the nekRS run (in addition to the usual nekRS input files).
#
# -- sideset 1: pellet surface
# -- sideset 2: duct inner surface
# -- sideset 3: inlet
# -- sideset 4: outlet

# Note that these sideset IDs are different from what you see in solid.i! This is
# because nekRS uses a completely separate mesh from MOOSE, and it so happens to have
# different sideset IDs.

[Mesh]
  # First, build a mirror of the nekRS mesh, but only on the boundaries that will be
  # communicating with MOOSE. Here, we want to send temperatures from nekRS and heat
  # flux into nekRS on the pin surfaces and duct inner surface.
  type = NekRSMesh
  boundary = '1 2'
[]

[Problem]
  type = NekRSProblem

  minimize_transfers_in = true
  transfer_in = synchronize
  minimize_transfers_out = true
[]

[Executioner]
  type = Transient

  [./TimeStepper]
    type = NekTimeStepper
  [../]
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]

[Postprocessors]
  [synchronize]
    type = Receiver
  []

  # This is the heat flux in the nekRS solution, i.e. it is not an integral
  # of nrs->usrwrk, instead this is directly an integral of k*grad(T)*hat(n).
  # So this should closely match 'flux_integral', a postprocessor that is
  # automatically added by NekRSProblem to receive the flux normalization
  [pin_flux_in_nek]
    type = NekHeatFluxIntegral
    boundary = '1'
  []
  [duct_flux_in_nek]
    type = NekHeatFluxIntegral
    boundary = '2'
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
