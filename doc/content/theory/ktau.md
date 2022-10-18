# The k-tau RANS Model

NekRS's $k$-$\tau$ [!ac](RANS) model solves the incompressible Navier-Stokes equations with
two additional PDEs for $k$ and $\tau$.
This model includes equations governing conservation of mass, momentum,
and energy,

\begin{equation}
\label{eq:n1}
\nabla\cdot\vec{u}=0
\end{equation}

\begin{equation}
\label{eq:n2}
\rho_f\left(\frac{\partial\vec{u}}{\partial t}+\vec{u}\cdot\nabla\vec{u}\right)=-\nabla P+\nabla\cdot\left\lbrack\left(\mu_f+\mu_T\right)\nabla\vec{u}\right\rbrack+\rho_f\vec{f}
\end{equation}

\begin{equation}
\label{eq:n3}
\rho_fC_{p,f}\left(\frac{\partial T_f}{\partial t}+\vec{u}\cdot\nabla T_f\right)=\nabla\cdot\left\lbrack\left(k_f+k_T\right)\nabla T_f\right\rbrack+\dot{q}_f
\end{equation}

where $\vec{u}$ is the velocity, $\rho_f$ is the fluid density, $P$ is the pressure, $\mu_f$ is the laminar dynamic viscosity, $\mu_T$ is the turbulent dynamic viscosity, $\vec{f}$ is a general momentum source, $C_{p,f}$ is the fluid isobaric specific heat capacity, $T_f$ is the fluid temperature, $k_f$ is the laminar fluid thermal conductivity, $k_T$ is the turbulent thermal conductivity, and $\dot{q}_f$ is a general energy source. Assuming similarity between turbulent momentum and energy transfer, $k_T$ is related to $\mu_T$ through the turbulent Prandtl number $Pr_T$,

\begin{equation}
\label{eq:PrT}
\begin{aligned}
Pr_T\equiv&\ \frac{\nu_T}{\alpha_T}\\
\equiv&\ \frac{\mu_TC_{p,f}}{k_T}\ ,
\end{aligned}
\end{equation}

where $\nu_T$ is the turbulent kinematic viscosity and $\alpha_T$ is the turbulent thermal diffusivity. The $k$-$\tau$ model is a modification of the standard $k$-$\omega$ turbulence model that bases the second transport equation on $\tau$, the inverse of the specific dissipation rate $\omega$,

\begin{equation}
\label{eq:tau}
\tau\equiv\frac{1}{\omega}\ .
\end{equation}

The equation for the turbulent kinetic energy $k$ is a model equation based on a gradient diffusion approximation for the turbulent transport and pressure diffusion terms in the *true* $k$ equation, which itself is derived by taking the trace of the Reynolds stress equation. The $k$ equation in NekRS is

\begin{equation}
\label{eq:n6}
\frac{\partial\left(\rho_fk\right)}{\partial t}+\nabla\cdot\left(\rho_fk\vec{u}\right)=\nabla\cdot\left\lbrack\left(\mu_f+\frac{\mu_T}{\sigma_k}\right)\nabla k\right\rbrack+\mathscr{P}-\rho_f\beta^*\frac{k}{\tau}\ ,
\end{equation}

where $\sigma_k$ and $\beta^*$ are constants and $\mathscr{P}$ is the production of turbulent kinetic energy by velocity shear. The $\tau$ equation is obtained by inserting [eq:tau] into the $\omega$ model equation of the $k$-$\omega$ equation, giving

\begin{equation}
\label{eq:n7}
\frac{\partial\left(\rho_f\tau\right)}{\partial t}+\nabla\cdot\left(\rho_f\tau\vec{u}\right)=\nabla\cdot\left\lbrack\left(\mu+\frac{\mu_T}{\sigma_\tau}\right)\nabla\tau\right\rbrack-\alpha\frac{\tau}{k}\mathscr{P}+\rho_f\beta-2\frac{\mu_f}{\tau}\nabla\tau\cdot\nabla\tau\ ,
\end{equation}

where $\sigma_\tau$, $\alpha$, and $\beta$ are constants. The turbulent dynamic viscosity is then related in terms of $k$ and $\tau$ as

\begin{equation}
\label{eq:mu_ktau}
\mu_T=\rho_fk\tau\ .
\end{equation}
