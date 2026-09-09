# DimensionalizeAction

## Overview

The `DimensionalizeAction` action is responsible for fetching the characteristic scales from the user
and applying them to dimensionalize any field passed from NekRS into MOOSE (AuxVariables,
Postprocessors, UserObjects, etc.). To use, add a `[Dimensionalize]` block inside the `[Problem]` block.

Cardinal assumes that the NekRS inputs were nondimensionalized
with the following:

\begin{equation}
\label{eq:u_ref}
u_i^\dagger\equiv\frac{u_i}{u_{ref}}
\end{equation}

\begin{equation}
\label{eq:p_ref}
P^\dagger\equiv\frac{P}{\rho_0u_{ref}^2}
\end{equation}

\begin{equation}
\label{eq:T_ref}
T^\dagger\equiv\frac{T-T_{ref}}{\Delta T}
\end{equation}

\begin{equation}
\label{eq:x_ref}
x_i^\dagger\equiv\frac{x_i}{L_{ref}}
\end{equation}

\begin{equation}
\label{eq:t_ref}
t^\dagger\equiv\frac{t}{L_{ref}/u_{ref}}
\end{equation}

where $\dagger$ superscripts indicate nondimensional quantities.
`U` is used to specify $u_{ref}$, `T` is used to specify $T_{ref}$,
`dT` is used to specify $\Delta T$, `L` is used to specify $L_{ref}$,
`rho` is used to specify $\rho_0$, and `Cp` is used to specify $C_{p,0}$
(which does not appear above, but is necessary for scaling a volumetric heat source).
Finally, the mesh mirror must be in the same units as used in the coupled MOOSE application,
so the `scaling` parameter on [NekRSMesh](NekRSMesh.md) must be set to
dimensionalize the nondimensional `.re2` mesh. In other words,
`scaling` must be set to $L_{ref}$.

For passive scalars, NekRS allows each to be dimensionalized in a manner similar
to temperature,

\begin{equation}
s_{i}^\dagger=\frac{s_i-s_{i,ref}}{\Delta s_i}
\end{equation}

`s01`, `s02`, and `s03` are used to indicate the $s_{i,ref}$ for scalars 1,
2, and 3, respectively. `ds01`, `ds02`, and `ds03` are used to indicate the
$\Delta s_i$ for scalars 1, 2, and 3, respectively.

!alert warning
These characteristic scales are used by Cardinal to scale the NekRS solution
into the units that the coupled MOOSE application expects. *You* still need to properly
non-dimensionalize the NekRS input files. That is, you cannot
simply specify the non-dimensional scales in this class and expect a *dimensional*
NekRS input specification to be converted to non-dimensional form.

## Non-Dimensional Governing Equations

Using the non-dimensional scales listed in the prior section, the governing equations
in NekRS take the following form for mass,

\begin{equation}
\begin{aligned}
\frac{\partial u_i}{\partial x_i}=&\ 0\\
\frac{\partial(u_i^\dagger L_{ref})}{\partial (x_i^\dagger L_{ref})}=&\ 0\\
\frac{\partial u_i^\dagger}{\partial x_i^\dagger}=&\ 0\\
\end{aligned}
\end{equation}

for momentum,

\begin{equation}
\begin{aligned}
\frac{\partial u_i}{\partial t}+u_j\frac{\partial u_i}{\partial x_j}=&\ -\frac{1}{\rho_0}\frac{\partial P}{\partial x_i}+\frac{\mu_0}{\rho_0}\frac{\partial^2u_i}{\partial x_j\partial x_j}\\
\frac{\partial (u_i^\dagger u_{ref})}{\partial (t^\dagger L_{ref}/u_{ref})}+u_j^\dagger u_{ref}\frac{\partial (u_i^\dagger u_{ref})}{\partial (x_j^\dagger L_{ref})}=&\ -\frac{1}{\rho_0}\frac{\partial (P^\dagger \rho_0u_{ref}^2)}{\partial (x_i^\dagger L_{ref})}+\frac{\mu_0}{\rho_0}\frac{\partial^2(u_i^\dagger u_{ref})}{\partial (x_j^\dagger L_{ref})\partial (x_j^\dagger L_{ref})}\\
\frac{u_{ref}^2}{L_{ref}}\left(\frac{\partial u_i^\dagger }{\partial t^\dagger }+u_j^\dagger \frac{\partial u_i^\dagger }{\partial x_j^\dagger }\right)=&\ -\frac{u_{ref}^2}{L_{ref}}\frac{\partial P^\dagger }{\partial x_i^\dagger }+\frac{u_{ref}}{L_{ref}^2}\frac{\mu_0}{\rho_0}\frac{\partial^2u_i^\dagger }{\partial x_j^\dagger \partial x_j^\dagger }\\
\frac{\partial u_i^\dagger }{\partial t^\dagger }+u_j^\dagger \frac{\partial u_i^\dagger }{\partial x_j^\dagger }=&\ -\frac{\partial P^\dagger }{\partial x_i^\dagger }+\underbrace{\frac{\mu_0}{\rho_0u_{ref}L_{ref}}}_{1/Re}\frac{\partial^2u_i^\dagger }{\partial x_j^\dagger \partial x_j^\dagger }\\
\end{aligned}
\end{equation}

and for thermal energy,

\begin{equation}
\begin{aligned}
\rho_0C_{p,0}\left(\frac{\partial T}{\partial t}+u_i\frac{\partial T}{\partial x_i}\right)=&\ k_0\frac{\partial^2 T}{\partial x_i\partial x_i}+\dot{q}\\
\rho_0C_{p,0}\left(\frac{\partial (T^\dagger\Delta T+T_{ref})}{\partial (t^\dagger L_{ref}/u_{ref})}+u_i^\dagger u_{ref}\frac{\partial (T^\dagger\Delta T+T_{ref})}{\partial (x_i^\dagger L_{ref})}\right)=&\ k_0\frac{\partial^2 (T^\dagger\Delta T+T_{ref})}{\partial (x_i^\dagger L_{ref})\partial (x_i^\dagger L_{ref})}+\dot{q}^\dagger \dot{q}_0\\
\frac{\rho_0C_{p,0}\Delta Tu_{ref}}{L_{ref}}\left(\frac{\partial T^\dagger}{\partial t^\dagger }+u_i^\dagger \frac{\partial T^\dagger}{\partial x_i^\dagger }\right)=&\ k_0\frac{\Delta T}{L_{ref}^2}\frac{\partial^2 T^\dagger}{\partial x_i^\dagger \partial x_i^\dagger }+\dot{q}^\dagger \dot{q}_0\\
\frac{\partial T^\dagger}{\partial t^\dagger }+u_i^\dagger \frac{\partial T^\dagger}{\partial x_i^\dagger }=&\ \underbrace{\frac{k_0}{\rho_0C_{p,0}u_{ref}L_{ref}}}_{1/Pe}\frac{\partial^2 T^\dagger}{\partial x_i^\dagger \partial x_i^\dagger }+\dot{q}^\dagger \dot{q}_0\frac{L_{ref}}{\rho_0C_{p,0}u_{ref}\Delta T}\\
\end{aligned}
\end{equation}

The above shows that the reference scale for a volumetric heat source, $\dot{q}_0$ is selected as

\begin{equation}
\dot{q}_0=\frac{\rho_0C_{p,0}u_{ref}\Delta T}{L_{ref}}
\end{equation}

and for a heat flux $\vec{q}$ as

\begin{equation}
\begin{aligned}
\vec{q}_0=&\ \dot{q}_0L_{ref}\\
=&\ \rho_0C_{p,0}u_{ref}\Delta T
\end{aligned}
\end{equation}

For a passive scalar $s$, for simplicity it is assumed that the coefficient on the material derivative is unity (so that `diffusionCoeff` is 1) so that

\begin{equation}
\begin{aligned}
\left(\frac{\partial s}{\partial t}+u_i\frac{\partial s}{\partial x_i}\right)=&\ D_0\frac{\partial^2 s}{\partial x_i\partial x_i}+\dot{c}\\
\left(\frac{\partial (s^\dagger\Delta s+s_{ref})}{\partial (t^\dagger L_{ref}/u_{ref})}+u_i^\dagger u_{ref}\frac{\partial (s^\dagger\Delta s+s_{ref})}{\partial (x_i^\dagger L_{ref})}\right)=&\ D_0\frac{\partial^2 (s^\dagger\Delta s+s_{ref})}{\partial (x_i^\dagger L_{ref})\partial (x_i^\dagger L_{ref})}+\dot{c}^\dagger \dot{c}_0\\
\frac{\Delta su_{ref}}{L_{ref}}\left(\frac{\partial s^\dagger}{\partial t^\dagger }+u_i^\dagger \frac{\partial s^\dagger}{\partial x_i^\dagger }\right)=&\ D_0\frac{\Delta s}{L_{ref}^2}\frac{\partial^2 s^\dagger}{\partial x_i^\dagger \partial x_i^\dagger }+\dot{c}^\dagger \dot{c}_0\\
\frac{\partial s^\dagger}{\partial t^\dagger }+u_i^\dagger \frac{\partial s^\dagger}{\partial x_i^\dagger }=&\ \underbrace{\frac{D_0}{u_{ref}L_{ref}}}_{1/Pe}\frac{\partial^2 s^\dagger}{\partial x_i^\dagger \partial x_i^\dagger }+\dot{c}^\dagger \dot{c}_0\frac{L_{ref}}{u_{ref}\Delta s}\\
\end{aligned}
\end{equation}

The above shows that the reference scale for a volumetric scalar source, $\dot{c}_0$ is selected as

\begin{equation}
\dot{c}_0=\frac{u_{ref}\Delta s}{L_{ref}}
\end{equation}

and for a scalar flux $\vec{J}$ as

\begin{equation}
\begin{aligned}
\vec{J}_0=&\ \dot{c}_0L_{ref}\\
=&\ u_{ref}\Delta s
\end{aligned}
\end{equation}

## Example Input File Syntax

An example where the NekRS input files are set up in non-dimensional form,

!listing /tutorials/msfr/nek.i
  block=Problem

## All Quantities in the .i File are Dimensionalized

All quantities that appear on the Cardinal side (AuxVariables, Postprocessors,
UserObjects, etc.) will be dimensional, whereas everything in NekRS is non-dimensional.
For example, suppose your NekRS input is in non-dimensional form. Applying a
[NekVolumeAverage](NekVolumeAverage.md) postprocessor to temperature
would be used to evaluate a volume average of temperature.

!listing
[Postprocessors]
  [avg_T]
    type = NekVolumeAverage
    field = temperature
  []
[]

If the NekRS inputs are properly non-dimensionalized and the correct scales
are provided to this class, then temperature is non-dimensionalized according
to [eq:T_ref] and volume is non-dimensionalized according to [eq:x_ref], or

\begin{equation}
\label{eq:V_ref}
V^\dagger=\frac{V}{L_{ref}^3}
\end{equation}

The [NekVolumeAverage](NekVolumeAverage.md) postprocessor
is then computed directly on the NekRS solution (in non-dimensional form) to give

\begin{equation}
\label{eq:pp}
p_{nd}=\frac{\int T^\dagger dV^\dagger}{\int dV^\dagger}
\end{equation}

where $p_{nd}$ is the value of the postprocessor in non-dimensional form. Before returning the value
of the postprocessor, [eq:pp] is dimensionalized by applying the scales in [eq:T_ref]
and [eq:V_ref] to give the

\begin{equation}
\label{eq:pp_dim}
p=\Delta T \frac{\int T^\dagger dV^\dagger}{\int dV^\dagger}+T_{ref}
\end{equation}

where $p$ is the value of the postprocessor in dimensional form (which
is what is actually returned by the postprocessor).


!syntax parameters /Problem/Dimensionalize/DimensionalizeAction
