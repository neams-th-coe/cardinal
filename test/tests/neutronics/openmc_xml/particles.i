[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []

  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  solid_blocks = '100'
  solid_cell_level = 0
  tally_type = cell
  normalize_by_global_tally = false
  skip_first_incoming_transfer = true

  # In the XML files, we set 100 particles, but here we override that to use 200 particles
  particles = 200
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Outputs]
  csv = true
[]
