!include ../common.i

[TriSubChannelMesh]
  [subchannel]
    type = SCMDetailedTriSubChannelMeshGenerator
    nrings = 5
    n_cells = 100
    flat_to_flat = ${fparse duct_inner_flat_to_flat * 1e-2}
    heated_length = ${fparse height * 1e-2}
    pin_diameter = ${fparse outer_clad_diameter * 1e-2}
    pitch = ${fparse pin_pitch * 1e-2}
  []
[]

[AuxVariables]
  [mdot]
    block = subchannel
  []
  [P]
    block = subchannel
  []
  [T]
    block = subchannel
  []
  [rho]
    block = subchannel
  []
  [S]
    block = subchannel
  []
[]

[Problem]
  type = NoSolveProblem
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
[]
