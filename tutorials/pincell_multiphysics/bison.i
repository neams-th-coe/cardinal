inlet_T  = 573.0           # inlet temperature
power = 250                # total power (W)
Re = 500.0                 # Reynolds number

height = 0.5               # total height of the domain
Df = 0.825e-2              # fuel diameter
pin_diameter = 0.97e-2     # pin outer diameter
pin_pitch = 1.28e-2        # pin pitch

mu = 8.8e-5                # fluid dynamic viscosity
rho = 723.6                # fluid density
Cp = 5512.0                # fluid isobaric specific heat capacity

Rf = ${fparse Df / 2.0}

flow_area = ${fparse pin_pitch * pin_pitch - pi * pin_diameter * pin_diameter / 4.0}
wetted_perimeter = ${fparse pi * pin_diameter}
hydraulic_diameter = ${fparse 4.0 * flow_area / wetted_perimeter}

U_ref = ${fparse Re * mu / rho / hydraulic_diameter}
mdot = ${fparse rho * U_ref * flow_area}
dT = ${fparse power / mdot / Cp}

[Mesh]
  [solid]
    type = FileMeshGenerator
    file = solid_in.e
  []
[]

[Problem]
  type = FEProblem
  material_coverage_check = false
[]

[Variables]
  [T]
    initial_condition = ${inlet_T}
  []
[]

[Kernels]
  [diffusion]
    type = HeatConduction
    variable = T
    block = '2 3'
  []
  [source]
    type = CoupledForce
    variable = T
    v = power
    block = '2'
  []
  [null]
    type = NullKernel
    variable = T
    block = '1'
  []
[]

[BCs]
  [pin_outer]
    type = MatchedValueBC
    variable = T
    v = nek_temp
    boundary = '5'
  []
[]

[Materials]
  [uo2]
    type = HeatConductionMaterial
    thermal_conductivity = 5.0
    block = '2'
  []
  [clad]
    type = HeatConductionMaterial
    thermal_conductivity = 20.0
    block = '3'
  []
[]

[AuxVariables]
  [nek_temp]
  []
  [flux]
    family = MONOMIAL
    order = CONSTANT
  []
  [power]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = ${fparse power / (pi * Rf * Rf * height)}
  []
[]

[Functions]
  [axial_fluid_temp]
    type = ParsedFunction
    expression = '${inlet_T} + z / ${height} * ${dT}'
  []
[]

[ICs]
  [nek_temp]
    type = FunctionIC
    variable = nek_temp
    function = axial_fluid_temp
  []
[]

[AuxKernels]
  [flux]
    type = DiffusionFluxAux
    diffusion_variable = T
    component = normal
    diffusivity = thermal_conductivity
    variable = flux
    boundary = '5'
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Transfers]
  [temperature]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temperature
    from_multi_app = nek
    variable = nek_temp
  []
  [flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    to_multi_app = nek
    variable = flux
    from_boundaries = '5'
  []
  [flux_integral]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
    from_postprocessor = flux_integral
    to_multi_app = nek
  []
  [send]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = transfer_in
    from_postprocessor = send
    to_multi_app = nek
  []
[]

[Postprocessors]
  [send]
    type = Receiver
    default = 1.0
  []
  [flux_integral] # evaluate the total heat flux for normalization
    type = SideDiffusiveFluxIntegral
    diffusivity = thermal_conductivity
    variable = T
    boundary = '5'
  []
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = power
    block = '2'
    execute_on = 'transfer initial'
  []
  [max_T_solid]
    type = ElementExtremeValue
    variable = T
    block = '2 3'
  []
[]

t_nek = 2.5e-3
dt0 = ${fparse hydraulic_diameter / U_ref}
M = 25

[Executioner]
  type = Transient
  dt = ${fparse M * t_nek * dt0}
  num_steps = 100

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-16
  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_hypre_type'
[]

[Outputs]
  exodus = true
  hide = 'power flux_integral send'
[]
