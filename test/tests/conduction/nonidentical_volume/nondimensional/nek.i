[Problem]
  type = NekRSProblem
  casename = 'cylinder'
  n_usrwrk_slots = 1

  [Dimensionalize]
    T = 500.0
    dT = 50.0
    rho = 5.0
    Cp = 6.0
    L = 0.5
    U = 1.0
  []

  [FieldTransfers]
    [heat_source]
      type = NekVolumetricSource
      usrwrk_slot = 0
      direction = to_nek
      postprocessor_to_conserve = source_integral
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
  volume = true

  # change to SECOND to exactly match the verification case in ../cylinder; we use
  # FIRST here just to reduce the size of the gold file
  order = FIRST

  # nekRS runs in nondimensional form, so we need to adjust the mesh (in nondimensional
  # coordinates) to the dimensional form expected by MOOSE
  scaling = 0.5
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
  [avg_T_volume]
    type = NekVolumeAverage
    field = temperature
  []
  [T_volume]
    type = NekVolumeIntegral
    field = temperature
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'

  # we can infer that these variables dont change by ensuring that 'temp' does not change,
  # since these other two variables are boundary conditions and source terms for the energy equation
  hide = 'heat_source'
[]
