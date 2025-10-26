[Problem]
  type = NekRSProblem
  casename = 'sfr_pin'
  n_usrwrk_slots = 1

  # This input is run in nondimensional form to verify that all the postprocessors
  # and data transfers in/out of nekRS are properly dimensionalized.
  [Dimensionalize]
    U = 0.0950466
    T = 628.15
    dT = 50.0
    L = 0.908e-2
    rho = 834.5
    Cp = 1228.0
  []

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
      postprocessor_to_conserve = flux_integral
    []
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
  exact = true

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
  [screen]
    type = Console
    hide = 'synchronization_in'
  []
[]
