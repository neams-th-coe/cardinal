# Poiseuille Flow (Traction Boundaries)

This case verifies the traction boundary-condition implementation in NekRS using fully developed Poiseuille flow in a half-channel.
The nondimensional channel height is $H=1$, and periodic boundary conditions are imposed in the streamwise $x$ and spanwise $z$ directions.
A traction boundary condition is imposed at $y=-1$, while a symmetry boundary condition is imposed at $y=0$.

## Governing equation

For steady, fully developed flow, the streamwise momentum equation reduces to

!equation id=eq\:shl-channel-momentum
\frac{1}{Re}\frac{d^2u}{dy^2}=\frac{dp}{dx},

where $u$ is the streamwise velocity, $Re$ is the Reynolds number, and $dp/dx$ is the imposed streamwise pressure gradient.

## Analytical solution

The analytical velocity profile is

!equation id=eq\:shl-channel-velocity
u(y)=1.5\left(1-y^2\right).

The corresponding streamwise body force is

!equation id=eq\:shl-channel-body-force
f_x=\frac{3}{Re}.

The wall traction is defined using the outward unit normal component $n_y=-1$ at the lower boundary,

!equation id=eq\:shl-channel-traction
\tau_w
=
\left.\frac{1}{Re}\frac{du}{dy}n_y\right|_{y=-1}
=
-\frac{3}{Re}.

The symmetry condition at $y=0$ is satisfied because the velocity gradient vanishes at the channel centerline,

!equation id=eq\:shl-channel-symmetry
\left.\frac{du}{dy}\right|_{y=0}=0.

## Verification criteria

Two CI modes are provided for this case.
CI mode 1 uses the channel geometry described above.
CI mode 2 verifies the same solution on a geometry rotated by $45^\circ$.
Both CI modes are qualified by evaluating the volume-integrated error norm of the velocity field with respect to the analytical Poiseuille solution,

!equation id=eq\:shl-channel-velocity-error
\varepsilon_{\boldsymbol{u}}
=
\left\lVert\boldsymbol{u}-\boldsymbol{u}_{\mathrm{exact}}\right\rVert_{L_2(\Omega)}
=
\left[
\int_\Omega
\left|\boldsymbol{u}-\boldsymbol{u}_{\mathrm{exact}}\right|^2
\,d\Omega
\right]^{1/2}.
