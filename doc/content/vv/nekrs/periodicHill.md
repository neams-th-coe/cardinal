# Periodic Hill Flow (RANS and Hybrid RANS/LES)

The *periodicHill* case models turbulent flow over a periodic array of two-dimensional hills.
The benchmark configuration and reference [!ac](LES) results are adopted from Fröhlich *et al.* [!citep](frohlich2005highly).
The adverse pressure gradient downstream of each hill produces flow separation, a recirculation region, and subsequent reattachment along the lower wall.
The case verifies the $k$-$\tau$ [!ac](SST) model and its [!ac](DDES) and [!ac](IDDES) variants in NekRS.
The $k$-$\tau$ SST formulation is described by Tomboulides *et al.* [!citep](tomboulides2025robust), while the hybrid RANS/LES formulations are discussed by Chang *et al.* [!citep](chang2023evaluation).

## Computational domain

The hill height $H=1$ is used as the reference length.
The domain dimensions are

!equation id=eq\:periodic-hill-domain
L_x=9H,\qquad L_y=3.035H,\qquad L_z=4.5H.

The lower boundary consists of a periodic hill profile with a width parameter $W=1.929H$, while the upper boundary is flat.
The domain is discretized using $12\times8\times9$ hexahedral elements in the streamwise, wall-normal, and spanwise directions, respectively, for a total of 864 elements.
A polynomial order of five is used within each element.
The mesh is stretched in the streamwise and wall-normal directions to increase resolution near the hill and within the separated shear layer.

Periodic boundary conditions are imposed in the streamwise $x$ and spanwise $z$ directions.
No-slip boundary conditions are imposed on the lower hill surface and the upper wall.
The turbulent quantities $k$ and $\tau$ are assigned zero Dirichlet boundary conditions at the walls.

[fig:periodic-hill-mesh] shows the regression-test mesh. Spectral elements use polynomial order $p=5$.

!media media/phill_mesh.png
       id=fig:periodic-hill-mesh
       style=width:75%;margin-left:auto;margin-right:auto;
       caption=Mesh for the periodic-hill regression test with fifth-order spectral elements.
       alt=Spectral-element mesh for the periodic-hill regression test

## Flow parameters

The bulk velocity, fluid density, and hill height are nondimensionalized as

!equation id=eq\:periodic-hill-reference-values
U_b=1,\qquad \rho=1,\qquad H=1.

The Reynolds number based on the bulk velocity and hill height is

!equation id=eq\:periodic-hill-reynolds
Re_H
=
\frac{\rho U_bH}{\mu}
=
\frac{U_bH}{\nu}
=
10595,

where $\mu$ and $\nu$ are the dynamic and kinematic viscosities, respectively.
The corresponding nondimensional molecular viscosity is

!equation id=eq\:periodic-hill-viscosity
\nu=\frac{1}{10595}.

A constant bulk flow rate is maintained in the streamwise direction throughout the simulation.

## Test Case

Three simulations are used to verify the turbulence-model implementations.
Each mode restarts from a statistically developed flow field and advances the solution for 0.5 nondimensional time units.
During this interval, the three velocity components are accumulated in time to obtain the mean velocity field.

The time-averaged velocity-magnitude contours and streamlines for the three turbulence models are shown in [fig:periodic-hill-sst], [fig:periodic-hill-ddes], and [fig:periodic-hill-iddes].
The separated flow region downstream of the hill and its subsequent reattachment are resolved by each model.

!media media/phill_sst.png
       id=fig:periodic-hill-sst
       style=width:80%;margin-left:auto;margin-right:auto;
       caption=Time-averaged velocity-magnitude contours and streamlines for the $k$-$\tau$ SST model.
       alt=Periodic-hill velocity-magnitude contours and streamlines for the SST model

!media media/phill_ddes.png
       id=fig:periodic-hill-ddes
       style=width:80%;margin-left:auto;margin-right:auto;
       caption=Time-averaged velocity-magnitude contours and streamlines for the $k$-$\tau$ SST DDES model.
       alt=Periodic-hill velocity-magnitude contours and streamlines for the SST DDES model

!media media/phill_iddes.png
       id=fig:periodic-hill-iddes
       style=width:80%;margin-left:auto;margin-right:auto;
       caption=Time-averaged velocity-magnitude contours and streamlines for the $k$-$\tau$ SST IDDES model.
       alt=Periodic-hill velocity-magnitude contours and streamlines for the SST IDDES model

The turbulence models and qualification tolerances for the three test cases are summarized in [tab:periodic-hill-ci-modes].

!table id=tab:periodic-hill-ci-modes caption=Periodic-hill test cases and skin-friction error tolerances.
| Test Case | Turbulence model | Restart field | Error tolerance |
| --- | --- | --- | ---: |
| 1 | $k$-$\tau$ SST | `sst.fld` | $1.04\times10^{-2}$ |
| 2 | $k$-$\tau$ SST DDES | `ddes.fld` | $1.14\times10^{-2}$ |
| 3 | $k$-$\tau$ SST IDDES | `iddes.fld` | $1.07\times10^{-2}$ |

## Verification criteria

The time-averaged velocity field is used to compute the mean strain-rate tensor and viscous shear stress along the periodic-hill wall.
The local skin-friction coefficient is defined as

!equation id=eq\:periodic-hill-skin-friction
C_f(x)
=
\frac{2\tau_w(x)}{\rho U_b^2},

where $\tau_w$ is the viscous wall shear stress.
Because $\rho=1$ and $U_b=1$ for this case, the coefficient evaluated by the test reduces to

!equation id=eq\:periodic-hill-skin-friction-reduced
C_f(x)=2\tau_w(x).

The computed skin-friction coefficient is compared with the reference LES results of Fröhlich *et al.* [!citep](frohlich2005highly).
The test uses the corresponding digitized skin-friction data available through the [NASA Turbulence Modeling Resource](https://turbmodels.larc.nasa.gov/Other_LES_Data/2Dhill_periodic/hill_LES_cf_digitized.dat).
The reference coefficient is linearly interpolated onto the surface quadrature points of the NekRS mesh.
The pointwise absolute error is

!equation id=eq\:periodic-hill-pointwise-error
e_{C_f}(x)
=
\left|
C_f(x)-C_{f,\mathrm{ref}}(x)
\right|.

The test qualification metric is the wall-area-averaged absolute error,

!equation id=eq\:periodic-hill-area-error
\varepsilon_{C_f}
=
\frac{
\displaystyle\int_{\Gamma_w}
\left|
C_f-C_{f,\mathrm{ref}}
\right|\,dA
}{
\displaystyle\int_{\Gamma_w}dA
},

where $\Gamma_w$ denotes the periodic-hill wall.
Each case passes when $\varepsilon_{C_f}$ is below its corresponding tolerance in [tab:periodic-hill-ci-modes].

[fig:periodic-hill-cf] compares the bottom-wall skin-friction coefficient obtained with the three turbulence models against the reference LES profile of Fröhlich *et al.* [!citep](frohlich2005highly).
The profiles in this comparison were generated using polynomial order $p=7$ and a velocity averaging interval of $\Delta t=100$; they provide a higher-resolution comparison than the modest configurations used for routine regression testing.

!media media/phill_cf.png
       id=fig:periodic-hill-cf
       style=width:70%;margin-left:auto;margin-right:auto;
       caption=Bottom-wall skin-friction coefficient from the turbulence models compared with the reference LES profile of Fröhlich *et al.* [!citep](frohlich2005highly).
       alt=Periodic-hill bottom-wall skin-friction coefficients compared with reference LES data
