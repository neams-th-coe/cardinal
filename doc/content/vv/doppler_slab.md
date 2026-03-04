# Doppler Slab Analytic Benchmark

## Problem Description

This test case models the 1D Doppler slab benchmark from [!cite](doppler_slab_benchmark), which
couples neutron transport and heat conduction in a semi-infinite slab with microscopic
absorption cross section $\sigma_{0}$, number density $N_{0}$, and thermal conductivity $k$. A
beam of neutrons (intensity $\psi_{0}$) is incident on the left face ($z = 0$) of the slab, which
is held at a fixed temperature of $T_{0}$. The geometry of the benchmark problem can be found in
[doppler_geom].

!media doppler_slab_geometry.png
  id=doppler_geom
  caption=Doppler slab geometry, taken from [!cite](doppler_slab_benchmark).
  style=width:40%;margin-left:auto;margin-right:auto;halign:center

The macroscopic absorption cross section in the slab takes one of two functional forms. The first is
a linear dependence on temperature:

!equation id=linear_feedback
\Sigma(z) = \underbrace{N_{0}\sigma_{0}}_{\Sigma_{0}} + \alpha\left(T(z) - T_{0}\right)\text{.}

The second is an inverse square-root dependence on temperature:

!equation id=inverse_root_feedback
\Sigma(z) = \underbrace{N_{0}\sigma_{0}}_{\Sigma_{0}}\sqrt{\frac{T_{0}}{T(z)}}\text{.}

Absorption reactions heat the slab with an energy released per reaction of $q$. The analytic solution
for the temperature ([linear_temp]) and flux ([linear_flux]) in the slab when the cross section is
represented by [linear_feedback] can be obtained:

!equation id=linear_temp
T(z) = T_{0} + \frac{2 \phi_{0} T_{0}\left[1 - \exp\left(-\sqrt{A}\Sigma_{0} z\right)\right]}
{1 - \exp\left(-\sqrt{A}\Sigma_{0}z\right) + \sqrt{A}\left[1 + \exp\left(-\sqrt{A}\Sigma_{0}z\right)\right]}\text{,}

!equation id=linear_flux
\psi(z) = \frac{4 \psi_{0} A\left[1 - \exp\left(-\sqrt{A}\Sigma_{0} z\right)\right]}
{\left[1 + \sqrt{A} - \left(1 - \sqrt{A}\right)\exp\left(-\sqrt{A}\Sigma_{0}z\right)\right]^{2}}\text{.}

$\phi_{0}$ is defined as:

!equation id=energy_flux
\phi_{0} = \frac{\psi_{0} q}{k T_{0} \Sigma_{0}}\text{,}

and $A$ is:

!equation id=A
A = 1 + \frac{2\alpha\psi_{0}q}{k\Sigma_{0}^2}\text{.}

For the case where [inverse_root_feedback] is used for the cross section, the following solutions for the
temperature ([inv_root_temp]) and flux ([inv_root_flux]) can be obtained:

!equation id=inv_root_temp
T(z) = T_{0}B^2\left[W\left(\frac{\phi_{0}}{2B}\exp\left(\frac{\Sigma_{0}z + C}{B}\right)\right) + 1\right]^2\text{,}

and

!equation id=inv_root_flux
\psi(z) = \frac{2k T_{0}\Sigma_{0}B}{q}\left[W\left(\frac{\phi_{0}}{2B}\exp\left(\frac{\Sigma_{0}z + C}{B}\right)\right)\right]\text{.}

$B$ is defined as:

!equation id=B
B = -\left[1 + \frac{\phi_{0}}{2}\right]\text{,}

and $C$ is:

!equation id=C
C = \frac{\phi_{0}}{2}\text{.}

$W(\mathcal{Z})$ is the Lambert W function (also known as the product logarithm). When using linear
temperature feedback, the benchmark parameters must be selected such that $A > 0$ to prevent non-
physical solutions (negative cross sections). There are no such constraints on the inverse-root
feedback. The cannonical benchmark parameters can be found in [benchmark_params].

!table id=benchmark_params caption=Doppler slab benchmark parameters proposed in [!cite](doppler_slab_benchmark).
| Parameter | Value |
| :- | :- |
| $\psi_{0}$ | $6.65\times 10^{11}$ (n cm^-2^ s^-1^) |
| $N_{0}$ | $0.025$ (atom barn^-1^ cm^-1^) |
| $\sigma_{0}$ | $4$ (barn) |
| $q$ | $10^{6}$ (eV) |
| $T_{0}$ | $293$ (K) |
| $k$ | 0.6 W (m^-1^ K^-1^) |
| $\alpha$ | $-0.0001$ (cm^-1^ K^-1^) |

## Cardinal Multi-Group Monte Carlo Solutions

Cardinal's coupling between MOOSE's heat conduction solver, OpenMC, and NekRS
running as a solid heat conduction solver was verified in [!cite](doppler_slab_mc_cardinal) with the
Doppler slab problem. 200 mesh elements with linear Lagrange basis functions were used in the heat
conduction solution. The OpenMC simulation used 200 cells and a mesh tally with 200 bins to ensure
temperature feedback and flux tallies matched the heat conduction mesh. As the benchmark problem
is semi-infinite, a length must be chosen to truncate the problem at in order to apply a symmetry boundary
condition (heat conduction) and vacuum condition (neutronics). This was selected to be $200$ cm.
Results for [linear_feedback] can be found in [doppler_linear], and results for
[inverse_root_feedback] can be found in [doppler_inv_root].

!media doppler_linear.png
  id=doppler_linear
  caption=Cardinal simulations of the Doppler slab problem with linear temperature feedback, taken from [!cite](doppler_slab_mc_cardinal).
  style=width:100%;margin-left:auto;margin-right:auto;halign:center

!media doppler_inv_root.png
  id=doppler_inv_root
  caption=Cardinal simulations of the Doppler slab problem with inverse root temperature feedback, taken from [!cite](doppler_slab_mc_cardinal).
  style=width:100%;margin-left:auto;margin-right:auto;halign:center

The maximum relative error for linear feedback is $0.0767\%$ (temperature) and $4.0100\%$ (flux). The maximim
relative error for inverse root feedback is $0.1500\%$ (temperature) and $0.7800\%$ (flux). Error in Cardinal results
is dominated by (i) statistical noise near the right boundary due to neutron attenuation, and (ii) boundary condition
effects at the right boundary caused by the finite (as opposed to semi-infinite) slab. More information regarding the
problem setup and results can be found in [!cite](doppler_slab_mc_cardinal).

Input files for the linear temperature feedback case are included in Cardinal to verify the heat conduction module,
Cardinal's OpenMC coupling, and different temperature interpolation schemes present in OpenMC. These input files
can be found in `/test/tests/neutronics/mg/doppler_slab_lin`. The OpenMC and heat conduction inpute files for this
problem can be found below.

!listing /test/tests/neutronics/mg/doppler_slab_lin/openmc_base.i

!listing /test/tests/neutronics/mg/doppler_slab_lin/solid.i
