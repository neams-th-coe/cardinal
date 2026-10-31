[Problem]
  type = NekRSProblem
  casename = 'brick'
  n_usrwrk_slots = 4

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = to_nek
      mesh = 'stl'

      initial_flux_integral_stl = 1.5
      usrwrk_slot = 0
    []
    [usrwrk01]
      type = NekVolumetricData
      direction = to_nek
      usrwrk_slot = 1
    []
    [usrwrk02]
      type = NekVolumetricData
      direction = to_nek
      usrwrk_slot = 2
    []
  []
[]

[ICs]
  [flux]
    type = FunctionIC
    variable = flux
    block = 9999
    function = f
  []
  [usrwrk01]
    type = ConstantIC
    variable = usrwrk01
    value = 7.5
  []
  [usrwrk02]
    type = ConstantIC
    variable = usrwrk02
    value = 1.0
  []
[]

[Functions]
  [f]
    type = ParsedFunction
    expression = 'x+y+z'
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
  stl = "../../nek_mesh/stl/small_sphere.stl"
  stl_translation = '0.5 0.5 0.5'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [stl_integral_0] # should be equal to 1.5
    type = NekUsrWrkSTLIntegral
    usrwrk_slot = 0
  []
  [stl_integral_1] # should be close to 7.5 times stl_area
    type = NekUsrWrkSTLIntegral
    usrwrk_slot = 1
  []
  [stl_area] # should be close to pi, since this is the STL surface area when scaled
    type = NekUsrWrkSTLIntegral
    usrwrk_slot = 2
  []
[]

[Outputs]
  csv = true
[]

