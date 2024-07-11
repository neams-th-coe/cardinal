[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
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
  [fluid]
    type = FileMeshGenerator
    file = ../../neutronics/heat_source/stoplight.exo
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
  [cell_vol]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_vol]
    type = CellVolumeAux
    variable = cell_vol
    volume_type = actual
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '100 200'
    tally_name = heat_source
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = '100 200'
  density_blocks = '200'
  verbose = true
  cell_level = 0

  initial_properties = xml
  volume_calculation = vol
[]

[UserObjects]
  [vol]
    type = OpenMCVolumeCalculation
    n_samples = 10000
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [vol_1]
    type = PointValue
    variable = cell_vol
    point = '0.0 0.0 0.0'
  []
  [vol_2]
    type = PointValue
    variable = cell_vol
    point = '0.0 0.0 4.0'
  []
  [vol_3]
    type = PointValue
    variable = cell_vol
    point = '0.0 0.0 8.0'
  []
  [vol_4]
    type = PointValue
    variable = cell_vol
    point = '0.0 0.0 2.0'
  []
[]

[Outputs]
  csv = true
[]
