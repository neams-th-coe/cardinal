# Tutorial 2A: LWR Pincell

In this tutorial, you will learn how to:

- Couple OpenMC via temperature and heat source feedback to MOOSE
- Control the resolution of physics feedback received by OpenMC
- Tally an OpenMC heat source on an unstructured mesh

This tutorial describes how to use Cardinal to perform temperature and heat
source coupling of OpenMC to MOOSE for a [!ac](LWR) pincell. While the entire
domain is modelled by OpenMC, the MOOSE domain only consists of the solid regions.
After providing the foundation in this tutorial,
in [Tutorial 3](openmc_fluid.md) we will describe how to introduce both
temperature and density feedback from MOOSE to OpenMC. Finally, in
[Tutorial 4](coupled.md), we will combine the content of the previous tutorials
to couple nekRS, OpenMC, and MOOSE heat conduction for fully-coupled multiphysics.

## Geometry and Computational Model

This section describes the geometry for a [!ac](LWR) pincell; the relevant dimensions
are summarized in [table1]. The pincell consists of a UO$_2$ pincell within a Zircaloy
cladding; a helium gap separates the fuel from the cladding. Borated water
is present outside the cladding.

!table id=table1 caption=Geometric and operating conditions for a [!ac](LWR) pincell
| Parameter | Value |
| :- | :- |
| Pellet outer radius | 0.39218 cm |
| Clad inner radius | 0.40005 cm |
| Clad outer radius | 0.45720 cm |
| Pin pitch | 1.25984 cm |
| Height | 300 cm |

Heat is produced in the pellet region and transfers by radiation and conduction across
the pellet-clad gap to the cladding. A total core power of 3000 MWth is assumed uniformly
distributed among 273 fuel bundles, each with 17$\times$17 pins. The tallies from OpenMC
will therefore be normalized according to a pin-wise power of

\begin{equation}
\label{eq:1}
q_{pin}=\frac{3000\text{\ MWth}}{n_bn_p}
\end{equation}

where $n_b=273$ is the number of fuel bundles and $n_p=289$ is the number of pins
per bundle.

## Boundary Conditions

This section describes the boundary conditions imposed on the Monte Carlo particle transport model and
the MOOSE heat conduction model.

### Neutronics Boundary Conditions

For the neutronics physics, the top and bottom of the pincell are assumed vacuum
conditions. For the four lateral faces of the pincell, reflective boundaries are used.

### Solid Boundary Conditions

Because heat transfer and fluid flow in the borated water is not modeled in this example,
we approximate the effect of heat removal by the fluid by setting
the outer surface of the cladding to a convection boundary
condition,

\begin{equation}
\label{eq:2}
q^{''}=h\left(T-T_\infty\right)
\end{equation}

where $h=1000$ W/m$^2$/K and $T_\infty=280$&deg;C. The top and bottom of the solid pincell
are assumed insulated.

The gap region between the pellet and the cladding is unmeshed, and a quadrature-based
thermal contact model is applied based on the sum of thermal conduction and thermal radiation
(across a transparent medium).
For a paired set of boundaries,
each quadrature point on boundary A is paired with the nearest quadrature point on boundary B.
Then, the sum of the radiation and conduction heat fluxes imposed between pais of
quadrature points is

\begin{equation}
\label{eq:3}
q^{''}=\sigma\frac{T^4-T_{gap}^4}{\frac{1}{\sigma_A}+\frac{1}{\sigma_B}-1}+\frac{T-T_{gap}}{r_{th}}
\end{equation}

where $\sigma$ is the Stefan-Boltzmann constant, $T$ is the temperature at a quadrature
point, $T_{gap}$ is the temperature of the nearest quadrature point across the gap,
$\sigma_A$ and $\sigma_B$ are emissivities of boundaries A and B, respectively, and
$r_{th}$ is the conduction resistance. For cylindrical geometries, the conduction
resistance is given as

\begin{equation}
\label{eq:4}
r_{th}=\frac{ln{\frac{r_2}{r_1}}}{2\pi L k}
\end{equation}

where $r_2>r_1$ are the radial coordinates associated with the outer and inner radii
of the cylindrical annulus, $L$ is the height of the annulus, and $k$ is the
thermal conductivity of the annulus material.

## Initial Conditions

The initial temperature is 280&deg;C, while the initial heat source in solid domain is zero.

## Meshing

This section describes the mesh used for the solid domain. MOOSE
[MeshGenerators](https://mooseframework.inl.gov/syntax/Mesh/index.html) are used to construct
the solid mesh. The mesh with block IDs and sidesets is shown in [solid_mesh].

!media pincell_solid_mesh.png
  id=solid_mesh
  caption=Mesh for the solid portions of a [!ac](LWR) pincell

## CSG Geometry

This section describes the [!ac](CSG) model setup in OpenMC. In the near future,
OpenMC will have unstructured mesh tracking capabilities - but until that is available,
OpenMC geometries all use the [!ac](CSG) geometry approach, where cells are created
from half-spaces of various common surfaces.

Because this is a Cardinal
tutorial, we assume you have some basic familiarity with OpenMC, so we only discuss the
portions of the model setup relevant to multiphysics feedback. When creating the OpenMC
geometry, there are two aspects that you must pay attention to when using Cardinal -

- The resolution of the temperature (and density, for fluid feedback) feedback to impose in OpenMC
- The "level" of the cells with which you want to perform feedback

The temperature in OpenMC is stored on the [!ac](CSG) cells. One constant temperature
can be set for each cell. Therefore, the resolution of the temperature feedback
received from MOOSE is determined during the OpenMC model setup. Each
OpenMC cell will receive a unique temperature, so the number of OpenMC cells dictates
this feedback resolution.

The second consideration is slightly more subtle, but allows great flexibility for
imposing multiphysics feedback for very heterogeneous geometries, such as
[!ac](TRISO) pebbles. The "level" of a [!ac](CSG) cell refers to the number of
nested universes (relative to the root universe) at which you would like to impose
feedback. If you construct your geometry without *filling* any OpenMC cells with
other universes, then all your cells are at level zero - i.e. the highest level in
the geometry. But if your model contains lattices, the level that you want to perform
multiphysics coupling on is most likely not the highest level in the geometry. For instance,
[!ac](LWR) core, comprised of hundreds of assemblies. Your approach to creating the
geometry would probably be to make a single fuel assembly universe, then repeat that
lattice several times throughout the geometry. If your assembly universe wasn't itself
filled with any universes, then all your cells of interest are actually at level 1.
We will illustrate this lattice concept with a set of [!ac](TRISO) pebbles
in [Tutorial 2B](triso.md).

!alert note
We *highly* recommend running Cardinal with `verbose = true` when setting up
your OpenMC coupling. This setting will display the mapping of OpenMC cells to
MOOSE elements and should help provide a grasp on the "level" concept.

OpenMC's Python [!ac](API)
is used to create the model with the script shown below. First, we define
materials for the various regions and create the geometry. With respect to the
first important consideration when setting up the OpenMC model - we manually add
ten cells to receive solid temperature feedback by dividing the entire axial
height by eleven axial planes. And with respec to the second important consideration,
because we are not filling any universes with other universes or lattices,
all of the cells in this problem are at the highest level of the geometry -
i.e. the root universe. Later in the OpenMC wrapping, we will need to provide
this information, so we simply make a note of it here.

!listing /tutorials/lwr_solid/make_openmc_model.py

!alert note
For obtaining *temperature* feedback in OpenMC, we simply need to create unique
cells to receive the temperature. We do not need to create unique materials, for
instance.
