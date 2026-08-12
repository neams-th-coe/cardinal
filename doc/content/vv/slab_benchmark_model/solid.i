T0 = ${units 293 K}
L0 = ${units 100 cm}
L = ${units 106.47 cm}
P = ${units 1.0e22 eV/s}
q = ${units 1e8 eV}
k0 = ${units 1.25e19 eV/s/cm/K^2} # k(T) = k0 T(x)
phi0 = ${units 2.5e14 1/s/cm^2} # (flux at the origin)
eV_to_J = ${units 1.602176634e-19 J/eV}
lam = '${fparse 0.5*(1+sqrt(1+(16*q*q*phi0*phi0)/(P*P)))}' # eigenvalue solution
h = '${fparse 1/(sqrt(L*(lam-1)/(k0*P)) - (2*T0)/(P))*eV_to_J }' # W/cm^2-K

E = ${units 1e6 N/m^2}
nu = 0.0
rho = ${units 20 g/cm^3}
c_p = ${units 0.3 J/g/K}

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 500
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

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
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
    function = ${T0}
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
    temperature = temperature
    thermal_conductivity_temperature_function = conductivity
    specific_heat = ${c_p}
  []
  [density]
    type = ParsedMaterial
    property_name = density
    expression = ${rho}
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = ${E}
    poissons_ratio = ${nu}
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
    expression = '1 / (2*sqrt(${T0}*t))'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  verbose = true
  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-7
  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  snesmf_reuse_base = false
  line_search = 'none'
  nl_max_its = 100
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-1
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

[Outputs]
  exodus = true
  csv = true
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
