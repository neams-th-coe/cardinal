[Mesh]
  type = NekRSMesh

  # This is the boundary we are coupling via conjugate heat transfer to MOOSE
  boundary = '3'
[]

[Problem]
  type = NekRSProblem
  casename = 'pebble'

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
    []
    [temp]
      type = NekFieldVariable
      direction = from_nek
      field = temperature
    []
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[UserObjects]
  [layered_bin]
    type = LayeredBin
    num_layers = 5
    direction = z
  []
  [wall_temp]
    type = NekBinnedSideAverage
    bins = 'layered_bin'
    boundary = '3'
    field = temperature
    map_space_by_qp = true
    interval = 10
  []
  [bulk_temp]
    type = NekBinnedVolumeAverage
    bins = 'layered_bin'
    field = temperature
    map_space_by_qp = true
    interval = 10
  []
[]

[VectorPostprocessors]
  [wall]
    type = SpatialUserObjectVectorPostprocessor
    userobject = wall_temp
  []
  [bulk]
    type = SpatialUserObjectVectorPostprocessor
    userobject = bulk_temp
  []
[]

[Outputs]
  exodus = true
  csv = true
  interval = 10
  hide = 'flux_integral'
[]
