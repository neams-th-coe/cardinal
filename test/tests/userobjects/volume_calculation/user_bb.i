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

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0
  temperature_blocks = '100'

  initial_properties = xml
  volume_calculation = vol
[]

[UserObjects]
  [vol]
    type = OpenMCVolumeCalculation
    n_samples = 10000
    lower_left = '0 0 0'
    upper_right = '0.1 0.1 0.1'
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
[]

[Outputs]
  csv = true
[]
