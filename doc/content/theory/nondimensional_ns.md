# Non-Dimensional Formulation

NekRS can solve its equations in dimensional or non-dimensional form.
When solving in non-dimensional form,
characteristic scales for velocity,
temperature, length, and time are defined and substituted into the [Navier-Stokes equations](theory/ins.md)
so that all solution fields (velocity, pressure, temperature) are of order unity.
Non-dimensional formulations for velocity, pressure, temperature, length,
and time are defined as

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

where a $\dagger$ superscript indicates a non-dimensional quantity and a $ref$ subscript indicates
a characteristic scale.
A "$0$" subscript indicates a reference parameter (so-called because a reference parameter is not
really a characteristic scale, but rather a reference value corresponding to the conditions
at the characteristic scales). Inserting these definitions into
the conservation of mass, momentum, and energy equations solved by NekRS gives

\begin{equation}
\label{eq:mass_nondim}
\frac{\partial u_i^\dagger}{\partial x_i^\dagger}=0
\end{equation}

\begin{equation}
\label{eq:momentum_nondim}
\rho^\dagger\left(\frac{\partial u_i^\dagger}{\partial t^\dagger}+u_j^\dagger\frac{\partial u_i^\dagger}{\partial x_j^\dagger}\right)=-\frac{\partial P^\dagger}{\partial x_i^\dagger}+\frac{1}{Re}\frac{\partial \tau_{ij}^\dagger}{\partial x_j^\dagger}+\rho^\dagger f_i^\dagger
\end{equation}

\begin{equation}
\label{eq:energy_nondim}
\rho^\dagger C_p^\dagger\left(\frac{\partial T^\dagger}{\partial t^\dagger}+u_i^\dagger\frac{\partial T^\dagger}{\partial x_i^\dagger}\right)=\frac{1}{Pe}\frac{\partial}{\partial x_i^\dagger}\left(k^\dagger\frac{\partial T^\dagger}{\partial x_i^\dagger}\right)+\dot{q}^\dagger
\end{equation}

New terms in these non-dimensional equations are $Re$ and $Pe$, the Reynolds and Peclet numbers,
respectively:

\begin{equation}
\label{eq:Re}
Re\equiv\frac{\rho_0 u_{ref}L_{ref}}{\mu_0}
\end{equation}

\begin{equation}
\label{eq:Pe}
Pe\equiv\frac{L_{ref}u_{ref}}{\alpha}
\end{equation}

where $\alpha$ is the thermal diffusivity.
NekRS solves for $\mathbf u^\dagger$, $P^\dagger$, and $T^\dagger$. Cardinal will handle
conversions from a non-dimensional NekRS solution to a dimensional MOOSE application.
