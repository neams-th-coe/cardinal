# The test compares the wall shear data from Dimopoulos & Hanratty, 1968, J.Fluid.Mech
# vs. wall shear from Cardinal. The test is coarse-grid and has a short run time to keep it lightweight.
# It reproduces the experimental data well. When the simulation is run for a few flow-throughs with a finer mesh,
# averaging the shear data after the vortex shedding starts (~100 non-dimensional time units) further
# improves agreement with the experimental data.

# The wall shear data was chosen from the Re=104 experiment, and non-dimensionalized with the dynamic pressure
# i.e density * U * U

[Mesh]
  type = NekRSMesh
  order = FIRST
  volume = true
  exact = true
  parallel_type = replicated
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'cylinder'
  output = 'wall_shear'
  synchronization_interval = constant
[]

[Outputs]
  exodus = false
  csv = true
  execute_on = 'FINAL'
  show = "avg_theta01 avg_theta02 avg_theta03 avg_theta04 avg_theta05 avg_theta06 avg_theta07 "
         "avg_theta08 avg_theta09 avg_theta10 avg_theta11 avg_theta12 avg_theta13 avg_theta14 "
         "avg_theta15 avg_theta16 avg_theta17 avg_theta18 avg_theta19 avg_theta20 avg_theta21 "
         "avg_theta22 avg_theta23 avg_theta24 avg_theta25 avg_theta26 avg_theta27 avg_theta28 "
         "avg_theta29 avg_theta30 avg_theta31 avg_theta32 avg_theta33 avg_theta34 avg_theta35 "
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

# the postprocessors were generated for each of the 35 experimental data points
# using a python script

[Postprocessors]

  [sim_theta01]
    type = PointValue
    point = '-0.498097349 0.043577871 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta01]
    type = TimeIntegratedPostprocessor
    value = sim_theta01
    execution_order_group = 1
  []
  [avg_theta01]
    type = ParsedPostprocessor
    expression = 'int_theta01/t'
    pp_names = 'int_theta01'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta02]
    type = PointValue
    point = '-0.492403877 0.086824089 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta02]
    type = TimeIntegratedPostprocessor
    value = sim_theta02
    execution_order_group = 1
  []
  [avg_theta02]
    type = ParsedPostprocessor
    expression = 'int_theta02/t'
    pp_names = 'int_theta02'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta03]
    type = PointValue
    point = '-0.482962913 0.129409523 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta03]
    type = TimeIntegratedPostprocessor
    value = sim_theta03
    execution_order_group = 1
  []
  [avg_theta03]
    type = ParsedPostprocessor
    expression = 'int_theta03/t'
    pp_names = 'int_theta03'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta04]
    type = PointValue
    point = '-0.46984631 0.171010072 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta04]
    type = TimeIntegratedPostprocessor
    value = sim_theta04
    execution_order_group = 1
  []
  [avg_theta04]
    type = ParsedPostprocessor
    expression = 'int_theta04/t'
    pp_names = 'int_theta04'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta05]
    type = PointValue
    point = '-0.453153894 0.211309131 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta05]
    type = TimeIntegratedPostprocessor
    value = sim_theta05
    execution_order_group = 1
  []
  [avg_theta05]
    type = ParsedPostprocessor
    expression = 'int_theta05/t'
    pp_names = 'int_theta05'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta06]
    type = PointValue
    point = '-0.433012702 0.25 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta06]
    type = TimeIntegratedPostprocessor
    value = sim_theta06
    execution_order_group = 1
  []
  [avg_theta06]
    type = ParsedPostprocessor
    expression = 'int_theta06/t'
    pp_names = 'int_theta06'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta07]
    type = PointValue
    point = '-0.409576022 0.286788218 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta07]
    type = TimeIntegratedPostprocessor
    value = sim_theta07
    execution_order_group = 1
  []
  [avg_theta07]
    type = ParsedPostprocessor
    expression = 'int_theta07/t'
    pp_names = 'int_theta07'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta08]
    type = PointValue
    point = '-0.383022222 0.321393805 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta08]
    type = TimeIntegratedPostprocessor
    value = sim_theta08
    execution_order_group = 1
  []
  [avg_theta08]
    type = ParsedPostprocessor
    expression = 'int_theta08/t'
    pp_names = 'int_theta08'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta09]
    type = PointValue
    point = '-0.353553391 0.353553391 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta09]
    type = TimeIntegratedPostprocessor
    value = sim_theta09
    execution_order_group = 1
  []
  [avg_theta09]
    type = ParsedPostprocessor
    expression = 'int_theta09/t'
    pp_names = 'int_theta09'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta10]
    type = PointValue
    point = '-0.321393805 0.383022222 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta10]
    type = TimeIntegratedPostprocessor
    value = sim_theta10
    execution_order_group = 1
  []
  [avg_theta10]
    type = ParsedPostprocessor
    expression = 'int_theta10/t'
    pp_names = 'int_theta10'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta11]
    type = PointValue
    point = '-0.25 0.433012702 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta11]
    type = TimeIntegratedPostprocessor
    value = sim_theta11
    execution_order_group = 1
  []
  [avg_theta11]
    type = ParsedPostprocessor
    expression = 'int_theta11/t'
    pp_names = 'int_theta11'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta12]
    type = PointValue
    point = '-0.211309131 0.453153894 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta12]
    type = TimeIntegratedPostprocessor
    value = sim_theta12
    execution_order_group = 1
  []
  [avg_theta12]
    type = ParsedPostprocessor
    expression = 'int_theta12/t'
    pp_names = 'int_theta12'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta13]
    type = PointValue
    point = '-0.171010072 0.46984631 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta13]
    type = TimeIntegratedPostprocessor
    value = sim_theta13
    execution_order_group = 1
  []
  [avg_theta13]
    type = ParsedPostprocessor
    expression = 'int_theta13/t'
    pp_names = 'int_theta13'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta14]
    type = PointValue
    point = '-0.129409523 0.482962913 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta14]
    type = TimeIntegratedPostprocessor
    value = sim_theta14
    execution_order_group = 1
  []
  [avg_theta14]
    type = ParsedPostprocessor
    expression = 'int_theta14/t'
    pp_names = 'int_theta14'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta15]
    type = PointValue
    point = '-0.086824089 0.492403877 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta15]
    type = TimeIntegratedPostprocessor
    value = sim_theta15
    execution_order_group = 1
  []
  [avg_theta15]
    type = ParsedPostprocessor
    expression = 'int_theta15/t'
    pp_names = 'int_theta15'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta16]
    type = PointValue
    point = '-0.043577871 0.498097349 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta16]
    type = TimeIntegratedPostprocessor
    value = sim_theta16
    execution_order_group = 1
  []
  [avg_theta16]
    type = ParsedPostprocessor
    expression = 'int_theta16/t'
    pp_names = 'int_theta16'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta17]
    type = PointValue
    point = '-3.06162e-17 0.5 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta17]
    type = TimeIntegratedPostprocessor
    value = sim_theta17
    execution_order_group = 1
  []
  [avg_theta17]
    type = ParsedPostprocessor
    expression = 'int_theta17/t'
    pp_names = 'int_theta17'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta18]
    type = PointValue
    point = '0.043577871 0.498097349 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta18]
    type = TimeIntegratedPostprocessor
    value = sim_theta18
    execution_order_group = 1
  []
  [avg_theta18]
    type = ParsedPostprocessor
    expression = 'int_theta18/t'
    pp_names = 'int_theta18'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta19]
    type = PointValue
    point = '0.086824089 0.492403877 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta19]
    type = TimeIntegratedPostprocessor
    value = sim_theta19
    execution_order_group = 1
  []
  [avg_theta19]
    type = ParsedPostprocessor
    expression = 'int_theta19/t'
    pp_names = 'int_theta19'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta20]
    type = PointValue
    point = '0.129409523 0.482962913 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta20]
    type = TimeIntegratedPostprocessor
    value = sim_theta20
    execution_order_group = 1
  []
  [avg_theta20]
    type = ParsedPostprocessor
    expression = 'int_theta20/t'
    pp_names = 'int_theta20'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta21]
    type = PointValue
    point = '0.171010072 0.46984631 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta21]
    type = TimeIntegratedPostprocessor
    value = sim_theta21
    execution_order_group = 1
  []
  [avg_theta21]
    type = ParsedPostprocessor
    expression = 'int_theta21/t'
    pp_names = 'int_theta21'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta22]
    type = PointValue
    point = '0.211309131 0.453153894 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta22]
    type = TimeIntegratedPostprocessor
    value = sim_theta22
    execution_order_group = 1
  []
  [avg_theta22]
    type = ParsedPostprocessor
    expression = 'int_theta22/t'
    pp_names = 'int_theta22'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta23]
    type = PointValue
    point = '0.25 0.433012702 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta23]
    type = TimeIntegratedPostprocessor
    value = sim_theta23
    execution_order_group = 1
  []
  [avg_theta23]
    type = ParsedPostprocessor
    expression = 'int_theta23/t'
    pp_names = 'int_theta23'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta24]
    type = PointValue
    point = '0.286788218 0.409576022 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta24]
    type = TimeIntegratedPostprocessor
    value = sim_theta24
    execution_order_group = 1
  []
  [avg_theta24]
    type = ParsedPostprocessor
    expression = 'int_theta24/t'
    pp_names = 'int_theta24'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta25]
    type = PointValue
    point = '0.321393805 0.383022222 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta25]
    type = TimeIntegratedPostprocessor
    value = sim_theta25
    execution_order_group = 1
  []
  [avg_theta25]
    type = ParsedPostprocessor
    expression = 'int_theta25/t'
    pp_names = 'int_theta25'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta26]
    type = PointValue
    point = '0.353553391 0.353553391 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta26]
    type = TimeIntegratedPostprocessor
    value = sim_theta26
    execution_order_group = 1
  []
  [avg_theta26]
    type = ParsedPostprocessor
    expression = 'int_theta26/t'
    pp_names = 'int_theta26'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta27]
    type = PointValue
    point = '0.383022222 0.321393805 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta27]
    type = TimeIntegratedPostprocessor
    value = sim_theta27
    execution_order_group = 1
  []
  [avg_theta27]
    type = ParsedPostprocessor
    expression = 'int_theta27/t'
    pp_names = 'int_theta27'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta28]
    type = PointValue
    point = '0.409576022 0.286788218 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta28]
    type = TimeIntegratedPostprocessor
    value = sim_theta28
    execution_order_group = 1
  []
  [avg_theta28]
    type = ParsedPostprocessor
    expression = 'int_theta28/t'
    pp_names = 'int_theta28'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta29]
    type = PointValue
    point = '0.433012702 0.25 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta29]
    type = TimeIntegratedPostprocessor
    value = sim_theta29
    execution_order_group = 1
  []
  [avg_theta29]
    type = ParsedPostprocessor
    expression = 'int_theta29/t'
    pp_names = 'int_theta29'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta30]
    type = PointValue
    point = '0.453153894 0.211309131 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta30]
    type = TimeIntegratedPostprocessor
    value = sim_theta30
    execution_order_group = 1
  []
  [avg_theta30]
    type = ParsedPostprocessor
    expression = 'int_theta30/t'
    pp_names = 'int_theta30'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta31]
    type = PointValue
    point = '0.46984631 0.171010072 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta31]
    type = TimeIntegratedPostprocessor
    value = sim_theta31
    execution_order_group = 1
  []
  [avg_theta31]
    type = ParsedPostprocessor
    expression = 'int_theta31/t'
    pp_names = 'int_theta31'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta32]
    type = PointValue
    point = '0.482962913 0.129409523 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta32]
    type = TimeIntegratedPostprocessor
    value = sim_theta32
    execution_order_group = 1
  []
  [avg_theta32]
    type = ParsedPostprocessor
    expression = 'int_theta32/t'
    pp_names = 'int_theta32'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta33]
    type = PointValue
    point = '0.492403877 0.086824089 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta33]
    type = TimeIntegratedPostprocessor
    value = sim_theta33
    execution_order_group = 1
  []
  [avg_theta33]
    type = ParsedPostprocessor
    expression = 'int_theta33/t'
    pp_names = 'int_theta33'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta34]
    type = PointValue
    point = '0.498097349 0.043577871 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta34]
    type = TimeIntegratedPostprocessor
    value = sim_theta34
    execution_order_group = 1
  []
  [avg_theta34]
    type = ParsedPostprocessor
    expression = 'int_theta34/t'
    pp_names = 'int_theta34'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

  [sim_theta35]
    type = PointValue
    point = '0.5 6.12323e-17 0.5'
    variable = wall_shear
    execution_order_group = 0
  []
  [int_theta35]
    type = TimeIntegratedPostprocessor
    value = sim_theta35
    execution_order_group = 1
  []
  [avg_theta35]
    type = ParsedPostprocessor
    expression = 'int_theta35/t'
    pp_names = 'int_theta35'
    execute_on = 'FINAL'
    use_t = true
    execution_order_group = 2
  []

[]
