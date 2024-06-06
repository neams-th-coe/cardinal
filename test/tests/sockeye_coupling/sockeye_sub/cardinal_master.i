[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [g]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [grid]
    type = CartesianGrid
    variable = g
    nx = 4
    ny = 4
    nz = 4
  []
[]

# We just want to check that Cardinal can run Sockeye as a sub-app.
# We omit all transfers just to check that the code executes.
[MultiApps]
  [sockeye]
    type = TransientMultiApp
    app_type = SockeyeApp
    input_files = 'flow_only_convection.i'
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]
