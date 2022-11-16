# April 2022 News

- Added a [mesh converter](https://cardinal.cels.anl.gov/source/meshgenerators/NekMeshGenerator.html)
  to convert between HEX8 or HEX27 meshes into the HEX20 mesh format needed to use NekRS's
  `exo2nek` mesh generation utility. This mesh generator also optionally moves nodes to preserve
  cylindrical surfaces, allowing high-order curved meshes in NekRS.
- Added a distributed mesh implementation for use with the OpenMC wrapping. OpenMC Cardinal
  simulations can now be coupled to a domain-distributed MOOSE simulation. For information on how
  to split meshes, consult the [MOOSE documentation](https://mooseframework.inl.gov/syntax/Mesh/splitting.html).
  Then, you can run with a distributed mesh implementation by passing the `--distributed-mesh`
  command line parameter to `cardinal-opt`.
- Added documentation for the [NekRSSeparateDomainProblem](https://cardinal.cels.anl.gov/source/problems/NekRSSeparateDomainProblem.html)
  for coupling NekRS to a 1-D MOOSE thermal-hydraulics code, such as SAM or THM.
