# SetupMGXSAction

!alert note
`SetupMGXSAction` can only set up multi-group cross section generation in problems which contain a
[OpenMCCellAverageProblem](OpenMCCellAverageProblem.md) in the `[Problem]` block. Otherwise,
attempting to add a tally will result in an error.

## Overview

The `SetupMGXSAction` action is responsible for automating the generation of multi-group cross sections (MGXS)
on Cardinal's mesh mirror for coupled deterministic codes. The type of spatial discretization used for MGXS
homogenization is selected by setting `tally_type` to `cell` (to select a [distributed cell tally](CellTally.md))
or to `mesh` ([to select an unstructured mesh tally](MeshTally.md)). The energy groups boundaries used can either be
specified in `energy_boundaries`, or a common group structure can be selected by setting `group_structure`. The type of
particle to filter for can be selected in `particle`. At present valid options include `neutron` and `photon` cross
sections, this list may be expanded in the future to support `electron` / `positron` cross sections if OpenMC adds
support for charged particle transport. The tally estimator used to generate the MGXS can be selected by setting
`estimator`. Please note that `estimator` is not applied to group properties derived from `nu-scatter` / `nu-fission`
scores (these are restricted to `analog` estimators), and setting `tally_type = mesh` requires a
non-tracklength estimator. A description of the group properties available through this action can be found below.

## Supported Group Properties

`SetupMGXSAction` implements MGXS generation in a manner similar to that of OpenMC's MGXS Python API. We provide a brief overview
of this approach below; for additional details we refer users to [OpenMC's MGXS documentation](https://docs.openmc.org/en/stable/pythonapi/mgxs.html#multi-group-cross-sections).

### Total Cross Sections

At a minimum, this action always computes total MGXS, which is computed with the following formula (implemented with
Cardinal's mapped tallies and filters):

\begin{equation}
\label{eq:mg_total_xs}
\Sigma_{t,i,g} = \frac{\langle\Sigma_{t}\psi\rangle_{i,g}}{\langle\psi\rangle_{i,g}}
\end{equation}

where

\begin{equation}
\label{eq:mg_Tot_rxn_rate}
\langle\sigma_{t}\psi\rangle_{i,g} = \int_{V_i}\,dr^3\int_{4\pi}\,d\Omega\int_{E_{g}}^{E_{g-1}}\,dE\,\Sigma_{t}(\vec{r}, E)\psi(\vec{r}, E, \hat{\Omega})
\end{equation}

and

\begin{equation}
\label{eq:mg_flux}
\langle\phi\rangle_{i,g} = \int_{V_i}\,dr^3\int_{4\pi}\,d\Omega\int_{E_{g}}^{E_{g-1}}\,dE\,\psi(\vec{r}, E, \hat{\Omega})
\end{equation}

$\Sigma_{t,i,g}$ is the homogenized total MGXS, $V_{i}$ is the homogenization volume (determined by `tally_type`), $\Sigma_{t}(\vec{r}, E)$ is the continuous-energy total macroscopic cross section, and $\psi(\vec{r}, E, \hat{\Omega})$ is the continuous energy angular flux.

### Absorption Cross Sections

Absorption cross sections can be added by setting `add_absorption = true`, and are calculated with the following:

\begin{equation}
\label{eq:mg_abs_xs}
\Sigma_{t,i,g} = \frac{\langle\Sigma_{a}\psi\rangle_{i,g}}{\langle\psi\rangle_{i,g}}
\end{equation}

where

\begin{equation}
\label{eq:mg_abs_rxn_rate}
\langle\sigma_{a}\psi\rangle_{i,g} = \int_{V_i}\,dr^3\int_{4\pi}\,d\Omega\int_{E_{g}}^{E_{g-1}}\,dE\,\Sigma_{a}(\vec{r}, E)\psi(\vec{r}, E, \hat{\Omega})
\end{equation}

$\Sigma_{a,i,g}$ is the homogenized absorption MGXS and $\Sigma_{a}(\vec{r}, E)$ is the equivalent continuous-energy macroscopic cross section.

### Nu-Scattering Matrices

By default this action adds MGXS scattering matrices with particle multiplication, though this behaviour can be disabled by setting
`add_scattering = false`. Scattering cross sections are expanded in Legendre moments of $\mu = \hat{\Omega}'\cdot\hat{\Omega}$,
where $\hat{\Omega}'$ is the direction entering a scattering reaction and $\hat{\Omega}$ is the direction exiting a scattering
reaction. The maximum order of the Legendre expansion can be specified by setting `legendre_order` (where the default of 0 indicates
isotropic scattering). At present, `SetupMGXSAction` implements a 'simple' formulation of the scattering matrix where the elements
are computed with `analog` estimators:

\begin{equation}
\label{eq:mg_scatter_xs}
\Sigma_{s,i,\ell,g'\rightarrow g} = \frac{\langle\sigma_{s}\psi\rangle_{i,\ell,g\rightarrow g'}}{\langle\psi\rangle_{i,g}}
\end{equation}

where

\begin{equation}
\label{eq:mg_scatter_rxn_rate}
\langle\nu\sigma_{s}\psi\rangle_{i,\ell,g'\rightarrow g} = \int_{V_i}\,dr^3\int_{4\pi}\,d\Omega'\int_{E'_{g}}^{E'_{g-1}}\,dE'\int_{4\pi}\,d\Omega\int_{E_{g}}^{E_{g-1}}\,dE\,\nu\Sigma_{s}(\vec{r}, \mu, E'\rightarrow E)P_{\ell}(\mu)\psi(\vec{r}, E, \hat{\Omega}')
\end{equation}

$\nu\Sigma_{s,i,\ell,g'\rightarrow g}$ is the homogenized nu-scatter MGXS, $\nu\Sigma_{s}(\vec{r}, \mu, E'\rightarrow E)$ is the equivalent continuous-energy macroscopic cross section, $P_{\ell}(\mu)$ are the Legendre polynomials, and $E'$ indicates the energy entering a scattering reaction.
`SetupMGXSAction` supports transport corrected P0 ($\ell = 0$) scattering cross sections, which are computed with the following:

\begin{equation}
\label{eq:mg_scatter_tc_xs}
\Sigma_{s,i,0,g'\rightarrow g} = \frac{\langle\sigma_{s}\psi\rangle_{i,0,g\rightarrow g'} - \delta_{gg'}\sum_{g''}\langle\sigma_{s}\psi\rangle_{i,1,g''\rightarrow g}}{\langle\psi\rangle_{i,g}}
\end{equation}

where $\delta_{gg'}$ is the Khronecker delta function. The transport correction can be applied by setting `transport_correction = true`.

### Nu-Fission Cross Sections

Nu-fission (neutron production) cross sections can be added by setting `add_fission = true`, and are calculated with the following:

\begin{equation}
\label{eq:mg_prod_xs}
\nu\Sigma_{f,i,g} = \frac{\langle\nu\Sigma_{f}\psi\rangle_{i,g}}{\langle\psi\rangle_{i,g}}
\end{equation}

where

\begin{equation}
\label{eq:mg_prod_rxn_rate}
\langle\sigma_{a}\psi\rangle_{i,g} = \int_{V_i}\,dr^3\int_{4\pi}\,d\Omega\int_{E_{g}}^{E_{g-1}}\,dE\,\nu\Sigma_{f}(\vec{r}, E)\psi(\vec{r}, E, \hat{\Omega})
\end{equation}

$\nu\Sigma_{f,i,g}$ is the homogenized neutron production MGXS and $\nu\Sigma_{f}(\vec{r}, E)$ is the equivalent continuous-energy macroscopic cross section. The tallies used to compute $\nu\Sigma_{f,i,g}$ are also used to compute $\chi_{i,g}$, necessitating the use of an `analog` estimator for $\nu\Sigma_{f,i,g}$.

### Discrete Chi Spectra

Discrete chi spectra can also be added by setting `add_fission = true`, and are computed with the following:

\begin{equation}
\label{eq:mg_chi}
\chi_{i,g} = \frac{\langle\chi\nu\Sigma_{f}\psi\rangle_{i,g}}{\langle\nu\Sigma_{f}\psi\rangle_{i}}
\end{equation}

where

\begin{equation}
\label{eq:mg_chi_rxn_rate}
\langle\chi\nu\Sigma_{f}\psi\rangle_{i,g} = \int_{V_i}\,dr^3\int_{4\pi}\,d\Omega'\int_{0}^{\infty}\,dE'\int_{E_{g}}^{E_{g-1}}\,dE\,\chi(\vec{r}, E)\nu\Sigma_{f}(\vec{r}, E')\psi(\vec{r}, E', \hat{\Omega}')
\end{equation}

and

\begin{equation}
\label{eq:mg_chi_norm}
\langle\nu\Sigma_{f}\psi\rangle_{i} = \int_{V_i}\,dr^3\int_{4\pi}\,d\Omega'\int_{0}^{\infty}\,dE'\int_{0}^{\infty}\,dE\,\chi(\vec{r}, E)\nu\Sigma_{f}(\vec{r}, E')\psi(\vec{r}, E', \hat{\Omega}')
\end{equation}

$\chi_{i,g}$ is the homogenized MG chi spectra and $\chi(\vec{r}, E)$ is the equivalent continuous-energy spectra. The need to know the entering and exiting energies for the nu-fission reaction rate necessitates the use of an `analog` estimator for $\chi_{i,g}$.

### Fission Heating Values

Fission heating values can be added by setting `add_fission_heating = true`, and are computed with the following:

\begin{equation}
\label{eq:mg_kf}
\kappa\Sigma_{f,i,g} = \frac{\langle\kappa\Sigma_{f}\psi\rangle_{i,g}}{\langle\psi\rangle_{i,g}}
\end{equation}

where

\begin{equation}
\label{eq:mg_kf_rxn_rate}
\langle\kappa\Sigma_{f}\psi\rangle_{i,g} = \int_{V_i}\,dr^3\int_{4\pi}\,d\Omega\int_{E_{g}}^{E_{g-1}}\,dE\,\kappa\Sigma_{f}(\vec{r}, E)\psi(\vec{r}, E, \hat{\Omega})
\end{equation}

$\kappa\Sigma_{f,i,g}$ is the homogenized MG fission heating cross section and $\kappa\Sigma_{f}(\vec{r}, E)$ is the equivalent continuous-energy cross section.

### Inverse Velocity Values

Inverse velocity values can be added by setting `add_inverse_velocity = true`, and are computed with the following:

\begin{equation}
\label{eq:mg_inv_v}
\Big(\frac{1}{v}\Big)_{i,g} = \frac{\langle\frac{\psi}{v}\rangle_{i,g}}{\langle\psi\rangle_{i,g}}
\end{equation}

where

\begin{equation}
\label{eq:mg_inv_v_rxn_rate}
\Big\langle\frac{\psi}{v}\Big\rangle_{i,g} = \int_{V_i}\,dr^3\int_{4\pi}\,d\Omega\int_{E_{g}}^{E_{g-1}}\,dE\,\frac{\psi(\vec{r}, E, \hat{\Omega})}{v(E)}
\end{equation}

$(\frac{1}{v})_{i,g}$ is the homogenized MG inverse velocity and $v(E)$ is the associated continuous energy velocity.

### Particle Diffusion Coefficients

Particle diffusion coefficients can be added by setting `add_diffusion_coefficient = true`, and are computed with the following:

\begin{equation}
\label{eq:mg_diffusion}
D_{i,g} = \frac{1}{3\Sigma_{tr,i,g}}
\end{equation}

\begin{equation}
\label{eq:mg_tr}
\Sigma_{tr,i,g} = \frac{\langle\sigma_{t}\psi\rangle_{i,g} - \sum_{g'}\langle\nu\sigma_{s}\psi\rangle_{i,1,g'\rightarrow g}}{\langle\psi\rangle_{i,g}}
\end{equation}

$D_{i,g}$ is the homogenized MG diffusion coefficient and $\Sigma_{tr,i,g}$ is the homogenized transport MGXS.

## Example Input Syntax

The example below computes every available group property using a distributed cell tally for spatial homogenization and the CASMO-2 energy group structure.

!listing /test/tests/neutronics/gen_mgxs/all_mgxs_cell.i
  block=Problem

!syntax parameters /Problem/MGXS/SetupMGXSAction

