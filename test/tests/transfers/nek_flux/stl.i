[Problem]
  type = NekRSProblem
  casename = 'brick'

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = from_nek
      mesh = 'stl'
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6'
  stl = "../../nek_mesh/stl/small_sphere.stl"
  stl_translation = '0.5 0.5 0.5'
[]

[AuxVariables]
  [moose_flux]
    block = 9999
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [moose_flux_integral]
    type = ElementIntegralVariablePostprocessor
    variable = moose_flux
    block = 9999
  []
  [volume]
    type = VolumePostprocessor
    block = 9999
  []

  [max_nek_flux]
    type = ElementExtremeValue
    variable = flux
    block = 9999
  []
  [min_nek_flux]
    type = ElementExtremeValue
    variable = flux
    block = 9999
    value_type = min
  []

  [max_moose_flux]
    type = ElementExtremeValue
    variable = moose_flux
    block = 9999
  []
  [min_moose_flux]
    type = ElementExtremeValue
    variable = moose_flux
    block = 9999
    value_type = min
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

