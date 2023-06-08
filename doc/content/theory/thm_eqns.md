The [Thermal Hydraulics Module (THM)](https://mooseframework.inl.gov/modules/thermal_hydraulics/index.html)
 solves for conservation of mass, momentum, and energy with 1-D area averages of the Navier-Stokes equations,

\begin{equation}
\label{eq:thm1}
\frac{\partial}{\partial t}\left(A\rho_f\right)+\frac{\partial}{\partial x}\left(A\rho_fu\right)=0\ ,
\end{equation}

\begin{equation}
\label{eq:thm2}
\frac{\partial}{\partial t}\left(A\rho_fu\right)+\frac{\partial}{\partial x}\left(A\rho_fu^2+AP\right)=\tilde{P}\frac{\partial A}{\partial x}-\frac{f}{2D_h}\rho_fu|u|A
\end{equation}

\begin{equation}
\label{eq:thm3}
\frac{\partial}{\partial t}\left(A\rho_f E_f\right)+\frac{\partial}{\partial x}\left\lbrack Au\left(\rho_fE_f+P\right)\right\rbrack=H_wa_w\left(T_\text{wall}-T_\text{bulk}\right)A
\end{equation}

where $x$ is the coordinate along the flow length, $A$ is the channel cross-sectional area, $\rho_f$ is the fluid density,
$u$ is the $x$-component of velocity, $\tilde{P}$ is the average pressure on the curve boundary, $E_f$ is the fluid total energy,
$f$ is the friction factor, $H_w$ is the wall heat transfer coefficient, $a_w$ is the heat transfer area density, $T_\text{wall}$ is the wall temperature, and $T_\text{bulk}$ is the area average bulk fluid temperature.
