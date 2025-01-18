[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../meshes/sphere.e
  []

  allow_renumbering = false
  parallel_type = replicated
[]

[AuxVariables]
  [grad_kappa_fission]
    type = VectorMooseVariable
    family = MONOMIAL_VEC
    order = CONSTANT
  []
[]

[AuxKernels]
  [comp_grad_kappa_fission]
    type = FDTallyGradAux
    variable = grad_kappa_fission
    score = 'kappa_fission'
  []
[]

[Problem]
  type = FEProblem
[]

[Executioner]
  type = Steady
[]
