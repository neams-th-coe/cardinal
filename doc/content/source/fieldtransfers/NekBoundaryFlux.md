# NekBoundaryFlux

## Description

`NekBoundaryFlux` is a [FieldTransfer](AddFieldTransferAction.md) that sends a boundary flux between NekRS and MOOSE. First, this object
creates an [AuxVariable](AuxVariable.md) using the name of the object; this variable will hold
the flux which NekRS reads (for `direction = to_nek`) or will be written by NekRS (for
`direction = from_nek`).

## Flux Normalization

The internal NekRS mesh is based on the spectral element method (with Guass-Lobatto
quadrature), whereas a coupled MOOSE applcation which wants to write/use a boundary flux will
likely be using a different quadrature rule.
In order to ensure perfect conservation of the
flux as it is exchanged between NekRS and MOOSE, a [Receiver](Receiver.md) postprocessor is automatically added with
name taken as `<name of the object>_integral`. For `direction = to_nek`, this postprocessor
should be populated by a parent/sub application with the total integrated flux in the originating
MOOSE application. For `direction = from_nek`, this postprocessor will be filled by the
total integrated flux computed internally in NekRS.

### Flux Normalization

By default, `NekBoundaryFlux`
will lump all "receiving" sidesets in NekRS together for the purpose of flux normalization.
For example, suppose we are coupling NekRS to MOOSE heat conduction through two boundaries.
Suppose MOOSE heat conduction predicts the following heat flux integrals on each of these
boundaries:

- MOOSE boundary 1: 100.0 W
- MOOSE boundary 2: 200.0 W

When this data gets mapped to NekRS's spectral element mesh, we need to renormalize to ensure
that we conserve power. This renormalization is necessary because the NekRS mesh can be entirely
different from the MOOSE mesh, and integrating a nodal field on the origin mesh from MOOSE
(say, a 1-st order Lagrange interpolation) will give a different integral value than
integrating the interpolated nodal values on the receiving NekRS mesh (say, a 7-th order
Lagrange interpolation of [!ac](GLL) points). By default, `NekBoundaryFlux` lumps all receiving
sidesets in the NekRS model together for the sake of normalization. For example, suppose
that once received on the NekRS mesh and integrated, that the received flux has values of:

- NekRS boundary 1: 102.0 W
- NekRS boundary 2: 199.0 W

By default, this class will renormalize the NekRS flux in order to match the *total* MOOSE
flux (of 300.0 W) to give:

- NekRS boundary 1: $102.0 * \frac{100.0 + 200.0}{102.0 + 199.0}\equiv 101.66$
- NekRS boundary 2: $199.0 * \frac{100.0 + 200.0}{102.0 + 199.0}\equiv 198.34$

The *total* power entering the flux is preserved, but not the distribution among the sidesets.
This is usually a *very small* error and is an acceptable approximation for many analyses.
However, we do also support an option where the heat flux on each sideset is preserved with
the `conserve_flux_by_sideset` option. When setting this parameter to true, the heat flux
on each NekRS sideset will instead be evaluated as:

- NekRS boundary 1: $102.0 * \frac{100.0}{102.0}\equiv 100.0$
- NekRS boundary 2: $199.0 * \frac{200.0}{199.0}\equiv 200.0$

This option requires using a vector postprocessor (we typically recommend the
[VectorOfPostprocessors](VectorOfPostprocessors.md)
object) to send indiviudal boundary flux values into your NekRS-wrapped case. This more advanced option
cannot be used if:

- Your sidesets in the coupled MOOSE App are not individually specified. For instance,
  if your heat conduction solver has one sideset that maps to *two* NekRS sidesets,
  there's no natural way to figure out what the heat flux is in those sub-sidesets
  to send a correct value to NekRS.
- Any nodes are shared among the NekRS sidesets, such as a node on a corner between two
  sidesets. When we renormalize the NekRS flux, nodes that are present on more than
  one sideset will get renormalized multiple times, so there is no guarantee that we
  can enforce the total flux.

### What are the Fluxes?

There are a few different heat fluxes involved when coupling NekRS via [!ac](CHT)
to MOOSE. When you run a [!ac](CHT) calculation, for each time step you will see
something like the following printed to the screen:

!listing!
Sending heat flux to NekRS boundary 1
Normalizing total NekRS flux of 78.8581 to the conserved MOOSE value of 78.6508
!listing-end!

Here, the "total NekRS flux of ..." is the integral of `nrs->usrwrk` over the
high-order NekRS spectral element mesh. Conversely, the "conserved MOOSE value of ..."
is the total heat flux that MOOSE is setting in NekRS (read from the `<name_of_object>_integral`
postprocessor that this object creates. These two numbers will be
different unless all of the following are true:

- The NekRS and MOOSE meshes are identical
- The NekRS polynomial order matches the MOOSE polynomial order, and for the same
  polynomial order the nodes are the same. Because NekRS is a spectral element method
  and MOOSE uses the finite element method, this criteria can only occur if NekRS and
  MOOSE are either 1st or 2nd order (because the node placement for 1st or 2nd order
  elements is the same for spectral and finite elements).
- The quadrature rule used to integrate in MOOSE is the [!ac](GLL) quadrature

Any differences between the "total NekRS flux" and the "conserved MOOSE value" will
simply arise due to differences in the integration of a field over their respective meshes,
and is not anything of concern - in fact, Cardinal is specifically designed to renormalized
so that conservation is maintained.
However, you may see large differences if the geometry is curved, since the NekRS high-order
mesh will capture the curvature and result in very different area integrals.
We always recommend doing a sanity check on the flux sent from MOOSE to NekRS
via the MOOSE output files.

You can also monitor the heat flux in NekRS by computing $-k\nabla T\cdot\hat{n}$ using a
[NekHeatFluxIntegral](NekHeatFluxIntegral.md) postprocessor. In general, the quantity
computed by this postprocessor will *not* match the heat flux set by MOOSE because
both the finite and spectral element methods solve weak forms where Neumann boundary
conditions are only *weakly* imposed. That is, we set the heat flux but only enforce
it by driving the entire nonlinear residual to a small-enough number, so heat flux
boundary conditions are never perfectly observed like Dirichlet boundary conditions are.

## Example Input File Syntax

As an example, the example below couples NekRS to MOOSE via [!ac](CHT). A coupled MOOSE
application sends a heat flux variable and a postprocessor representing the total flux
integral into a NekRS-wrapped case. The `NekBoundaryFlux` object creates automatically
an auxiliary variable named `avg_flux` and a postprocessor by the name of `flux_integral`
(the default postprocessor name would have been `avg_flux_integral` but you can also
specify a custom name if desired as shown here). The `NekBoundaryFlux` object then handles
writing this heat flux into usrwrk slot 0.

!listing tests/cht/pebble/nek.i
  block=Problem

Then, all that is required to actually _apply_ this heat flux to the NekRS model is to use
it in the `scalarNeumannConditions` boundary condition.
Below, `bc->usrwrk` is the same as `nrs->o_usrwrk`, or the scratch space on the
device; this function applies the heat flux computed by MOOSE to the flux boundaries.

!listing /test/tests/cht/pebble/onepebble2.oudf language=cpp
  re=void\sscalarNeumannConditions.*?^}

!syntax parameters /Problem/FieldTransfers/NekBoundaryFlux

!syntax inputs /Problem/FieldTransfers/NekBoundaryFlux
