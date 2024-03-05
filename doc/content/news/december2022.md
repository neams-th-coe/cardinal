# December 2022 News

- Added interface to NekRS's mesh elasticity solver, which allows coupled NekRS-MOOSE solid mechanics simulations. Mesh deformations computed by MOOSE can be applied to a fluid boundary, and NekRS will then solve an elasticity problem to deform the mesh throughout the entire fluid domain. Tutorials will be coming soon. In the meantime, you can explore [this test](https://github.com/neams-th-coe/cardinal/tree/devel/test/tests/deformation/mesh-velocity-areas).
- Added a [new tutorial](https://cardinal.cels.anl.gov/tutorials/pincell_multiphysics.html) that couples OpenMC, NekRS, and MOOSE heat conduction for a pincell. This tutorial is fast-running and should be accessible to all users.
- Added a new NekRS mesh mirror option that will "exactly" rebuild the spectral element mesh. To use, simply set `exact = true` for the mesh. For more documentation, look [here](https://cardinal.cels.anl.gov/source/mesh/NekRSMesh.html).

```
[Mesh]
  type = NekRSMesh
  exact = true

  # ... other options
[]
```
