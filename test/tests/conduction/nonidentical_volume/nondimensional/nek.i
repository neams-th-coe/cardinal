[Problem]
  type = NekRSProblem
  casename = 'cylinder'

  nondimensional = true
  T_ref = 500.0
  dT_ref = 50.0
  rho_0 = 5.0
  Cp_0 = 6.0
  L_ref = 0.5
  U_ref = 1.0
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
