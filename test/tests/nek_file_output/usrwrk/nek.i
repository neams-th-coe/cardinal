[Mesh]
  type = NekRSMesh
  volume = true
  exact = true
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'
  usrwrk_output = '0 1'
  usrwrk_output_prefix = 'aaa ccc'
  n_usrwrk_slots = 2

  [FieldTransfers]
    [usrwrk00]
      type = NekVolumetricData
      direction = to_nek
      usrwrk_slot = 0
    []
    [usrwrk01]
      type = NekVolumetricData
      direction = to_nek
      usrwrk_slot = 1
    []
  []
[]

[AuxKernels]
  [usrwrk00]
    type = FunctionAux
    variable = usrwrk00
    function = usrwrk00
    execute_on = timestep_begin
  []
  [usrwrk01]
    type = FunctionAux
    variable = usrwrk01
    function = usrwrk01
    execute_on = timestep_begin
  []
[]

[Functions]
  [usrwrk00]
    type = ParsedFunction
    expression = 'z'
  []
  [usrwrk01]
    type = ParsedFunction
    expression = 'x'
  []
[]
[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
