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
  [average_f_sub]
  []
  [g]
  []
  [average_g_sub]
    family = MONOMIAL
    order = CONSTANT
  []
  [rank]
  []
[]

[ICs]
  [g]
    type = FunctionIC
    variable = g
    function = g
  []
[]

[Functions]
  [g]
    type = ParsedFunction
    expression = x*x
  []
[]

[AuxKernels]
  # These auxkernels are strictly needed to visualize the results of the userobject
  # transfer - they are not part of the transfer.
  [average_f_sub]
    type = SpatialUserObjectAux
    variable = average_f_sub
    user_object = average_f_sub
  []
  [average_g_sub]
    type = SpatialUserObjectAux
    variable = average_g_sub
    user_object = average_g_sub
  []
  [rank]
    type = ProcessorIDAux
    variable = rank
  []
[]

[UserObjects]
  [average_f_sub] # this receives from the transfer
    type = NearestPointReceiver
    positions = '0 0 0
                 4 0 0
                 8 1 0'
  []
  [./average_g_sub] # This computes the average that will be transferred to master
    type = NearestPointAverage
    variable = g
    points = '0 0 0
              4 0 0
              8 1 0'
    execute_on = 'initial timestep_end'
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

[Executioner]
  type = Transient
  nl_abs_tol = 1e-6
[]

[Outputs]
  exodus = true
  hide = 'rank'
[]
