!include mesh.i

[Problem]
  type = FEProblem
[]

[Variables]
  [T]
    initial_condition = 293.6
  []
[]

[Kernels]
  [diffusion]
    type = HeatConduction
    variable = T
    block = '0'
  []
  [source]
    type = CoupledForce
    variable = T
    v = power
    block = '0'
  []
[]

[BCs]
  [left_boundary]
    type = DirichletBC
    variable = T
    boundary = 'left'
    value = 293.6
  []
  [insulated]
    type = NeumannBC
    variable = T
    boundary = 'right top bottom front back'
    value = 0
  []
[]


[Materials]
  [absorbing]
    type = HeatConductionMaterial
    thermal_conductivity = ${fparse 0.6 / 100.0} # W/cm-K
    block = '0'
  []
[]

[Postprocessors]
  [T_solid]
    type = ElementExtremeValue
    variable = T
  []
[]

[AuxVariables]
  [power]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
  []
[]


[Executioner]
  type = Transient
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-16
  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_hypre_type'
[]
