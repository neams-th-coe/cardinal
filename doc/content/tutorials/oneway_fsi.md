# One-way coupled fluid-structure interaction

Tutorial outline:

Introduction to benchmark (with warning to not use it as such)

Description of mesh files

  Parameters you can control at the top, where mesh refinement may be necessary (near edges)

Description of Nek input files

  Description of velocity profile,

Setup of tensor mechancics

  

Initial Condition Generation

  Why is it necessary? Incomp. fluid, sudden shift, displacement, negative Jacobians
  Better to ease it into the displaced state

Description of coupling

  Mirror, transfer of mesh velocity/traction

Tips for visualization

  Warp by vector in Paraview. Variables starting with `disp` are automatically recognized
  by Paraview as displacements used to move the mesh.

