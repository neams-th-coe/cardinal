mu = 20
area_side1 = ${fparse 2*.05*2*0.15}
area_side3 = ${fparse 2*0.1*2*0.15}
area_side5 = ${fparse 2*.05*2*0.1}

[Problem]
  type = NekRSProblem
  casename = 'brick'
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
[]

# The compare_... postprocessors compute the components of the drag analytically
# given the imposed velocity in the udf file
[Postprocessors]
  [viscous_total_1]
    type = NekViscousSurfaceForce
    boundary = '1'
    mesh = 'fluid'
    component = 'total'
  []
  [viscous_x_1]
    type = NekViscousSurfaceForce
    boundary = '1'
    mesh = 'fluid'
    component = 'x'
  []
  [viscous_y_1]
    type = NekViscousSurfaceForce
    boundary = '1'
    mesh = 'fluid'
    component = 'y'
  []
  [viscous_z_1]
    type = NekViscousSurfaceForce
    boundary = '1'
    mesh = 'fluid'
    component = 'z'
  []
  [compare_x_1]
    type = ConstantPostprocessor
    value = '${fparse area_side1*mu*(1+2)}'
  []
  [compare_y_1]
    type = ConstantPostprocessor
    value = '${fparse area_side1*mu*(2+2)}'
  []
  [compare_z_1]
    type = ConstantPostprocessor
    value = '${fparse area_side1*mu*(3+2)}'
  []
  [compare_total_1]
    type = ParsedPostprocessor
    expression = 'sqrt(compare_x_1*compare_x_1+compare_y_1*compare_y_1+compare_z_1*compare_z_1)'
    pp_names = 'compare_x_1 compare_y_1 compare_z_1'
  []

  [viscous_total_3]
    type = NekViscousSurfaceForce
    boundary = '3'
    mesh = 'fluid'
    component = 'total'
  []
  [viscous_x_3]
    type = NekViscousSurfaceForce
    boundary = '3'
    mesh = 'fluid'
    component = 'x'
  []
  [viscous_y_3]
    type = NekViscousSurfaceForce
    boundary = '3'
    mesh = 'fluid'
    component = 'y'
  []
  [viscous_z_3]
    type = NekViscousSurfaceForce
    boundary = '3'
    mesh = 'fluid'
    component = 'z'
  []
  [compare_x_3]
    type = ConstantPostprocessor
    value = '${fparse area_side3*mu*(1+1)}'
  []
  [compare_y_3]
    type = ConstantPostprocessor
    value = '${fparse area_side3*mu*(2+1)}'
  []
  [compare_z_3]
    type = ConstantPostprocessor
    value = '${fparse area_side3*mu*(3+1)}'
  []
  [compare_total_3]
    type = ParsedPostprocessor
    expression = 'sqrt(compare_x_3*compare_x_3+compare_y_3*compare_y_3+compare_z_3*compare_z_3)'
    pp_names = 'compare_x_3 compare_y_3 compare_z_3'
  []

  [viscous_total_5]
    type = NekViscousSurfaceForce
    boundary = '5'
    mesh = 'fluid'
    component = 'total'
  []
  [viscous_x_5]
    type = NekViscousSurfaceForce
    boundary = '5'
    mesh = 'fluid'
    component = 'x'
  []
  [viscous_y_5]
    type = NekViscousSurfaceForce
    boundary = '5'
    mesh = 'fluid'
    component = 'y'
  []
  [viscous_z_5]
    type = NekViscousSurfaceForce
    boundary = '5'
    mesh = 'fluid'
    component = 'z'
  []
  [compare_x_5]
    type = ConstantPostprocessor
    value = '${fparse -area_side5*mu*(1+3)}'
  []
  [compare_y_5]
    type = ConstantPostprocessor
    value = '${fparse -area_side5*mu*(2+3)}'
  []
  [compare_z_5]
    type = ConstantPostprocessor
    value = '${fparse -area_side5*mu*(3+3)}'
  []
  [compare_total_5]
    type = ParsedPostprocessor
    expression = 'sqrt(compare_x_5*compare_x_5+compare_y_5*compare_y_5+compare_z_5*compare_z_5)'
    pp_names = 'compare_x_5 compare_y_5 compare_z_5'
  []
[]
