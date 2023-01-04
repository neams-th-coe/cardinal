# In this input, MOOSEs domain contains the entire OpenMC domain, but some
# MOOSE elements arent mapped anywhere (this is facilitated by adding an
# extra pebble to the MOOSE mesh).

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
                 0 0 8
                 9 9 9'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
  [fluid]
    type = FileMeshGenerator
    file = stoplight.exo
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '200'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
  []
[]

[AuxVariables]
  [cell_volume]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_volume]
    type = CellVolumeAux
    variable = cell_volume
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  solid_blocks = '100'
  fluid_blocks = '200'
  tally_blocks = '100 200'
  tally_type = cell
  initial_properties = xml

  verbose = true
  solid_cell_level = 0
  fluid_cell_level = 0
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  execute_on = 'final'
  exodus = true
  hide = 'density kappa_fission temp'
[]
