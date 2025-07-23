[Problem]
  type = NekRSProblem
  casename = 'brick'
  n_usrwrk_slots = 4

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      initial_flux_integral = 1
      usrwrk_slot = 1
      direction = to_nek
    []
    [heat_source]
      type = NekVolumetricSource
      initial_source_integral = 2
      usrwrk_slot = 2
      direction = to_nek
    []
  []

  [Dimensionalize]
    L = 5.0
    U = 0.2
    T = 10.0
    dT = 200.0
    rho = 1000
    Cp = 3000
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
  volume = true
  scaling = 5.0
[]

[ICs]
  [flux]
    type = FunctionIC
    variable = avg_flux
    function = f
  []
  [source]
    type = FunctionIC
    variable = heat_source
    function = s
  []
[]

[Functions]
  [f]
    type = ParsedFunction
    expression = '(100+200*x+300*y+400*z)/4e4'
  []
  [s]
    type = ParsedFunction
    expression = '(100+200*x+300*y+500*z)/2.937500e+05/1.106383e+00*2'
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [u01]
    type = NekPointValue
    field = usrwrk01
    point = '0.25 0.3 0.27'
  []
  [u02]
    type = NekPointValue
    field = usrwrk02
    point = '0.25 0.3 0.27'
  []
  [integral_flux]
    type = SideIntegralVariablePostprocessor
    variable = avg_flux
    boundary = '1'
  []
  [integral_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
[]

[Outputs]
  csv = true
  hide = 'avg_flux_integral heat_source_integral'
[]
