# This problem solves a dummy heat conduction problem - f is an auxvariable that
# is simply set to the value of x, and average_f is the average of f in each pebble,
# with those values associated with points in space (which we set to the pebble centroids).
# We want to check that the sub-app receives the correct value in each pebble.

# Likewise, in the sub-app, g is just an auxvariable that is set to x*x, and average_g is
# the average in each pebble, with those values associated with points in space (which
# we set to the pebble centroids). We want to check that the master app receives the
# correct value in each pebble.

[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = sphere.e
  []
  [combiner]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 4 0 0
                 8 1 0'
  []
[]

[Variables]
  [temp]
    initial_condition = 300
  []
[]

[AuxVariables]
  [f]
  []
  [average_f_master]
    family = MONOMIAL
    order = CONSTANT
  []
  [average_g_master]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [f]
    type = FunctionIC
    variable = f
    function = f
  []
[]

[Functions]
  [f]
    type = ParsedFunction
    expression = 'x'
  []
[]

[Kernels]
  [hc]
    type = Diffusion
    variable = temp
  []
[]

[BCs]
  [outside]
    type = DirichletBC
    variable = temp
    boundary = '1'
    value = 300
  []
[]

[UserObjects]
  [./average_f_master] # this computes the average that well send to sub
    type = NearestPointAverage
    variable = f
    points = '0 0 0
              4 0 0
              8 1 0'
    execute_on = 'initial timestep_end'
  []
  [average_g_master] # this receives from the transfer from sub
    type = NearestPointReceiver
    positions = '0 0 0
                 4 0 0
                 8 1 0'
  []
[]

[AuxKernels]
  # These auxkernels are strictly needed to visualize the results of the userobject
  # transfer - they are not part of the transfer.
  [average_f_master]
    variable = average_f_master
    type = SpatialUserObjectAux
    user_object = average_f_master
    execute_on = 'initial timestep_end'
  []
  [average_g_master]
    type = SpatialUserObjectAux
    variable = average_g_master
    user_object = average_g_master
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  dt = 0.1
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'sub.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [./average_f_to_sub]
    type = NearestPointReceiverTransfer
    to_multi_app = sub
    from_uo = average_f_master
    to_uo = average_f_sub
  []
  [./average_g_from_sub]
    type = NearestPointReceiverTransfer
    from_multi_app = sub
    from_uo = average_g_sub
    to_uo = average_g_master
  []
[]
