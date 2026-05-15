# MOOSE Thermomechanics model input file for the slab benchmark problem

T0 = 293 # K
L0 = 100 # cm
L = 106.47 # cm
P = 1.0e22 # eV/s
q = 1e8 # eV
k0 = 1.25e19 # eV/(s-cm-K^2) k(T) = k0 T(x)
phi0 = 2.5e14 # 1/s-cm^2 (flux at the origin)
eV_to_J = 1.602e-19 # J/eV
lam = ${fparse 0.5*(1+sqrt(1+(16*q*q*phi0*phi0)/(P*P)))} # eigenvalue solution
h = ${fparse 1/(sqrt(L*(lam-1)/(k0*P)) - (2*T0)/(P))*eV_to_J } # W/cm^2-K

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
    xmin = -50
    xmax = 50
  []
  [pin]
    type = ExtraNodesetGenerator
    input = generated
    new_boundary = pin
    coord = '0 0 0'
  []
[]

[GlobalParams]
  displacements = 'disp_x'
  use_displaced_mesh = true
[]

[Variables]
  [temperature]
  []
  [disp_x]
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [block1]
        strain = FINITE
        add_variables = true
        eigenstrain_names = eigenstrain
        generate_output = 'strain_xx'
        temperature = temperature
      []
    []
  []
[]

[ICs]
  [temperature]
    type = FunctionIC
    variable = temperature
    function = 293
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  [kappa_fission]
    type = CoupledForce
    variable = temperature
    v = kappa_fission
  []
  [dt]
    type = HeatConductionTimeDerivative
    variable = temperature
  []
[]

[AuxVariables]
  [kappa_fission]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = '${fparse P*eV_to_J/L0 }'
  []
  [temperature_praux]
    family = MONOMIAL
    order = CONSTANT
  []
  [disp_praux]
  []
[]

[AuxKernels]
   [temperature_projection]
     type = ProjectionAux
     variable = temperature_praux
     v = temperature
   []
   [disp_projection]
     type = ProjectionAux
     variable = disp_praux
     v = disp_x
   []
[]

[Materials]
  [thermal_parameters]
    type = HeatConductionMaterial
    temp = temperature
    thermal_conductivity_temperature_function = conductivity
    specific_heat = 0.3
  []
  [density]
    type = Density
    density = 20.0
    use_displaced_mesh = false
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
  [thermal_expansion_strain]
    type = ComputeMeanThermalExpansionFunctionEigenstrain
    stress_free_temperature = ${T0}
    thermal_expansion_function_reference_temperature = ${T0}
    thermal_expansion_function = cte_func_mean
    temperature = temperature
    eigenstrain_name = eigenstrain
  []
[]

[BCs]
  [sides_convective_BC]
    type = ConvectiveFluxFunction
    T_infinity = ${T0}
    variable = temperature
    boundary = 'left right'
    coefficient = ${h}
  []
  [pin_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'pin'
    value = 0.0
  []
[]

[Functions]
  [conductivity]
    type = ParsedFunction
    expression = '${fparse k0*eV_to_J} * t'
  []
  [cte_func_mean]
    type = ParsedFunction
    symbol_names = 'T0'
    symbol_values = '293'
    expression = '1 / (2*sqrt(T0*t))'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  verbose = true
  nl_abs_tol = 5e-6
  nl_rel_tol = 5e-6
  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  snesmf_reuse_base = false
  line_search = 'none'
  nl_max_its = 100
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e1
    optimal_iterations = 10
    growth_factor = 1.25
    cutback_factor = 0.8
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [source_integral]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    execute_on = 'transfer initial timestep_end'
  []
  [strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  []
  [total_length]
    type = FunctionElementIntegral
    function = 1
    execute_on = 'TIMESTEP_BEGIN LINEAR TIMESTEP_END'
  []
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]
