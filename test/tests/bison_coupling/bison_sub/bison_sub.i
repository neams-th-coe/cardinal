#This is a test of the Radius auxkernel on an axisymmetric 2D model.
#This test consisists of a mechanical model of a ring subjected to
#an internal pressure that causes a radial expansion of the ring.
#The test verifies that the radius is correctly computed for both the
#undeformed state and the deformed state of the ring. In the undeformed
#state (step 0), the radius is the distance of the nodes from the
#center of rotation. In the deformed state, it is the distance of the
#displaced nodes from the center of rotation.

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
[]

[Mesh]
  coord_type = RZ
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = 1.0
    xmax = 1.1
    ymin = 0.0
    ymax = 0.1
    elem_type = QUAD4
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [radius]
  []
[]

[Functions]
  [pressure_history]
    type = PiecewiseLinear
    xy_data = '0 0
               1 10000'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = false
    strain = SMALL
    incremental = true
  []
[]

[AuxKernels]
  [radius]
    type = Radius
    variable = radius
    point1 = '0 0 0'
    point2 = '0 1 0'
    execute_on = 'initial timestep_end'
  []
[]

[BCs]
  [bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0
  []
  [Pressure]
    [pressure]
      boundary = left
      function = pressure_history
    []
  []
[]

[Materials]
  [stiffStuff]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_rel_tol = 1e-12
  end_time = 1
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
  []
[]

[Outputs]
  exodus = true
  [console]
    type = Console
    output_linear = true
  []
[]
