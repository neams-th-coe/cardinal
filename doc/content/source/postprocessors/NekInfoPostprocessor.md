# NekInfoPostprocessor

!syntax description /Postprocessors/NekInfoPostprocessor

## Description

This postprocessor is used to extract results or information in the current time-step from the NekRS simulation.
The field is specified with the `test_type` parameter, which may be one of:

- 'n_iter_velocity': Number of iterations in the velocity solver.
- 'n_iter_pressure': Number of iterations in the pressure solver.
- 'n_iter_temperature': Number of iterations in the temperature solver.
- 'n_iter_scalar01': Number of iterations in the scalar 01.
- 'n_iter_scalar02': Number of iterations in the scalar 02.
- 'n_iter_scalar03': Number of iterations in the scalar 03.
- 'cfl': CFL number.
- 'tstep': Total number of time-steps performed.
- 'p0th': Thermodynamic pressure.
- 'dp0thdt': Time derivative of the thermodynamic pressure.
- 'solver_velocity': Return 1 if the velocity solver is active in NekRS.
- 'solver_temperature': Return 1 if the temperature solver is active in NekRS.
- 'solver_scalar01': Return 1 if the scalar 01 solver is active in NekRS.
- 'solver_scalar02': Return 1 if the scalar 02 solver is active in NekRS.
- 'solver_scalar03': Return 1 if the scalar 03 solver is active in NekRS.
- 'xmin': Minimum x of the NekRS geometry.
- 'xmax': Maximum x of the NekRS geometry.
- 'ymin': Minimum y of the NekRS geometry.
- 'ymax': Maximum y of the NekRS geometry.
- 'zmin': Minimum z of the NekRS geometry.
- 'zmax': Maximum z of the NekRS geometry.


## Example Input Syntax

Examples to get the number of iterations for the velocity, pressure, temperature, and scalars solver are presented:

!listing test/tests/nek_ci/ethier/nek2.i
  block=Postprocessors
  type=NekInfoPostprocessor

!syntax parameters /Postprocessors/NekInfoPostprocessor

!syntax inputs /Postprocessors/NekInfoPostprocessor

!syntax children /Postprocessors/NekInfoPostprocessor
