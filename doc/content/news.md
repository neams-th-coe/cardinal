# News

This page summarizes recent changes to Cardinal and outlines a number of up-and-coming
features.

## Up-and-Coming Features

- The NekRS wrapping has basic support for applying MOOSE volume deformations to
  a NekRS mesh. This is being extended for boundary-based mesh deformations by
  connecting NekRS's [ALE solver](https://www.osti.gov/biblio/1510253) to MOOSE's
  tensor mechanics module.
- OpenMC will soon support unstructured mesh tracking; we will extend the OpenMC
  wrapping to support tracking directly on a `MooseMesh`, combined with mesh
  deformations from MOOSE's tensor mechanics module.
- Add a distributed mesh implementation for the OpenMC wrapping to improve performance
  for large problems

## Latest News

- [March 2022](news/mar2022.md)
- [February 2022](news/feb2022.md)
