r_pin = 0.39218
clad_ir = 0.40005
clad_or = 0.45720
L = 300.0

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
    type = AdvancedExtruderGenerator
    input = clad
    heights = '${L}'
    num_layers = '40'
    direction = '0 0 1'
  []

  # A sideset in MOOSE is both an ID and a name.
  # This renames the sideset numbers on the clad inner surface (0) and outer surface (1)
  # to (5) and (4), respectively. This is done to avoid name collisions with another
  # AnnularMeshGenerator we use for the fuel pellet. After we rename the sideset IDs, we
  # need another RenameBoundaryGenerator to rename the sideset names.
  [rename_clad]
    type = RenameBoundaryGenerator
    input = extrude_clad
    old_boundary = '1 0' # outer surface, inner surface
    new_boundary = '5 4'
  []
  [rename_clad_names]
    type = RenameBoundaryGenerator
    input = rename_clad
    old_boundary = 'rmax rmin' # outer surface, inner surface
    new_boundary = 'rmax_c rmin_c'
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
    type = AdvancedExtruderGenerator
    input = fuel
    heights = '${L}'
    num_layers = '40'
    direction = '0 0 1'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'rename_clad_names extrude'
  []

  # one of the mesh generators does not work with distributed mesh
  parallel_type = replicated
[]
