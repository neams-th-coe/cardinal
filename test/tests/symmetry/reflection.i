[Mesh]
  # using a finer mesh will more clearly show the symmetry plane,
  # but we use coarser here to have a smaller gold file
  [sphere]
    type = FileMeshGenerator
    file = ../neutronics/meshes/sphere.e
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []
[]

[AuxVariables]
  [x]
    family = MONOMIAL
    order = CONSTANT
  []
  [y]
    family = MONOMIAL
    order = CONSTANT
  []
  [z]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [x]
    type = PointTransformationAux
    variable = x
    component = x
  []
  [y]
    type = PointTransformationAux
    variable = y
    component = y
  []
  [z]
    type = PointTransformationAux
    variable = z
    component = z
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  symmetry_mapper = sym
[]

[UserObjects]
  [sym]
    type = SymmetryPointGenerator
    normal = '1.0 1.0 1.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
