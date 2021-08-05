The boundaries over which to compute this postprocessor in
the NekRS mesh are specified with the `boundary` parameter; these boundaries
are the sidesets in NekRS's mesh (i.e. the `.re2` file).
You can specify more than one boundary, with syntax such as
`boundary = '1 2'`.
To be clear, this postprocessor is *not* evaluated on the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.
