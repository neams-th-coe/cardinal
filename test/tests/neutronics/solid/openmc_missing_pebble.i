# The OpenMC model is created with the scripts/openmc_pebble_bed_model.py with the following args
# openmc_pebble_bed_model.py -e 1.0
#
# This is a vertical stack of three pebbles, with a 1 meter thick layer of flibe
# on the outer periphery and on the top and bottom, to get a k closer to 1.0.
# In addition, to ensure that this test is reproducible, we modify the settings.xml
# to use the nearest temperature method and turn off windowed multipole.

# The tallies in each pebble are automatically created by OpenMCProblem -
# in this test, we accidentally forget to include all of the pebbles in the
# 'positions' provided to the mesh and OpenMCProblem to ensure that we do
# correctly catch that kappa-fission does not sum to the total.

[Mesh]
  [sphere]
    # Mesh of a single pebble with outer radius of 1.5 (cm)
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [multiple]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4'
  []
[]

[AuxVariables]
  [average_temp]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [average_temp]
    type = SpatialUserObjectAux
    variable = average_temp
    user_object = average_temp
  []
[]

[Problem]
  type = OpenMCProblem
  power = 2000.0
  centers = '0 0 0
             0 0 4'

  # volumes of each of the pebbles in the MOOSE mesh, so that we get the
  # correct normalization
  volumes = '1.322128e+01'

  tally_type = 'cell'
  pebble_cell_level = 1
[]

[Postprocessors]
  # These are the max/min temperatures received from MOOSE, which we expect to
  # match the max/min temperatures printed to screen following the temperature
  # transfer into OpenMC
  [max_T]
    type = ElementExtremeValue
    variable = average_temp
    value_type = max
  []
  [min_T]
    type = ElementExtremeValue
    variable = average_temp
    value_type = min
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
