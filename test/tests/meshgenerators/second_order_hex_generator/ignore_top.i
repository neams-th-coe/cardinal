[Mesh]
  [tube]
    type = FileMeshGenerator
    file = tube_in.e
  []

  # Rename the top boundary to match the sideset we want to move, which we ignore because
  # its normal is parallel to the 'axis'
  [rename]
    type = RenameBoundaryGenerator
    input = tube
    old_boundary = '3'
    new_boundary = 'rmin'
  []

  [to_hex20]
    type = Hex20Generator
    input = rename
    boundary = 'rmin'
    radius = '0.05'
  []

  parallel_type = replicated
[]
