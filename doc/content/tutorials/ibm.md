# Immersed Boundary Methods with NekRS

In this tutorial, you will learn:

- The basic concepts of immersed boundary methods
- How to build a flow simulation in NekRS using the [!ac](IBM)

To access this tutorial,

```
cd cardinal/tutorials/ibm_sphere
```

!alert warning
Immersed boundary methods are a new feature available in NekRS and the API and dependencies are likely
to change in the future to simplify the workflow. This tutorial was
designed using the simplest set of dependencies (only VTK), whereas
more advanced feature sets are described in [!cite](hegazy_ibm).

## Immersed Boundary Methods

The [!ac](IBM) was introduced in the early 1970s by Peskin, originally formulated for deformable boundaries with applications to biological flows at low Reynolds number [!cite](peskin).
Since this time, there have been hundreds of papers published on the method, with many variants and different applications. IBMs have been developed for most major classes of spatial discretizations (e.g., finite difference [!cite](fadlun,ikeno),
finite volume [!cite](wongkham), spectral methods [!cite](goldstein,hegazy_thesis),
finite elements [!cite](cervone) and turbulence models (e.g. [!ac](LES) [!cite](hegazy_thesis,ikeno),
[!ac](RANS) [!cite](cervone). Excellent overviews of the history of the
field can be found in [!cite](Mittal2005, Kim2019, articlemittal2022, verzicco2023immersed, PhysRevFluids.8.100501, hegazy_thesis). In this tutorial, only a
general overview of IBMs is provided to outline general common features with a
special focus on the spectral element implementation in NekRS. Full details
on the algorithmic implementation in NekRS can be found in [!cite](hegazy_thesis),
with a description on how to use the IBM given in [#alg].

The basic concept of an IBM seeks to solve for the flow equations on an
Eulerian grid with points $\vec{x}$ in the presence of an immersed body of
Lagrangian points $\vec{x}_L$ that may not coincide with the Eulerian grid.
This is shown in [ibm_geometry_overview] (left). IBMs are complementary to body-fitted
simulations, and useful for higher throughput modeling for design within large parameter spaces (while still giving excellent agreement with body-fitted scenarios in terms of both local and integral quantities). The effort to develop body-fitted meshes, if desired, can then be reserved for mature design concepts.

!media ibm_geometry_overview.png
  id=ibm_geometry_overview
  caption=Notational definitions for IBMs. The left image shows a spectral element with polynomial order 4 and [!ac](GLL) quadrature.
  style=width:100%;margin-left:auto;margin-right:auto

Many formulations of IBMs are based on a *forcing* technique, wherein an
additional body force is added to the momentum equation in order for the fluid to feel the presence of the surface. When a fluid flows along a no-slip surface, the fluid exerts pressure and viscous forces on the wall. An equal and opposite force is exerted on the fluid by the wall and it is this localized force that acts on the fluid to enforce the no-slip boundary conditions (this is physically analogous, albeit formulated in a weak sense, to strong imposition of no-slip boundary conditions). However, this force is not known *a priori*, and if used as a boundary condition must be determined iteratively using the flow solution.
The mass and momentum equations for an incompressible fluid with constant viscosity is

\begin{equation}
\label{eq:mom}
\nabla\cdot\vec{V}=0,\hspace{1cm}\rho\left(\frac{\partial \vec{V}}{\partial t}+\vec{V}\cdot\nabla\vec{V}\right)=-\nabla P+\mu\nabla^2\vec{V}+\rho \vec{f}+\rho\vec{f}\ '
\end{equation}

where $\vec{V}$ is the velocity, $P$ is the pressure, $\mu$ is the viscosity, and $\rho$ is the density. The body forces are distinguished between the term added for the IBM methodology ($\vec{f}'$) and other possible body forces ($\vec{f}$) present depending on the problem (such as a Boussinesq term for buoyancy or a Lorentz force term for magnetohydrodynamics).
IBMs largely differ in their computation of the force $\vec{f}\ '$ and how they relate this point-force (on the Lagrangian nodes) to the Eulerian grid.
In the most general formulation, the velocity on the Lagrangian points is not a degree of freedom in the solve, and must be obtained using information from the Eulerian field. Likewise, the point forces on the Lagrangian nodes must be spread back to the Eulerian nodes where Eq. \eqref{eq:f} is defined.
In other words, $\vec{f}\ '$ is only nonzero on the Lagrangian points,

\begin{equation}
\label{eq:fd}
\vec{f}\ '(\vec{x},t)=\int_\Gamma \vec{F}_L(\vec{x}_L,t)\delta(\vec{x}-\vec{x}_L)dS
\end{equation}

where $\vec{F}_L$ is the Lagrangian force density and $\delta$ is a three-dimensional delta function.

### Definition of the Force Term

In this section, the issue of the point forces not coinciding with Eulerian [!ac](DOFs) is momentarily ignored (so that in the notation, you may consider $\vec{f}\ '$ to be the same as $\vec{F}_L$, i.e., Eulerian points happen to coincide with the Lagrangian points). Different techniques to relate the Lagrangian and Eulerian points, when they do not coincide.

Force-based IBMs broadly fall into two categories, of (i) continuous and (ii) discrete methods. Continuous methods formulate the additional force prior to discretization, as shown in Eq. \eqref{eq:mom} and differ in their selection of the structure of the body force $\vec{f}\ '$. One general formulation, when the desired surface velocity and surface position are known, can be written as 

\begin{equation}
\label{eq:f}
\vec{f}\ '(\vec{x}_L,t)=&\ \alpha\int_0^t\left\lbrack\vec{V}(\vec{x}_L,t')-\vec{V}_L(t')\right\rbrack dt'+\beta \left\lbrack\vec{V}(\vec{x}_L,t)-\vec{V}_L\right\rbrack+\gamma\frac{\pl \left\lbrack\vec{V}(\vec{x}_L,t)-\vec{V}_L\right\rbrack}{\pl t}+\\
&\ \zeta \norm{\vec{V}(\vec{x}_L,t)-\vec{V}_L}\left\lbrack\vec{V}(\vec{x}_L,t)-\vec{V}_L\right\rbrack+\ \cdots
\end{equation}

where $\vec{V}(\vec{x}_L,t)$ is the Eulerian velocity field, evaluated at a Lagrangian point. $\vec{V}_L$ is the desired Lagrangian surface velocity at the point $\vec{x}_L$. In general, the Eulerian velocity and the Lagrangian velocity will differ at the points on the immersed surface, unless the immersed boundary technique has perfectly enforced the no-slip boundary condition. Hence, the force terms are proportional to $\vec{V}_L-\vec{V}(\vec{x}_L)$ such that any discrepancy generates a force that aims to drive the velocities to match. In the case of a stationary boundary, $\vec{V}_L=0$, otherwise the boundary nodes $\vec{x}_s$ themselves must move. The coefficients $\alpha$, $\beta$, $\gamma$, and $\zeta$ are negative. The first two terms (in absence of the other non-immersed-boundary force terms in Eq. \eqref{eq:mom}) reduce the Navier-Stokes equations to a damped harmonic oscillator [!cite](goldstein). Additional terms involving spatial integrals and derivatives have also been posited [!cite](goldstein), which are indicated above as ``$\cdots$.''

In scenarios with moving boundaries, such as those pioneered by Peskin and colleagues, the transmitted force is computed from the internal body forces of the moving structure (approximated as collections of linked nodes considering internal stresses and strains) [!cite](peskin). Other types of surface boundary conditions, such as symmetry conditions, can also be formulated as forces.

Various immersed boundary formulations utilize different selections for the coefficients. For example, Goldstein et al., in the first use of immersed boundary techniques with spectral methods, selects $\alpha$ and $\beta$ according to stability constraints on the numerical integration scheme selected for the time integral term in Eq. \eqref{eq:f} in conjunction with transient considerations such that the damped harmonic oscillator frequency is faster than the fastest flow timescales (so that the external force can respond to the changing flow) [!cite](goldstein), and hence are problem-dependent. Hegazy instead selects $\gamma=1$ for strong feedback forcing [!cite](hegazy_thesis).

A particular sub-class of continuous forcing methods, which appear in the literature by a variety of names, could be referred to as porous-IBM approaches. In these methods, the coefficients $\beta$ and $\zeta$ are selected so as to reflect a Darcy-Forchheimer drag in terms of the local porosity in each element. For example,
in the approach developed by [!cite](dai_2026, an external force with $\beta=-C(1-\epsilon)^2 /(\epsilon^3+10^{-3})$ (i.e., only the linear damping term) is added to the momentum equation in all elements.
The form of the forcing term reflects the $(1-\epsilon)^2/\epsilon^3$ proportionality of the normalized pressure drop in porous media [!cite](novak_thesis),
with a tunable coefficient $C$ and a stability parameter $10^{-3}$ to enforce very high but finite values of the force in solid elements ($\epsilon=0$). Another approach from Wongkham et al. applies an additional adaptive mesh refinement in the cut elements to further refine the sharp interface in porosity [!cite](wongkham) but in this case selects nonzero, porous-media-inspired, coefficients for both $\beta$ and $\zeta$.
Note that these approaches differ from traditional porous media methods because the porosity is evaluated by overlaying the solid geometry in the fluid elements, and therefore varies from element to element (as opposed to varying in a smooth sense corresponding to a representative elementary volume [!cite](novak_thesis)).

In discrete forcing methods, the required force is instead formulated from the spatially- and temporally-discretized equations themselves rather than imposing a heuristic form as in Eq. \eqref{eq:f}. For example, a discretized form of Eq. \eqref{eq:mom} can be written as

\begin{equation}
\label{eq:df}
\frac{\vec{V}_i^{n+1}-\vec{V}_i^n}{\Delta t}=\text{RHS}_i^{n+1}+(\vec{f}\ ')_i^{n+1}
\end{equation}

where $i$ indicates the grid point index, $n$ indicates the time step index, a first-order implicit Euler discretization is used for the time term, and all the details of the discretization of the right-hand side (the pressure, viscous, and body force terms) is written in shorthand as RHS$^{n+1}$. If the Eulerian point happened to coincide with the Lagrangian surface on which the velocity is known, then Eq. \eqref{eq:df} could be solved (e.g., by replacing $\vec{V}_i^{n+1}$ with zero for a no-slip stationary surface) to obtain the required forcing $(\vec{f}\ ')_i^{n+1}$ at the grid point. This is essentially the structure of many discrete forcing methods [!cite](fadlun,ikeno). Of course, the Eulerian points will in general not coincide with the Lagrangian points. An advantage of this discrete forcing technique is the absence of user-defined parameters.

Good agreements with canonical and experimental flow problems have been obtained with both continuous and discrete forcing approaches [!cite](goldstein,fadlun), though there are differences in terms of stability and implications related to selecting problem-dependent coefficients. 
Many other numerical techniques are related to these force-based IBMs, including shifted boundary methods and cut cell methods.

### Connection Between Lagrangian and Eulerian Points

A commonality to force-based IBMs
are the spreading and interpolation operators needed to evaluate
$\vec{V}(\vec{x}_L,t)$.
The simplest approach is to simply apply the force at the Eulerian points as if they were coincident with the Lagrangian points [!cite](fadlun); that is, assign forces to the Eulerian points by simply reading the force at the nearest Lagrangian point (or, conversely, by simply evaluating the force at the Eulerian point as if it were a Lagrangian point). This is effectively the approach taken in many finite volume solvers, particularly the porous-IBm approaches [!cite](wongkham,dai_2026), where information regarding the distance to the immersed surface is not directly used in the force computation. This approach is also sometimes used in higher-order solvers as well; for example, Cervone et al. [!cite](cervone) apply a Galerkin projection to a nodal Heaviside field (unity inside the solid, zero in the fluid) to obtain a pointwise differentiable field $\alpha$. The unit inward normal of the immersed surface is then computed as the average value of $\|\nabla\alpha\|$ over the cut element and the approximate position of the immersed interface snapped to the closest node for which $\alpha=1$.

More accurate techniques apply various forms of interpolation to construct the Lagrangian values from the Eulerian data. For instance, Hegazy applies an interpolation to obtain Lagrangian values using the same Lagrange polynomial basis functions used to represent the spectral solution [!cite](hegazy_thesis). Other approaches use a smoothed function $\mathscr{D}$ that weights the Eulerian grid points by their distance to the Lagrange point, 

\begin{equation}
\label{eq:d}
\vec{V}(\vec{x}_L,t)=\sum_\text{g}\mathscr{D}(\vec{x}_g-\vec{x}_L)\vec{V}(\vec{x}_g,t)
\end{equation}

where $\mathscr{D}$ is a three-dimensional product of 1-D functions such as Gaussians [!cite](peskin,goldstein) (the symbol $\mathscr{D}$ is used to indicate a smoothed function rather than a discrete delta function $\delta$ as in Eq. \eqref{eq:fd}) and $g$ are all the Eulerian grid points each of which is located at the point $\vec{x}_g$ that contribute (discussed shortly).

Once the force is computed on the Lagrangian points, a ``spreading'' operator is used to send these forces back to the Eulerian points to transform Eq. \eqref{eq:fd} into a form useful for implementation:

\begin{equation}
\label{eq:d2}
\vec{f}\ '(\vec{x}_g)\approx \sum_l\vec{F}_L(\vec{x}_L,t)\mathscr{D}(\vec{x}_g-\vec{x}_L)\Delta A_l \Delta S_l
\end{equation}

where $l$ are all the Lagrangian nodes that the Eulerian node $g$ is communicating with, and $\Delta A_l$ and $\Delta S_l$ are area and length scales associated with the surface patch that the Lagrangian point $l$ corresponds to (full details in [!cite](hegazy_thesis)) in order to obtain the correct units on $\vec{f}\ '$. For example, this spreading operator can use the same weighting function as used in Eq. \eqref{eq:d}.
For both the interpolation and spreading operators, the Eulerian points $g$ that communicate with a given Lagrangian point (for interpolation) and the Lagrangian points $l$ that communicate with a given Eulerian point (for spreading) can, in the simplest case, be taken as all the Eulerian and Lagrangian points within a cut element, respectively [!cite](hegazy_thesis). 
Using a larger stencil that extends beyond the cut element is more accurate [!cite](hegazy_thesis)
, but comes at additional complexity for parallel communication when combined with domain decomposition.

In the context of the discrete forcing approach, another option is to use a multidimensional linear interpolation to find the value of $\vec{V}_i^{n+1}$ (at the Eulerian point of interest) that is a linear scaling from the imposed Lagrangian velocity on the nearby surface and a ``virtual point'' along the line, in other words such that the force at the Eulerian point $(\vec{f}\ ')_i^{n+1}$ accounts for the fact that this DOF is not on the immersed surface [!cite](fadlun,ikeno).

### Treatment of the Solid Region

Quite different treatments are also performed for the [!ac](DOFs) within the interior of the body. Since the velocity within a stationary body should be zero, the simplest option is to treat these nodes as strongly-imposed Dirichlet conditions of zero velocity [!cite](cervone). Another
option is to simply retain the same continuous forcing term as in Eq. \eqref{eq:f} [!cite](hegazy_thesis) or to set $\vec{V}_i^{n+1}=0$ as in Eq. \eqref{eq:df} [!cite](ikeno,fadlun).
Alternatively, these internal points could be applied zero force altogether, with the flow allowed to freely develop [!cite](fadlun). A different approach is to apply the same spreading operator as used exterior to the body, but applied inwards to mirror the force on the interior as seen in the fluid [!cite](hegazy_thesis). Sometimes various force techniques are combined with an increase in the fluid viscosity to discourage internal flows [!cite](hegazy_thesis) (this is effectively the outcome in the hybrid porous-immersed techniques wherein $\epsilon=0$ results in infinite resistance [!cite](wongkham), though the latter is done automatically as opposed to an additional modification to the fluid's properties).
Treatments may manipulate the velocity in other ways, such as by applying a reversed flow (with respect to the fluid domain) so as to minimize energy in the highest wavenumbers for stability purposes [!cite](goldstein). For [!ac](rans) models, modified forms of the additional transport equations are typically employed at grid points in the solid [!cite](cervone).

Many applications of IBMs apply these considerations to {\it all} [!ac](DOFs) interior to the solid body, which can add unnecessary expense when the solid comprises a large portion of the overall volume [!cite](fadlun). One solution is to remove elements fully interior to the domain as a pre-processing step, leaving only fluid and interface elements in the solve [!cite](hegazy_thesis). This is shown in the third image in Fig. \ref{fig:geometry}, where elements interior to pebbles have been deleted. This has the added benefit of enhancing mass conservation, by eliminating some possible internal flows that draw mass from the fluid.

## Geometry

This model consists of flow over a sphere with parameters summarized in [table1]. All
quantities are given in non-dimensional form.

!table id=table1 caption=Geometry and simulation parameters to be used for the flow over a sphere.
| Parameter | Value (cm) |
| :- | :- |
| Sphere diameter | 1.0 |
| Sphere center position | $(0, 0, 0)$ |
| Domain vertical extent | $-10\leq z\leq 30$ |
| Domain width | $ 8 \times 8$ |
| Reynolds number | 100 |

## Dependencies

In order to use the immersed boundary method within NekRS, there is some additional software you will need to install.

### VTK
id=vtk

[!ac](VTK) is used for optional pre-processing the mesh to delete solid elements (step 3 in [#alg]),
as well as an option for the signed distance field calculation used to relate the Eulerian and
Lagrangian points. Instructions to install [!ac](VTK) version 9.3.0 into a folder at `${HOME}/vtk/install`
are below (this is just an example, as you can use newer versions as well).

```
cd $HOME
mkdir -p vtk/source vtk/build vtk/install
cd vtk

wget https://www.vtk.org/files/release/9.3/VTK-9.3.0.tar.gz
tar -xzf VTK-9.3.0.tar.gz --strip-components=1 -C source

cd build

cmake -DCMAKE_INSTALL_PREFIX=$HOME/vtk/install \
      -DCMAKE_BUILD_TYPE=Release \
      -DVTK_USE_MPI=ON \
      -DVTK_MODULE_ENABLE_VTK_IOGeometry=YES \
      -DVTK_MODULE_ENABLE_VTK_FiltersHybrid=YES \
      -DVTK_MODULE_ENABLE_VTK_RenderingCore=YES \
      -DVTK_MODULE_ENABLE_VTK_RenderingOpenGL2=NO \
      -DVTK_MODULE_ENABLE_VTK_RenderingFreeType=NO \
      -DVTK_MODULE_ENABLE_VTK_RenderingContext2D=NO \
      -DVTK_BUILD_TESTING=OFF \
      ../source

make -j12
make install
```

Add the following to your bashrc (be sure to change `VTK_ROOT` to where you have installed VTK, if not in your home directory):

```
export VTK_ROOT="$HOME/vtk"
export LD_LIBRARY_PATH="${VTK_ROOT}/install/lib"
```

[!ac](VTK) is also used for some routines during the flow solve. As such, you will need to make
[!ac](VTK) accessible to NekRS's `.udf` files. In the directory with the case
files, you will see a `udf.cmake` file. Note that you may need to edit the
path in the `find_package` call if your VTK version is different from 9.3.

!listing /tutorials/ibm_sphere/udf.cmake


## Immersed Boundary Method Algorithm
id=alg

This section describes how immersed boundary methods are implemented in NekRS. [ibm_geometry]
shows a summary of several stages of the geometry preparation which will be described in the list below.

!media ibm_geometry.png
  id=ibm_geometry
  caption=STL mesh (left) and VTK re-triangulation (middle) which is ultimately used in the NekRS IBM simulation to define the Lagrangian points. The right image shows the background Eulerian mesh after the elements fully inside the solid have been deleted.
  style=width:60%;margin-left:auto;margin-right:auto


1. Generate an STL file corresponding to the immersed surface. This format represents the
   surface as a tesselation, using triangles. An STL file can be
   represented in either binary or ASCII format. Each triangle on the surface can be uniquely
   described with a unit normal and the $(x,y,z)$ coordinates of the three vertices. For example,
   below is the STL file we will use in this tutorial; this file is in ASCII format.

!listing /tutorials/ibm_sphere/small_sphere.stl

2. Generate a "background" mesh on which the Eulerian flow solve will occur. This mesh can be
   generated using any software you like to generate meshes with. Since our flow domain is a
   rectangular prism, we can use a [GeneratedMeshGenerator](GeneratedMeshGenerator.md) to
   build this in a straightforward manner.

!listing /tutorials/ibm_sphere/background_mesh.i

   To generate this mesh, run the following, which will create a file `background_mesh_in.e`.

```
cardinal-opt -i background_mesh.i --mesh-only
```

3.  (Optional) You may optionally decide to delete the elements which are fully inside the
    solid body; this is useful to help conserve mass (by reducing the amount of fluid which
    penetrates the solid surface to enter the solid region) and computational expense (if
    the body comprises a large part of the elements in the background mesh, removing those
    elements will reduce the size of the [!ac](CFD) solve). Install [!ac](VTK)
    following the instructions in [#vtk].

    Then, you will need to compile the `delete.cpp` C++ routine, which handles the element
    deletion. This will create a program called `delete_enclosed_elements`.

```
g++ -I${HOME}/vtk/install/include/vtk-9.3 -L${HOME}/vtk/install/lib -o delete_enclosed_elements delete.cpp -lvtkCommonCore-9.3 -lvtkCommonDataModel-9.3 -lvtkFiltersCore-9.3 -lvtkIOXML-9.3 -lvtkCommonExecutionModel-9.3 -lvtksys-9.3 -lvtkIOCore-9.3 -lvtkIOExodus-9.3 -lvtkIOGeometry-9.3 -lvtkIOLegacy-9.3
```

    Finally, you can perform the element deletion. For this tutorial,

```
./delete_enclosed_elements background_mesh_in.e small_sphere.stl
```

    When you run this script, the original sidesets in the mesh are deleted. You will need to re-add
    those sidesets back in, while also creating a new sideset to represent the sides of the cut elements.
    You can re-add sidesets using Cubit, or if the mesh is simple enough we can add them using
    MOOSE's mesh generators. For example, in the file below, we use the unit normals on the six outer
    faces to create sidesets; we then can assign a sideset to all the jagged faces on the cut elements
    using a [SideSetsFromBoundingBoxGenerator](SideSetsFromBoundingBoxGenerator.md).

!listing /tutorials/ibm_sphere/add_sidesets.i

    To run this input file,

```
cardinal-opt -i add_sidesets.i --mesh-only
```

   For this tutorial, we have assigned the following sideset numbers: (1) inlet, (2) outlet, (3) four side-walls, and (3) surface of the cut elements that span the sphere surface. The inlet and outlet will be periodic, whereas surface 3 will be no-slip.

4. Run `exo2nek` to convert the background mesh into NekRS's `.re2` format. In this example, we would enter
   `add_sidesets_in` as the name of our mesh when prompted. We will name the output mesh `cube` to create a
   file named `cube.re2`. Note that the inlet boundary is 1 and the outlet boundary is 2, which are periodic.

```
exo2nek
```

5. Run the flow simulation! Note in the `.udf` file, you will need to specify the path to the STL file, if
   different from the one used in this tutorial.

## Input Files

All of the details of the immersed boundary method are housed in the `cube.udf` and `cube.oudf` files.
There are a lot of details in these files, since the method evolved over time and in some cases there is
more than one option for how to define a part of the algorithm (for instance, the signed distance field
can be computed using either [!ac](VTK) or Embree; different spreading operators were also tested).

!listing /tutorials/ibm_sphere/cube.udf language=cpp

!listing /tutorials/ibm_sphere/cube.oudf language=cpp

The `.par` file is routine, without any unique aspects specific to the IBM setup.

!listing /tutorials/ibm_sphere/cube.par

## Execution and Postprocessing

To run the immersed boundary simulation,

```
mpiexec -np 10 cardinal-opt -i nek.i
```

This will run NekRS with 10 MPI ranks. Alternatively, you could use the following to run without the
Cardinal wrapping,

```
nrsmpi cube 10
```

To run the simulation faster, you can increase the number of ranks, or
simply decrease the polynomial order.
During the simulation and when the simulation has completed, you will have created a number of different output files:

- `surface_mesh.vtk` contains a VTK representation of the original STL surface, after it has
  been re-triangulated so that each surface triangle is fully contained within one NekRS
  element. For instance, [ibm_geometry] shows a comparison of the original STL file (`small_sphere.stl`)
  with the retriangulated surface (`surface_mesh.vtk`). Area calculations are performed to ensure
  minimial distortion in the geometry after re-triangulation.

- `cube0.f*`, the NekRS field files which contain the solution on the Eulerian grid

For example, [ibm_velocity] shows the fluid velocity and pressure on the Eulerian grid. Note that in the vicinity of the immersed surface,
using a finer resolution (such as with NekNek) or a higher polynomial
order will diminish the small oscillations. See [!cite](hegazy_ibm)
for more details on improving surface refinement and the tradeoffs
in combined $h$ and $p$ refinement, the STL triangle resolution, and
overlapping meshes.

!media ibm_velocity.png
  id=ibm_velocity
  caption=Velocity and pressure computed by NekRS's IBM solver.
  style=width:60%;margin-left:auto;margin-right:auto
