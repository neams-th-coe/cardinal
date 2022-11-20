[Mesh]
  [disc]
    type = AnnularMeshGenerator
    rmax = 0.5
    rmin = 0.0
    nr = 12
    nt = 24
    growth_r = 0.9
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = disc
    heights = '20.0'
    num_layers = '20'
    direction = '0 0 1'
  []
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [avg_velocity]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
