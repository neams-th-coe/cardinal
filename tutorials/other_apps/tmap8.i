node_num = 200
thickness = '${units 1 m}'
end_time = '${units 10 s}'
temperature = '${units 1000 K}'
diffusivity = '${units 1 m^2/s}'

density = '${units 3.1622e22 at/m^3}'
cl = '${units 3.1622e18 at/m^3}'
trapping_prefactor = '${units 1e15 1/s}'
release_prefactor = '${units 1e13 1/s}'
release_energy = '${units 100 K}'
trapping_fraction = 0.1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = ${node_num}
  xmax = ${thickness}
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
  library_path = '/opt/tmap8/lib/tmap8'
[]

[Variables]
  [mobile]
  []
  [trapped]
  []
[]

[AuxVariables]
  [tritium_source]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = mobile
    diffusivity = ${diffusivity}
    extra_vector_tags = ref
  []
  [time]
    type = TimeDerivative
    variable = mobile
    extra_vector_tags = ref
  []
  [coupled_time]
    type = CoupledTimeDerivative
    variable = mobile
    v = trapped
    extra_vector_tags = ref
  []
  [source]
    type = CoupledForce
    variable = mobile
    v = tritium_source
    coef = '${fparse 1 / cl}'
    extra_vector_tags = ref
  []
[]

[NodalKernels]
  [time]
    type = TimeDerivativeNodalKernel
    variable = trapped
  []
  [trapping]
    type = TrappingNodalKernel
    variable = trapped
    alpha_t = ${trapping_prefactor}
    N = '${fparse density / cl}'
    Ct0 = ${trapping_fraction}
    mobile_concentration = 'mobile'
    temperature = ${temperature}
    extra_vector_tags = ref
  []
  [release]
    type = ReleasingNodalKernel
    alpha_r = ${release_prefactor}
    temperature = ${temperature}
    detrapping_energy = ${release_energy}
    variable = trapped
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = mobile
    value = 0
    boundary = left
  []
  [right]
    type = DirichletBC
    variable = mobile
    value = 0
    boundary = right
  []
[]

[Postprocessors]
  [outflux_left]
    type = SideDiffusiveFluxAverage
    boundary = left
    diffusivity = ${diffusivity}
    variable = mobile
    outputs = none
  []
  [outflux_right]
    type = SideDiffusiveFluxAverage
    boundary = right
    diffusivity = ${diffusivity}
    variable = mobile
    outputs = none
  []
  [release_left]
    type = ScalePostprocessor
    value = outflux_left
    scaling_factor = ${cl}
  []
  [release_right]
    type = ScalePostprocessor
    value = outflux_right
    scaling_factor = ${cl}
  []
  [production]
    type = ElementIntegralVariablePostprocessor
    variable = tritium_source
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  end_time = ${end_time}
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  compute_scaling_once = false

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01
    optimal_iterations = 6
    growth_factor = 1.2
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
