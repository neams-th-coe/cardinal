When using `field = velocity_component`, the manner in which the velocity
direction is selected is given by the `velocity_component` parameter, which
may be one of:

- `normal`: normal to the face bins (only valid for the side binning user objects)
- `user`: arbitrary user-specified direction, provided with the `velocity_direction`
  parameter

When using a velocity component as the field, this user object computes the
dot product of the velocity with the specified direction. To retain the components
of the dot product, such as for visualizing vector glyphs of the user object result
in Paraview, you can use a [NekSpatialBinComponentAux](NekSpatialBinComponentAux.md).
