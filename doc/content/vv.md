# Verification and Validation

Verification and validation of Cardinal is essential to justify relevance of this application
to end-use applications in energy. Below, we collect a summary of the [!ac](V&V)
cases included in Cardinal; each includes a brief documentation describing the test case,
the input files, and [!ac](V&V) results. Cardinal and its dependencies have a very rich
history of [!ac](V&V), and the cases shown on this page only represent those test cases
which are included in Cardinal's [!ac](V&V) suite and which have been documented here; extending this page is actively funded
through the [!ac](DOE) [!ac](NEAMS) Program.

## Thermal-Fluids

The thermal-fluids cases below are configured as practical, repeatable regression tests.
To keep routine continuous-integration costs manageable, the cases use deliberately modest mesh resolutions, and their qualifying criteria reflect the selected resolutions and associated discretization errors. 
Further spatial refinement can reduce these errors and support tighter qualifying criteria.

- [Laminar natural convection in a 2-D square cavity](vv/square_cavity_natural_convection.md)
- [Turbulent Channel (RANS)](vv/nekrs/ktauChannel.md)
- [Rayleigh-Bénard Convection](vv/nekrs/rbc.md)
- [Turbulent Pipe Flow (LES)](vv/nekrs/turbPipePeriodic.md)
- [Backward Facing Step (RANS)](vv/nekrs/bfs.md)
- [Low-Mach Compressible Flow](vv/nekrs/lowMach.md)
- [Moving Cylinder (Low-mach)](vv/nekrs/mv_cyl.md)
- [Conjugate Heat Transfer](vv/nekrs/cht_nek.md)
- [Stokes Flow](vv/nekrs/channel.md)
- [Poiseuille Flow](vv/nekrs/shlChannel.md)
- [Periodic Hills (RANS and Hybrid RANS)](vv/nekrs/periodicHill.md)
- [Ethier (Navier-Stokes)](vv/nekrs/ethier.md)

## Neutronics

- [1D Doppler slab benchmark](vv/doppler_slab.md)
- [1D thermal expansion slab benchmark](vv/s2_thermal_expansion.md)
