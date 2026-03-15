# Griesheimer and Kooreman Analytic Benchmark

## Problem Description

This test case models the benchmark problem proposed in [!cite](s2_thermal_expansion),
which verifies coupled neutronics, heat conduction, and thermal expansion.
The problem consists of a 1D multiplying slab where neutrons are restricted to travel
along either $\hat{\Omega} = (-1, 0, 0)$ or $\hat{\Omega} = (1, 0, 0)$, and vacuum boundary
conditions are applied at $x = \pm L/2$ (where $L$ is the deformed length of the slab). The
S@2@ neutron transport equation may be written as a neutron diffusion equation under these
constraints:

!equation id=transport_eq
\frac{d}{dx}\left(\frac{1}{\Sigma_{t}(x)}\frac{d\phi(x)}{dx}\right) + \left(\lambda - 1\right)\Sigma_{t}(x)\phi(x) = 0\text{,}

with the following Marshak boundary condition:

!equation id=transport_bc
\frac{d\phi(x)}{dx} + \Sigma_{t}(x)\phi(x),\,\,\,\, x = \pm \frac{L}{2}\text{.}

$\phi(x)$ is the scalar neutron flux (cm^-2^ s^-1^). $\Sigma_{t}(x)$ is the total cross
section (cm^-1^), which has an inverse dependence on temperature:

!equation id=total_xs
\Sigma_{t}(x) = \Sigma_{t,0}\frac{T_{0}}{T(x)}\text{,}

where $T_{0}$ is the reference temperature of the slab (K) and $T(x)$ is the slab temperature (K).
$\lambda$ is the combined scattering and k-eigenvalue fission source:

!equation id=lambda
\lambda = \frac{1}{k_{eff}}\nu\frac{\Sigma_{f}}{\Sigma_{t}} + \frac{\Sigma_{s}}{\Sigma_{t}}\text{,}

where $k_{eff}$ is the k-eigenvalue, $\nu$ is the average number of neutrons released per
fission reaction, $\Sigma_{f}/\Sigma_{t}$ is the fractional probability of a fission
reaction and $\Sigma_{s}/\Sigma_{t}$ is the fractional probability of a scattering reaction.
Both reaction probabilities are constant over the length of the slab. $\Sigma_{t,0}$ is defined
as:

!equation id=total_xs_0
\Sigma_{t,0} = \frac{N_{A}}{A}\rho_{0}\sigma_{t,0}\text{,}

where $N_{A}$ is Avogadro's number, $A$ is the molar mass of the slab (g mol^-1^), $\rho_{0}$
is the unheated slab density (g cm^-3^), and $\sigma_{t,0}$ is the reference microscopic total
cross section (cm). The neutron flux solution is normalized to a chosen power $P$ (eV s^-1^):

!equation id=normalization
P = \int_{-L/2}^{L/2}q\Sigma_{t}(x)\phi(x)\,dx

where $q$ is the energy released per neutron interaction (eV). The temperature distribution in the
slab is governed by the following heat conduction equation:

!equation id=conduction
\frac{d}{dx}\left(\kappa(x)\frac{d T(x)}{dx}\right) + q\Sigma_{t}(x)\phi(x) = 0\text{,}

with the following convection boundary condition:

!equation id=conduction_bc
\kappa(x)\frac{dT(x)}{dx} + h\left(T(x) - T_{0}\right),\,\,\,\, x = \pm \frac{L}{2}\text{.}

$\kappa(x)$ is the thermal conductivity of the material (eV s^-1^ cm^-1^ K^-1^), which has a
linear dependence on temperature:

!equation id=thermal_conduct
\kappa(x) = \kappa_{0}T(x)

where $\kappa_{0}$ is the reference thermal conductivity (eV s^-1^ cm^-1^ K^-2^). $h$ is a convective heat
transfer coefficient (eV s^-1^ cm^-2^ K^-1^). The final set of physics considered in this problem is
thermal expansion, where the differential slab length $\ell(x)$ (cm) expands according to the local
thermal strain $\epsilon_{x}(x)$:

!equation id=length
\ell(x) = \ell_{0}(1 + \epsilon_{x}(x))\text{,}

!equation id=thermal_strain
\epsilon_{x}(x) = \int_{T_{0}}^{T(x)}\alpha(\tau)\, d\tau\text{,}

where $\ell_{0}$ is the initial slab length, and the linear expansion coefficient $\alpha(T)$ (K^-1^) is:

!equation id=linear_expansion
\alpha(T) = \frac{1}{\sqrt{2T_{0}T(x)}}\text{.}

Mass is conserved by ensuring that the following expression is satisfied:

!equation id=mass_conservation
\rho(x)\ell(x) = \rho_{0}\ell_{0}\text{.}

## Analytical Solutions

The analytical solutions to the coupled problem were derived by [!cite](s2_thermal_expansion)
under the assumption that the temperature and neutron flux fields are proportional to each other.
To ensure this assumption is valid, $\sigma_{t,0}$ and $h$ are manufactured based on other
benchmark parameters. For details regarding the derivation of the benchmark solution, readers
are referred to [!cite](s2_thermal_expansion).

The analytic solutions for the neutron flux ([neutron_flux_sln]), temperature ([temperature_sln]),
and power density ([q_dot_sln]) can be found below:

!equation id=neutron_flux_sln
\phi(x) = \phi(0)\sqrt{1 - \frac{(\lambda - 1)P^2 x^2}{L^2 q^2 \phi^2(0)}}\text{,}

!equation id=temperature_sln
T(x) = \frac{q \Sigma_{t,0} T_{0} L}{P}\phi(x)\text{,}

and

!equation id=q_dot_sln
\Sigma_{t}(x)\phi(x)q = \frac{P}{L}\text{.}

The neutron flux in the center of the slab ($\phi(0)$) is arbitrary, and is therefore considered
a benchmark parameter. The slab eigenvalue can be calculated with [lambda_sln]:

!equation id=lambda_sln
\lambda = \frac{1}{2}\left(1 + \sqrt{1 + \frac{16 q^2 \phi^2(0)}{P^2}}\right)\text{,}

and the expanded slab length can be calculated with [length_sln]:

!equation id=length_sln
L = \sqrt[3]{\left(\frac{L_{0}}{_2F_1\left(1/4,1/2,3/2,1/\lambda\right)}\right)^{4}\left(\frac{\frac{q^2 \phi^2(0)}{P(\lambda - 1)} - \frac{P}{4}}{\kappa_{0}T_{0}^2(1 - 1/\lambda)}\right)}\text{,}

where $_2F_1(a,b,c,d)$ is the Gauss hypergeometric function. The manufactured benchmark parameters are
calculated with [manufactured_sigma] and [manufactured_h]:

!equation id=manufactured_sigma
\sigma_{t, 0} = \frac{1}{M\rho_{0}T_{0}}\sqrt{\frac{P}{\kappa_{0}L(\lambda - 1)}}\text{;}

!equation id=manufactured_h
h = \left(\sqrt{\frac{L(\lambda - 1)}{\kappa_{0}P}} - \frac{2T_{0}}{P}\right)^{-1}\text{.}

To ensure positivity of the solution, the power must be chosen such that:

!equation id=power_constraint
P \geq \frac{\lambda \kappa_{0}T_{0}^2}{L(\lambda - 1)}\text{.}

The cannonical parameters of the benchmark proposed by [!cite](s2_thermal_expansion) can be
found in [benchmark_params]. These parameters result in an expanded slab length of $L = 106.47$
cm and a slab k-eigenvalue of $k_{eff} = 0.29557$.

!table id=benchmark_params caption=Thermal expansion benchmark parameters proposed in [!cite](s2_thermal_expansion).
| Parameter | Value |
| :- | :- |
| $\rho_{0}$ | $1.2$ (g cm^-3^) |
| $L_{0}$ | $100$ (cm) |
| $A$ | $180$ (g mol^-1^) |
| $T_{0}$ | $293$ (K) |
| $q$ | $1\times 10^{8}$ (eV) |
| $P$ | $1\times 10^{22}$ (eV s^-1^) |
| $\kappa_{0}$ | $1.25\times 10^{19}$ (eV s^-1^ K^-2^) |
| $\phi(0)$ | $2.5\times 10^{14}$ (cm^-2^ s^-1^) |
| $\nu\Sigma_{f}/\Sigma_{t}$ | $1.5$ |
| $\Sigma_{s}/\Sigma_{t}$ | $0.45$ |

## Cardinal Multi-Group Monte Carlo Results

[!cite](eltawila_thermo) used the Griesheimer and Kooreman problem benchmark problem to verify Cardinal's
thermomechanic-neutronics coupling to OpenMC. Those benchmark results requried substantial modifications
to OpenMC's multi-group Monte Carlo solver to enable S@2@ neutron transport, precluding the inclusion of
input files in Cardinal's regression suite. Readers interested in these results are referred to
[!cite](eltawila_thermo) for more information.

## Cardinal Random Ray Solutions

The Griesheimer and Kooreman problem is solved in Cardinal using [!ac](TRRM) in a similar manner to the
multi-group Monte Carlo approach presented in [!cite](eltawila_thermo). The neutronics domain is
discretized into 200 slab regions, which are further subdivided into 31 tetrahedral elements per
slab subregion (for a total of 6200 flat source regions). The conversion to tetrahedral elements is
necessary to use on-the-fly geometry skinning in Cardinal for material deformation.
Unfortunately, this conversion introduces non-physical source region gradients in
the y and z direction which bias the [!ac](TRRM) solution. It is important to note that this bias
only exists for solutions using [!ac](TRRM); multi-group Monte Carlo solutions in Cardinal do not
experience any solution bias as the fission source is continuous. 100 inactive batches are used to
converge the scattering and fission sources, and 100 active batches are used to accumulate tally results.
An inactive ray length of $150$ cm is used to ensure rays reach a vacuum boundary condition (to
remove initialization bias), which is followed by an active length of $7500$ cm. Temperature
feedback is implemented by interpolating a cross section grid with a spacing of $1$ K.

The neutronics model is coupled with a heat conduction and thermal expansion solver from the MOOSE
Heat Transfer module in an identical manner to the approach presented in [!cite](eltawila_thermo).
Temperatures and displacements are solved on a 1D mesh with linear Lagrange basis functions. A
quasi-static finite strain formulation is used, and a time derivative is included in the temperature
model to act as physics-based relaxation. The solution is iterated between neutronics and thermomechanics
with Picard iterations; it is considered to be converged when the relative difference between neutronics
field variables between Picard iterations is less then $10^{-8}$. The resulting flux and temperature
distributions can be found in [gk_rr_200].

!media gk_rr_200.png
  id=gk_rr_200
  caption=Scalar flux and temperature distributions predicted with [!ac](TRRM) in Cardinal.
  style=width:100%;margin-left:auto;margin-right:auto;halign:center

The impact of the y-z induces source region gradients can be seen in the flux solution, resulting in
reasonable (but not perfect) agreement with [flux_sln]. The thermomechanics problem uses a true
1D mesh, resulting in better agreement between the numerical temperature predictions with
[temperature_sln]. The [!ac](TRRM) solution predicts $k_{eff} = 0.29520$ ($\Delta k_{eff} = −37$ pcm)
and an expanded slab length $L = 106.47$ cm. It is expected that the agreement in both the flux
and eigenvalue solutions will improve when hexahedral element support is added to the [MoabSkinner.md],
as this will eliminate the y-z source region gradients in the [!ac](TRRM) solution.

The input files for the [!ac](TRRM) solutions to the Griesheimer and Kooreman problem can be found in
`/test/tests/neutronics/mg/rr_thermal_expansion`, where they are used to regression test on-the-fly
skinning with [!ac](TRRM). Please note that these input files have been modified to run a 20 cell mesh,
with fewer Picard iterations and rays per batch to reduce the computational requirements of the test.

!listing /test/tests/neutronics/mg/rr_thermal_expansion/openmc.i

!listing /test/tests/neutronics/mg/rr_thermal_expansion/solid.i
