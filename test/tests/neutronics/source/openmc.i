r_pin = 0.39218
clad_ir = 0.40005
clad_or = 0.45720
L = 300.0

num_layers = 10

[Mesh]
  [clad] # This makes a circular annulus that will represent the clad
    type = AnnularMeshGenerator
    nr = 3
    nt = 20
    rmin = ${clad_ir}
    rmax = ${clad_or}
    quad_subdomain_id = 1
    tri_subdomain_id = 0
  []
  [extrude_clad] # this extrudes the circular annulus in the axial direction
    type = FancyExtruderGenerator
    input = clad
    heights = '${L}'
    num_layers = '${num_layers}'
    direction = '0 0 1'
  []
  [rename_clad] # this renames some sidesets on the clad to avoid name clashes
    type = RenameBoundaryGenerator
    input = extrude_clad
    old_boundary = '1 0' # outer surface, inner surface
    new_boundary = '5 4'
  []
  [fuel] # this makes a circle that will represent the fuel
    type = AnnularMeshGenerator
    nr = 10
    nt = 20
    rmin = 0
    rmax = ${r_pin}
    quad_subdomain_id = 2
    tri_subdomain_id = 3
    growth_r = -1.2
  []
  [extrude] # this extrudes the circle in the axial direction
    type = FancyExtruderGenerator
    input = fuel
    heights = '${L}'
    num_layers = '${num_layers}'
    direction = '0 0 1'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'rename_clad extrude'
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 600.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = ${fparse 3000e6 / 273 / (17 * 17)}
  solid_blocks = '1 2 3'
  tally_blocks = '2 3'
  tally_type = cell
  solid_cell_level = 0
  reuse_source = true
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
