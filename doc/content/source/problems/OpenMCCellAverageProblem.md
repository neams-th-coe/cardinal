# OpenMCCellAverageProblem

This class performs all activities related to solving OpenMC as a MOOSE
application. This class also facilitates data transfers to/from
[MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
to exchange field data between OpenMC and MOOSE. The crux of this OpenMC wrapping
is to identify a mapping between OpenMC cells and, if present, unstructured
mesh tallies, to a [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).
Then, field data on the `MooseMesh` is volume-averaged and applied to the
corresponding OpenMC cells, while field data in OpenMC is directly applied
as `CONSTANT MONOMIAL` fields on the `MooseMesh`.

The smallest possible MOOSE-wrapped input file that can be used to run OpenMC
is shown below. The crux of an OpenMC wrapping is the `OpenMCCellAverageProblem`
class, which controls program execution and data transfers to whatever mesh is
supplied in the `[Mesh]` block.

!listing /smallest_openmc_input.i
  id=openmc1
  caption=Smallest possible OpenMC wrapped input file.

The remainder of this page describes how `OpenMCCellAverageProblem` wraps
OpenMC as a MOOSE application.

## Initializing MOOSE-type Field Interfaces

When initializing a coupling of OpenMC within the MOOSE framework, the first
action taken by this class is to initialize MOOSE-type variables
([MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html))
needed to
communicate OpenMC's solution with a general MOOSE application.

First, `OpenMCCellAverageProblem` initializes [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
to receive data necessary for multiphysics coupling. Depending on the settings
for this class, the following variables will be added:

- `heat_source`, the OpenMC fission heat source to be sent to MOOSE based on a `kappa-fission` tally score
- `temp`, the MOOSE temperature to be sent to OpenMC
- `density`, the MOOSE density to be sent to OpenMC (fluid coupling)

Here, "fluid coupling" refers to the case where part of the MOOSE domain
has been indicated as fluid. Solid density is not currently coupled to OpenMC,
because doing so would require on-line modification of OpenMC's geometry to
properly preserve the solid mass (as opposed to the fluid case, which is assumed
to be flowing and continually entering/exiting the domain).

The order of all of these variables is `CONSTANT MONOMIAL` to simplify the
averaging performed in space (for data going *in* to OpenMC) and to simplify the
application of the cell or element-average heat source (for data going *out* of OpenMC)
to the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).
The initialization of these MOOSE variables happens behind the scenes -
for instance, in [openmc1], we have indicated that we are coupling OpenMC via
a heat source (by setting `tally_blocks`) to both fluid and solid regions (by setting
`solid_blocks` and `fluid_blocks`). Therefore, `OpenMCCellAverageProblem`
essentially adds the following to the input file:

!listing
[AuxVariables]
  [temp] # always added
    order = CONSTANT
    family = MONOMIAL
  []
  [density] # only added if fluid_blocks is specified
    order = CONSTANT
    family = MONOMIAL
  []
  [heat_source] # always added
    order = CONSTANT
    family = MONOMIAL
  []
[]

This auxiliary variable addition happens automatically to simplify input file creation,
so you don't need to add these variables yourself in the input file.

## Cell to Element Mapping

After adding the variables to receive/extract coupling data from,
a mapping from the elements in the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
(i.e. the mesh in the `[Mesh]` block
in the OpenMC-wrapped input file) is established to the OpenMC geometry
(as specified in a `geometry.xml` file).

!alert note
The OpenMC geometry is _always_ specified in length units of centimeters.
Most other MOOSE applications use SI units, i.e. meters for the length unit.
`OpenMCCellAverageProblem` has capabilities to couple OpenMC to a MOOSE
application in a length unit other than centimeters, which will be described
in [#scaling].

The mesh in the `[Mesh]` block
is referred to here as a "mesh mirror" in an attempt to achieve uniform
descriptive language with [NekRSMesh](/mesh/NekRSMesh.md), which truly is
a mirror of the NekRS [!ac](CFD) mesh for MOOSE-wrapped Nek calculations.
However, OpenMC differs in a significant manner from NekRS because the Monte Carlo
solution is represented as volume averages over [!ac](CSG) cells, rather than
over a mesh. This causes the significance of the phrase "mesh mirror" to differ
between the OpenMC and NekRS wrappings in Cardinal. It is worthwhile to devote
some time to clarify the difference between the `[Mesh]` block used in the
NekRS and OpenMC wrappings.

For the NekRS wrapping, the [NekRSMesh](/mesh/NekRSMesh.md) mesh mirror represents
a lower-order version on the mesh on which the [!ac](CFD) calculation occurs.
For the OpenMC wrapping, the mesh mirror is instead created off-line by the user,
and (combined with the cell definitions in the OpenMC model) represents the
resolution of coupling data sent in/out of OpenMC. The mesh mirror for OpenMC
wrappings is only used for receiving data, so there are no requirements on node continuity
across elements. That is, if you already have generated a [!ac](CFD) mesh
for the fluid phase, and a separate solid mesh for the solid regions (and these
two meshes don't share nodes at the fluid-solid interface), you can simply
generate the OpenMC mesh mirror by combining the two.

!listing
[Mesh]
  [fluid]
    type = FileMeshGenerator
    file = fluid.e
  []
  [solid]
    type = FileMeshGenerator
    file = solid.e
  []
  [combine]
    type = CombinerGenerator
    inputs = 'fluid solid'
  []
[]

Once you have generated a mesh mirror for use with an OpenMC wrapping,
`OpenMCCellAverageProblem` will loop over all the elements in the mesh mirror
and map each element to an OpenMC cell according to the element centroid.
[openmc_mesh] depicts an example OpenMC geometry, a corresponding mesh mirror,
and a visualization of the mapping from these elements to the OpenMC cells.
For the cell IDs colored in the lower left, the element-to-cell mapping is shown
on the right. The insert in the lower right shows the boundary of an OpenMC cell
as a white dashed line; the element centroids, shown as white dots, determine the
cell-to-element mapping.

!media openmc_mesh.png
  id=openmc_mesh
  caption=Illustration of OpenMC particle transport geometry and the mapping of OpenMC cells to a user-supplied mesh (referred to as the "mesh mirror").

#### Mapping Requirements

There are no requirements on alignment of elements/cells or on preserving volumes -
the OpenMC cells and mesh mirror elements do not need to be conformal. Elements
that don't map to an OpenMC cell simply do not participate in the multiphysics
coupling (and vice versa for the cells). This feature can be used to exclude regions
such as reflectors from multiphysics feedback.
The `fluid_blocks` and `solid_blocks` parameters are used to indicate which
blocks in the mesh mirror correspond to fluid, and which to solid. All OpenMC
cells that map to elements in the `fluid_blocks` will receive temperature and
density feedback from those elements. Likewise, all OpenMC cells that map to
elements in the `solid_blocks` will receive only temperature feedback from
those elements. Any blocks in the
mesh that aren't listed in either `fluid_blocks` or `solid_blocks` are ignored in the coupling.
The only requirements imposed here are:

- The same block cannot be present in both `fluid_blocks` and `solid_blocks`,
  such as for thermal models based on the porous media approximation where
  fluid-solid phase boundaries are homogenized. This particular assumption will
  be relaxed in the future.
- An OpenMC cell cannot map to elements in both the `fluid_blocks` and `solid_blocks` -
  otherwise, it is unclear if the cell should receive density feedback or not.

Despite these limited requirements, the cell-to-element mapping should be established
with care - there are two general behavior patterns that are typically undesirable
in multiphysics calculations. In [openmc_coarser], consider the case where the
`[Mesh]` has four equal-sized elements, each of volume $V$, while the OpenMC
domain has three equal-sized cells. Assume also for illustration purposes that
the power produced by each OpenMC cell is $Q$, so that the total power of the OpenMC
domain is $3Q$. By nature of the centroid mapping, one OpenMC cell will map to
a much larger region of space than the other two cells, and even though the fission
power in each OpenMC cell is $Q$, the power density will differ by a factor of
two once mapped to the `[Mesh]`.

!media openmc_coarser.png
  id=openmc_coarser
  caption=Illustration of potential element to cell mapping for OpenMC cells coarser than the `[Mesh]`
  style=width:60%;margin-left:auto;margin-right:auto

You can
monitor this aspect of the mapping by checking the mesh volumes that each OpenMC cell maps to
by setting `verbose = true`, which will print for each cell a message similar to:

```
cell   1, instance   0 (of   1):   200 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  3.56463
```

If the "Mapped elems volume" differs significantly among cells that actually have
the same volume, you may consider adjusting the `[Mesh]` and/or the OpenMC [!ac](CSG)
geometry. You may also consider running an [OpenMC volume calculation](https://docs.openmc.org/en/latest/usersguide/volume.html)
to compare the "Mapped elems volume" with a stochastic calculation of cell volumes
to ensure a reasonable mapping.

In [openmc_finer], consider the case where the `[Mesh]` has two equal-sized elements,
while the OpenMC domain has three equal-sized cells. By nature of the centroid mapping,
one OpenMC cell not participate in coupling because no elements in the `[Mesh]`
have a centroid within the cell.

!media openmc_finer.png
  id=openmc_finer
  caption=Illustration of potential element to cell mapping for OpenMC cells finer than the `[Mesh]`
  style=width:60%;margin-left:auto;margin-right:auto

If your OpenMC geometry consists of a single coordinate level, and any OpenMC cells
are skipped in the coupling, this class prints a warning message like

```
*** Warning ***
The following warning occurred in the object "MOOSE Problem", of type "OpenMCCellAverageProblem".

Skipping multiphysics feedback for 3 OpenMC cells
```

Of course the situations shown in [openmc_coarser] and [openmc_finer] can occur even if the number
of mesh elements exactly matches the number of OpenMC cells, depending on where
the element centroids and cell boundaries are located. Therefore,
we recommend using `verbose = true` when building a coupled model to ensure
the desired multiphysics feedback resolution.

#### Cell Levels

There are two important parameters used in establishing the mapping from
elements to cells - `fluid_cell_level` and `solid_cell_level`. These two parameters
indicate the coordinate "level" in the OpenMC geometry to "stop at" for
identifying the cell ID/instance pairs. [!ac](CSG) geometries can be constructed
by nesting repeated universes/lattices at multiple locations in the domain, but
for multiphysics coupling, you may not be interested in always coupling cells
at the lowest level to MOOSE. For instance, with [!ac](TRISO) fuel pebbles, it is often
desirable to apply temperature feedback to all the cells in a pebble at once
(rather than separately to the thousands of [!ac](TRISO) particles). A second
important consideration with feedback in geometries such as this arises from
the simple pairing of each element to *one* OpenMC cell based on its centroid. You
can imagine a situation where a [!ac](TRISO) pebble consists of 10,000 distinct
cells, which is coupled to a sphere unstructured mesh with 300 elements. By mapping
each element according to its centroid, a maximum of 300 OpenMC cells (out of the
10,000) could possibly receive feedback.

For these reasons, the `fluid_cell_level` and `solid_cell_level` parameters
indicate the coordinate level (relative to 0, the highest level in the geometry)
at which to "stop" the find cell routine. In other words, if a spherical cell
is defined at level 2, and a [!ac](TRISO) particle lattice fills that cell (i.e. at level 3), then
`solid_cell_level` should be set to 2 in order to apply homogenized pebble feedback,
and to 3 in order to apply individual [!ac](TRISO) feedback (though the latter
requires a very fine pebble mesh in the `[Mesh]` in order for all OpenMC cells
in the pebble region to map to an element).

## Adding Tallies

This class automatically creates `kappa-fission` tallies (recoverable energy release
from fission) in order to compute the fission power distribution. Cardinal includes
two options for tallying the fission power in OpenMC:

1. Cell tallies
2. libMesh unstructured mesh tallies

The tally type is specified with the `tally_type` parameter.
The fission tally is normalized according to the specified `power`. By default,
the normalization is done against a global `kappa-fission` tally added over the entire
OpenMC domain. By setting `normalize_by_global_tally` to false, however, the fission tally is instead
normalized by the sum of the fission tally itself. 

To fully elaborate, consider an
OpenMC problem consisting of 3 fuel pebbles that produce a total of 1 W. Suppose
pebble 1 produces 0.3 W, pebble 2 produces 0.5 W, and pebble 3 produces 0.2 W. If your
`[Mesh]` only overlaps spatially with the first two pebbles, then setting
`normalize_by_global_tally = true` (the default) will apply an OpenMC heat source
to `[Mesh]` with a magnitude of 0.8 W. Normalizing by a global tally indicates that
the `power` represents the total power of the OpenMC domain, regardless of which regions actually
got mapped to the `[Mesh]`. Conversely, by setting `normalize_by_global_tally = false`,
OpenMC will apply a heat source to `[Mesh]` with a magnitude of 1.0 W. Normalizing
by the local tally indicates that the `power` represents only the power of the
regions that got mapped. This latter setting becomes the default when using
mesh tallies; this will be described in more detail in [#um].

### Cell Tallies

When using cell tallies, the `tally_blocks` parameter is used to specify which blocks
in the `[Mesh]` should be tallied. Then, any OpenMC cells that mapped to those blocks
are added to a cell tally that adds a bin for each unique cell ID/instance combination.
The only requirement imposed is that:

- An instance of an OpenMC cell cannot map to elements that are both in `tally_blocks` and not in
  `tally_blocks` - otherwise, it is unclear if the cell should have a tally or not.

### Unstructured Mesh Tallies
  id=um

When using unstructured mesh tallies, the `tally_blocks` parameter is unused. Instead,
a `mesh_template` is used to provide a path to an unstructured mesh that OpenMC
will tally on. To translate the same mesh to multiple locations in the OpenMC geometry
(while only taking up the memory needed to store a single mesh), or
even simply to move a single mesh to a different location than where the mesh template
is defined,
you can set the `mesh_translations` or `mesh_translations_file` to provide a set
of coordinates to apply to translate the mesh template. This is a useful feature for
geometries that consist of many repeated geometry units, such as pebble bed and pin fuel
systems.

!alert note
Because OpenMC solves in units of centimeters, both the `mesh_template` and
any translation coordinates must also be given in units of centimeters
(regardless of any scaling as described in [#scaling]).

At present, unstructured mesh tallies are copied directly to the `[Mesh]` (without
doing any type of nearest-node lookup). Therefore, there is an important limitation
when using unstructured mesh tallies in Cardinal that is best explained by example.
Suppose the mesh template consists of a mesh for a pincell with $N$ elements
that you have translated to 3 different locations, giving a total of $3N$ tally
bins. Because a direct copy is used to transfer the mesh tally results to the `[Mesh]`,
the first $3N$ elements in the `[Mesh]` must exactly match the $3N$ elements in
the mesh tally (except for a possible mesh scaling, as described in [#scaling]).
This equivalence is required for the direct copy to be accurate - otherwise, the
mesh tally results would be transferred to incorrect regions of space in the `[Mesh]`.
Assuming the mesh is set up as follows:

!listing
[Mesh]
  [single_pin]
    type = FileMesh
    file = pincell.e
  []
  [combine]
    type = CombinerGenerator
    inputs = single_pin
    positions = '1.0 0.0 0.0
                 3.0 0.0 0.0
                 5.0 0.0 0.0'
  []
  [some_other_part_of_mesh_mirror]
    type = FileMesh
    file = fluid_and_reflector.e
  []
[]

Then the following setup for the mesh tallies would be correct:

!listing
[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  solid_blocks = 1
  solid_cell_level = 0
  tally_type = mesh
  mesh_template = pincell.e
  mesh_translations = '1.0 0.0 0.0
                       3.0 0.0 0.0
                       5.0 0.0 0.0'
[]

This setup is correct because the first $3N$ elements in the `[Mesh]`
exactly match the mesh template (we use the same `pincell.e` mesh and
translate that pincell in the same order to the three positions). Using a
mesh template other than `pincell.e` for `OpenMCCellAverageProblem`, *or*
using a different order of translations to the mesh template than the element
ordering in `[Mesh]`, will trigger an error. In practice, this design
just requires some attention in the construction of the `[Mesh]` mesh mirror -
but because meshes for each phase (solid pins, fluid, other structural regions)
are usually created separately anyways, this requirement has in practice not been
a significant limitation.

A simple way to guarantee equivalence between the `[Mesh]` and the mesh
template is to:

1. Create the `[Mesh]` block as desired for the tally region.
2. Run the OpenMC-wrapped input file in "mesh only" mode, by passing `--mesh-only` to the run
   command, such as `cardinal-opt -i input.i --mesh-only`. This will produce an output file
   named `input_in.e` that contains the mesh specified in the `[Mesh]` block.
3. Set the mesh template to `mesh_template = input_in.e`.

## Overall Calculation Methodology

`OpenMCCellAverageProblem` inherits from the [ExternalProblem](https://mooseframework.inl.gov/source/problems/ExternalProblem.html)
class. For each time step, the calculation proceeds according to the `ExternalProblem::solve()` function.
Data gets sent into OpenMC, OpenMC runs a "time step"
(actually a k-eigenvalue calculation), and data gets extracted from OpenMC.
`OpenMCCellAverageProblem` mostly consists of defining the `syncSolutions` and `externalSolve` methods.

!listing /framework/src/problems/ExternalProblem.C
  re=void\sExternalProblem::solve.*?^}

Each of these functions will now be described.

### External Solve
  id=solve

The actual solve of a "time step" by OpenMC is peformed within the
`externalSolve` method, which performs the following.

!listing language=cpp
void OpenMCCellAverageProblem::externalSolve()
{
  int err = openmc_run();
  if (err)
    mooseError(openmc_err_msg);
}

This function simply runs a $k$-eigenvalue OpenMC calculation.

### Transfers to OpenMC

In the `TO_EXTERNAL_APP` data transfer, [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
are read from the `[Mesh]` and volume-averaged over all elements corresponding to cell
$i$ and then applied to cell $i$. Temperature is always communicated to
all OpenMC cells that were mapped to MOOSE, while density is only communicated
to those cells that mapped to elements on the `fluid_blocks`.

### Transfers from OpenMC

In the `FROM_EXTERNAL_APP` data transfer, [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
are written to on the `[Mesh]` by writing a heat source. For cell tallies,
all elements that mapped to cell $i$ are written with the same cell-averaged
heat source. For mesh tallies, each tally bin is written to the corresponding
element in the `[Mesh]`.

## Other Features

While this class mainly facilitates data transfers to and from OpenMC, a number of
other features are implemented in order to enable more convenient input file
setup. These are described in this section.

### Mesh Scaling
  id=scaling

OpenMC always uses a length unit of centimeters, but a coupled MOOSE application
often uses SI units (with a length unit of meters). When transferring field data
to/from OpenMC, it is important for data transferred from OpenMC to match the length units
of the coupled MOOSE application. This class contains a `scaling` parameter that
is used to apply a multiplicative factor to the `[Mesh]` to get to units of
centimeters assumed by OpenMC. This multiplicative factor is then applied in the:

- Find cell routines in OpenMC in order to correctly map a centimeters-based
  OpenMC geometry to a different length unit in the `[Mesh]`
- Scale the fission power in OpenMC (units of W/cm$^3$) to a different volume
  unit in the `[Mesh]`

For instance, if the `[Mesh]` is specified in units of meters:

!listing test/tests/neutronics/feedback/different_units/openmc.i
  block=Mesh

then `scaling` should be set to 100.0 to indicate that the `[Mesh]` is
specified in a unit 100.0 times larger than the OpenMC unit of centimeters.

!listing test/tests/neutronics/feedback/different_units/openmc.i
  block=Problem

!syntax parameters /Problem/OpenMCCellAverageProblem

!syntax inputs /Problem/OpenMCCellAverageProblem

!syntax children /Problem/OpenMCCellAverageProblem
