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

[Executioner]
  type = Transient
  num_steps = 2
[]

# We just want to check that Cardinal can run Bison as a sub-app.
# We omit all transfers just to check that the code executes.
[MultiApps]
  [bison]
    type = TransientMultiApp
    app_type = BisonApp
    input_files = 'bison_sub.i'
    execute_on = timestep_end
  []
[]

[Outputs]
  exodus = true
[]
