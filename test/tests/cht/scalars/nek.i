[Problem]
  type = NekRSProblem
  casename = 'sfr_pin'
  n_usrwrk_slots = 1

  # This input is run in nondimensional form to verify that all the postprocessors
  # and data transfers in/out of nekRS are properly dimensionalized.
  [Dimensionalize]
    U = 0.000950466
    s02 = 628.15
    ds02 = 50.0
    L = 0.908e-2
    transport_coeff_2 = ${fparse 834.5*1228.0}
  []

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
      postprocessor_to_conserve = flux_integral
      flux = scalar02_flux
    []
    [temp]
      type = NekFieldVariable
      field = scalar02
      direction = from_nek
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'

  # nekRS runs in non-dimensional form, which means that we shrunk the mesh
  # from physical units of meters to our characteristic scale of 0.908e-2 m
  # (the pin pitch, arbitrarily chosen). That means that we must multiply
  # the nekRS mesh by 0.908e-2 to get back in units of meters that BISON is
  # running in.
  scaling = 0.908e-2
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]

  [synchronization_in]
    type = Receiver
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
  hide = 'avg_flux'

  [screen]
    type = Console
    hide = 'synchronization_in'
  []
[]
