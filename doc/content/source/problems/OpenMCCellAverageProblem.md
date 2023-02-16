# OpenMCCellAverageProblem

This class couples OpenMC cell-based models (e.g. [!ac](CSG) or [!ac](DAGMC)) to MOOSE. The crux
is to identify a mapping between OpenMC cells and
a [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).
Then, field data on the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
is volume-averaged and applied to the
corresponding OpenMC cells, while field data in OpenMC is directly applied
as `CONSTANT MONOMIAL` fields on the
[MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).
The smallest possible input file to run OpenMC
is shown below.
The remainder of this page describes how `OpenMCCellAverageProblem` wraps
OpenMC as a MOOSE application.

!listing /smallest_openmc_input.i
  id=openmc1
  caption=Smallest possible OpenMC wrapped input file.

!alert warning
OpenMC as a standalone application has several
[command line parameters](https://docs.openmc.org/en/stable/usersguide/scripts.html). Because
all of these settings can also be controlled through the XML files, and
most of OpenMC's command line parameters control features related to debugging,
Cardinal does not propagate any *OpenMC* command line parameters down to OpenMC
when using Cardinal. The only exception
is that we specify the number of threads on the command line with
`--n-threads=<threads>`, a MOOSE command line parameter (in other words, OpenMC's
command line specification of threads with `-s <threads>` is ignored). If you want to use
any of OpenMC's command line features, we recommend running with the `openmc`
executable built as part of Cardinal's compilation process.

## Initializing MOOSE-type Field Interfaces

When coupling OpenMC to MOOSE, the first
action taken by this class is to initialize MOOSE-type variables
([MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html))
needed to
communicate OpenMC's solution with a general MOOSE application.
Depending on the user settings,
the following variables will be added:

- A variable representing the OpenMC tally; the score is selected with the `tally_score`
  parameter, while the name is selected with the `tally_name` parameter (which defaults to
  the name of the score).
- `temp`, the temperature to be sent to OpenMC
- `density`, the density to be sent to OpenMC (fluid coupling only)

Here, "fluid coupling" refers to the case where part of the MOOSE domain
has been indicated as fluid. Solid density is not currently coupled to OpenMC,
because doing so would require on-line modification of OpenMC's geometry to
properly preserve the solid mass (as opposed to the fluid case, which is assumed
to be flowing and continually entering/exiting the domain).

The order of all of these variables is `CONSTANT MONOMIAL` to simplify the
averaging performed in space (for data going *in* to OpenMC) and to simplify the
application of the cell or element-average tally (for data going *out* of OpenMC)
to the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).
The initialization of these MOOSE variables happens behind the scenes -
for instance, in [openmc1], we have indicated that we are coupling OpenMC via
a tally (by setting `tally_blocks`) to both fluid and solid regions (by setting
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
  [kappa_fission] # always added; the name is determined by the tally_score
    order = CONSTANT
    family = MONOMIAL
  []
[]

This auxiliary variable addition happens automatically,
so you don't need to add these variables yourself.

## Cell to Element Mapping

Next, a mapping from the elements in the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
(i.e. the mesh in the `[Mesh]` block in the OpenMC-wrapped input file) is established to the OpenMC geometry.

!alert note
The OpenMC geometry is *always* specified in length units of centimeters.
Most other MOOSE applications use SI units, i.e. meters for the length unit.
See [#scaling] to learn how to couple OpenMC models to non-centimeter-based MOOSE applications.

The mesh in the `[Mesh]` block
is referred to here as a "mesh mirror" in our attempt to achieve uniform
descriptive language with [NekRSMesh](/mesh/NekRSMesh.md) (which truly is
a mirror of the NekRS [!ac](CFD) mesh for MOOSE-wrapped Nek calculations).
For OpenMC, the mesh mirror is created off-line by the user,
and (combined with the cell definitions in the OpenMC model) represents the
resolution of data sent in/out of OpenMC. The mesh mirror
is only used for receiving data, so there are no requirements on node continuity
across elements. That is, if you already have generated a [!ac](CFD) mesh
for the fluid phase, and a separate solid mesh for the solid regions
you can simply
generate the OpenMC mesh mirror by combining the two with a
[CombinerGenerator](https://mooseframework.inl.gov/source/meshgenerators/CombinerGenerator.html).

`OpenMCCellAverageProblem` will loop over all the elements in the mesh mirror
and map each element to an OpenMC cell according to the element centroid.
[openmc_mesh] depicts an OpenMC geometry, a mesh mirror,
and a visualization of the mapping from these elements to the OpenMC cells.
For the cell IDs colored in the lower left, the element-to-cell mapping is shown
on the right. The insert in the lower right shows the boundary of an OpenMC cell
as a white dashed line; the element centroids, shown as white dots, determine the
cell-to-element mapping.

!media openmc_mesh.png
  id=openmc_mesh
  caption=Illustration of OpenMC particle transport geometry and the mapping of OpenMC cells to a user-supplied "mesh mirror."

#### Mapping Requirements

There are no requirements on alignment of elements/cells or on preserving volumes -
the OpenMC cells and mesh mirror elements do not need to be conformal. Elements
that don't map to an OpenMC cell simply do not participate in the multiphysics
coupling (and vice versa for the cells). This feature can be used to exclude regions
such as reflectors from multiphysics feedback.

The `fluid_blocks` and `solid_blocks` parameters are used to indicate which
blocks in the mesh mirror correspond to fluid and solid. All OpenMC
cells that map to fluid elements will receive temperature and
density feedback. Likewise, all OpenMC cells that map to
solid elements will receive only temperature feedback.
Any blocks in the
mesh that aren't listed in either `fluid_blocks` or `solid_blocks` are ignored in the coupling.
The only requirements imposed here are:

- The same block cannot be present in both `fluid_blocks` and `solid_blocks`,
  such as for thermal models based on the porous media approximation where
  fluid-solid phase boundaries are homogenized. This assumption will
  be relaxed in the future.
- An OpenMC cell cannot map to elements in both the `fluid_blocks` and `solid_blocks` -
  otherwise, it is unclear if the cell should receive density feedback or not.

Despite our flexibility, the cell-to-element mapping should be established
with care - there are two general behavior patterns that are typically undesirable.
In [openmc_coarser], consider the case where the
`[Mesh]` has four equal-sized elements, each of volume $V$, while the OpenMC
domain has three equal-sized cells. Assume that
the power produced by each OpenMC cell is $Q$, so that the total power of the OpenMC
domain is $3Q$. By nature of the centroid mapping, one OpenMC cell will map to
a much larger region of space than the other two cells, and even though the fission
power in each OpenMC cell is $Q$, the power density will differ by a factor of
two once mapped to the `[Mesh]`.

!media openmc_coarser.png
  id=openmc_coarser
  caption=Element to cell mapping for OpenMC cells coarser than the `[Mesh]`
  style=width:60%;margin-left:auto;margin-right:auto

You can
monitor this aspect of the mapping by checking the mesh volumes that each OpenMC cell maps to
by setting `verbose = true`, which will print for each cell a message similar to:

```
cell   1, instance   0 (of   1):   200 solid elems  0 fluid elems  0 uncoupled elems  |  Mapped elems volume (cm3):  3.56463
```

If the "Mapped elems volume" differs significantly among cells that actually have
the same volume, you may consider adjusting the `[Mesh]` and/or the OpenMC
geometry. You may also consider running an [OpenMC volume calculation](https://docs.openmc.org/en/latest/usersguide/volume.html)
to compare the "Mapped elems volume" with a stochastic calculation of cell volumes
to ensure a reasonable mapping. If all of the tallied OpenMC cells are actually
the same volume, you can also use the `check_equal_mapped_tally_volumes`
parameter to automatically check that the mapped volue for each tally bin is identical.

In [openmc_finer], consider the case where the `[Mesh]` has two equal-sized elements,
while the OpenMC domain has three equal-sized cells. By nature of the centroid mapping,
one OpenMC cell not participate in coupling because no elements in the `[Mesh]`
have a centroid within the cell.

!media openmc_finer.png
  id=openmc_finer
  caption=Element to cell mapping for OpenMC cells finer than the `[Mesh]`
  style=width:60%;margin-left:auto;margin-right:auto

If your OpenMC geometry consists of a single coordinate level, and any OpenMC cells
are skipped in the coupling, this class prints a warning message like

```
*** Warning ***
The following warning occurred in the object "MOOSE Problem", of type "OpenMCCellAverageProblem".

Skipping multiphysics feedback for 3 OpenMC cells
```

Of course, the situations shown in [openmc_coarser] and [openmc_finer] can occur even if the number
of mesh elements exactly matches the number of OpenMC cells, depending on where
the element centroids and cell boundaries are located. Therefore,
we recommend using `verbose = true` when building a coupled model to ensure
the desired multiphysics feedback resolution.

#### Cell Levels

A unique temperature and density will be assigned to each OpenMC cell that has
a unique *combination* of a cell ID and instance. As such,
there are two important parameters used in establishing the mapping
- `fluid_cell_level` and `solid_cell_level`. These two parameters
indicate the coordinate "level" in the OpenMC geometry to "stop at" for
identifying the cell ID/instance pairs. Cell-based geometries can be constructed
by nesting repeated universes/lattices at multiple locations in the domain, but
you may not be interested in always coupling cells
at the lowest level to MOOSE. For instance, with [!ac](TRISO) fuel pebbles, it is often
desirable to apply temperature feedback to all the cells in a pebble at once
(rather than separately to the thousands of [!ac](TRISO) particles). A second
important consideration with feedback in geometries such as this arises from
the simple pairing of each element to *one* OpenMC cell based on its centroid. You
can imagine a situation where a [!ac](TRISO) pebble consists of 10,000 distinct
cells, which is coupled to a sphere unstructured mesh with 300 elements. By mapping
each element according to its centroid, a maximum of 300 OpenMC cells (out of the
10,000) could possibly receive feedback.

The `fluid_cell_level` and `solid_cell_level` parameters
indicate the coordinate level (relative to 0, the highest level in the geometry)
at which to "stop" the find cell routine. In other words, if a spherical cell
is defined at level 2, and a [!ac](TRISO) particle lattice fills that cell (i.e. at level 3), then
`solid_cell_level` should be set to 2 in order to apply homogenized pebble feedback,
and to 3 in order to apply individual [!ac](TRISO) feedback.

In many cases, the coordinate levels on which you would like to couple OpenMC
to MOOSE are not the same everywhere in the OpenMC geometry. Consider
the case of a reactor core constructed with lattices of fuel bundles, each of which
is itself a lattice of pincells. To couple
on the pincell level, the `solid_cell_level` would be set to 2 (level 0 is
the highest level, level 1 is the bundle lattice, and level 2 is the pincell lattice).
However, the model might contain a core
vessel on level 0 that you would like to couple
without having to double-nest it below the root universe. To allow coupling across multiple
coordinate levels, use the `lowest_solid_cell_level` and/or `lowest_fluid_cell_level`
parameters in place of the `solid_cell_level`/`fluid_cell_level` parameters. When set,
these indicate that coupling should be performed on the specified level *except* in
regions where there are no cells at that level. In other words, for the previous
full core example, by setting `lowest_solid_cell_level = 2`, coupling will be done
on coordinate level 2 unless there are no cells at that level. This means that in the
vessel region, coupling will be performed on coordinate level 0 because that is the
locally "deepest" coordinate level.

## Adding Tallies

This class automatically creates tallies
to compute the power distribution, with
two options:

1. cell tallies
2. libMesh unstructured mesh tallies

The tally type is specified with the `tally_type` parameter.
The fission tally is normalized according to the specified `power` or `source_strength`
(depending on whether you are running a $k$-eigenvalue or fixed-source problem). By default,
the normalization is done against a global tally added over the entire
OpenMC domain. By setting `normalize_by_global_tally` to false, however, the fission tally is instead
normalized by the sum of the fission tally itself.

#### Cell Tallies

With cell tallies, `tally_blocks` specifies which blocks
in the `[Mesh]` should be tallied. Then, any OpenMC cells that map to those blocks
are added to a cell tally, with one bin for each unique cell ID/instance combination.
The only requirement imposed is that:

- An instance of an OpenMC cell cannot map to elements that are both in `tally_blocks` and not in
  `tally_blocks` - otherwise, it is unclear if the cell should have a tally or not.

#### Unstructured Mesh Tallies
  id=um

There are two options with unstructured mesh tallies:

- Do nothing, in which case OpenMC will tally on the `[Mesh]`
- Specify a `mesh_template`, which provides a path to a mesh file

For the `mesh_template` option, it is possible
to translate the same mesh to multiple locations in the OpenMC geometry
(while only taking up the memory needed to store a single mesh).
You can set the `mesh_translations` or `mesh_translations_file` to provide a set
of coordinates to apply to translate the mesh template. This is a useful feature for
geometries that consist of many repeated geometry units, such as pebble bed and pin fuel
systems.

!alert note
The `mesh_template` and the mesh translations must be in the same units as the
`[Mesh]` block.

At present, unstructured mesh tallies are copied directly to the `[Mesh]` (without
doing any type of nearest-node lookup). Therefore, there is an important limitation
when using unstructured *file-based* mesh tallies.
Suppose the mesh template consists of a mesh for a pincell with $N$ elements
that you have translated to 3 different locations, giving a total of $3N$ tally
bins. Because a direct copy is used to transfer the mesh tally results to the `[Mesh]`,
the first $3N$ elements in the `[Mesh]` must exactly match the $3N$ elements in
the mesh tally (except for a possible mesh scaling, as described in [#scaling]).
This equivalence is required for the direct copy to be accurate - otherwise, the
mesh tally results would be transferred to incorrect regions of space.
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

Using a mesh template other than `pincell.e` *or*
using a different order of translations to the mesh template than the element
ordering in `[Mesh]`, will trigger an error.
Because meshes for each phase (solid pins, fluid, other structural regions)
are usually created separately anyways, this requirement has in practice not been
a significant limitation.

## Calculation Methodology

`OpenMCCellAverageProblem` inherits from the [ExternalProblem](https://mooseframework.inl.gov/source/problems/ExternalProblem.html)
class. For each time step, the calculation proceeds according to the `ExternalProblem::solve()` function.
Data gets sent into OpenMC, OpenMC runs a "time step"
(a $k$-eigenvalue or fixed source calculation), and data gets extracted from OpenMC.
`OpenMCCellAverageProblem` defines the `syncSolutions` and `externalSolve` methods.
Each of these functions will now be described.

!listing /framework/src/problems/ExternalProblem.C
  re=void\sExternalProblem::solve.*?^}

#### External Solve
  id=solve

The actual solve of a "time step" by OpenMC is peformed within the
`externalSolve` method, which runs a $k$-eigenvalue or fixed source
calculation (depending on the run mode of OpenMC).

!listing language=cpp
void OpenMCCellAverageProblem::externalSolve()
{
  int err = openmc_run();
  if (err)
    mooseError(openmc_err_msg);
}

#### Transfers to OpenMC

In the `TO_EXTERNAL_APP` data transfer, [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
are read from the `[Mesh]` and volume-averaged over all elements corresponding to cell
$i$ and then applied to cell $i$. Temperature is always communicated to
all OpenMC cells that were mapped to MOOSE, while density is only communicated
to those cells that mapped to elements on the `fluid_blocks`.

#### Transfers from OpenMC

In the `FROM_EXTERNAL_APP` data transfer, [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
are written to on the `[Mesh]` by writing a tally. For cell tallies,
all elements that mapped to cell $i$ are written with the same cell-averaged
tally value. For mesh tallies, each tally bin is written to the corresponding
element in the `[Mesh]`.

## Other Features

While this class mainly facilitates data transfers to and from OpenMC, a number of
other features are implemented in order to enable more convenient input file
setup and achieve better iterative performance. These are described in this section.

#### Mesh Scaling
  id=scaling

OpenMC always uses a length unit of centimeters, but a coupled MOOSE application
often uses SI units (with a length unit of meters). When transferring field data
to/from OpenMC, it is important to account for the length units
of the coupled MOOSE application. The `scaling` parameter
is used to apply a multiplicative factor to the `[Mesh]` and `mesh_template`
(if using a file-based mesh tally) to get to units of
centimeters assumed by OpenMC. This multiplicative factor is then used to

- Find cell routines in OpenMC in order to map a centimeters-based
  OpenMC geometry to a different length unit in the `[Mesh]`
- Scale the fission power in OpenMC (units of W/cm$^3$) to a different volume
  unit in the `[Mesh]`
- If an unstructured mesh tally is used, scale the `mesh_template` and
  `mesh_translations`/`mesh_translations_file` to units of centimeters

For instance, if the `[Mesh]` is specified in units of meters,
then `scaling` should be set to 100.0 to indicate that the `[Mesh]` is
specified in a unit 100.0 times larger than the OpenMC unit of centimeters.

#### Tally Optimizations

"Spatially separate" tallies are tallies where a particle can only score to one
bin for a given event - for instance, a tally with one bin per fuel pebble is a
spatially separate tally because a particle scoring to fission in pebble $A$ cannot
also score to fission in pebble $B$. OpenMC allows you
to specify that tallies are [spatially separate](https://docs.openmc.org/en/latest/io_formats/tallies.html#assume-separate-element), which can offer a big performance improvement. If you know
that your problem satisfies all of the following criteria,
`assume_separate_tallies = true` will greatly speed up the particle tracking rate:

- `check_tally_sum = false`; when you set this parameter to true, Cardinal
  automatically adds a *global* tally to check that your local
  tally that couples to MOOSE didn't "miss" any fissile regions. This parameter
  must be false to ensure spatially-separate tallies.
- `normalize_by_global_tally = false`; when you set this parameter to true,
  Cardinal will again automatically add a *global* tally in order
  to normalize the local tally.
  For the same reasons as above, your tallies will no longer be spatially separate.
- Your `tallies.xml` file does not contain any other tallies that would fail
  to be spatially separate from the tallies automatically added by Cardinal.

#### Tally Scores

You can customize the type of score that OpenMC uses for its tally.
Options include:

- `heating`: total nuclear heating
- `heating_local`: same as the `heating` score, except that energy from secondary photons
   is deposited locally
- `kappa_fission`: recoverable energy from fission, including prompt sources (fission fragments,
   prompt neutrons, prompt gammas) and delayed sources (delayed neutrons,
   delayed gammas, delayed betas). Neutrino energy is neglected. The energy from photons
   is assumed to deposit locally.
- `fission_q_prompt`: the prompt components of the `kappa_fission` score, except that the energy
   released is a function of the incident energy by linking to optional fission energy release data.
- `fission_q_recoverable`: same as the `kappa_fission` score, except that the score depends
   on the incident energy by linking to optional fission energy release data
- `damage_energy`: damage energy production

For more information on the specific meanings of these various scores,
please consult the [OpenMC tally documentation](https://docs.openmc.org/en/stable/usersguide/tallies.html).

#### Relaxation

OpenMC is coupled to MOOSE via fixed point iteration, also referred to
as Picard iteration. For many problems, oscillations can exist between OpenMC
and the coupled thermal-fluid physics. The OpenMC wrapping offers several
options by which the tally can be "relaxed" between
iterations to effectively damp these oscillations. For all these relaxation schemes,
the tally that gets coupled to MOOSE for iteration $n+1$ is taken as a weighted
sum of the previous iterate and the most-recent Monte Carlo calculation:

\begin{equation}
\label{eq:fp1}
\dot{q}^{n+1}=\left(1-\alpha^n\right)\dot{q}^n+\alpha^n\Phi^n
\end{equation}

where $\alpha^n$ is the weighting factor, $\Phi^n$ indicates
the $n$-th Monte Carlo solve (which itself is a function of an operator representing the
thermal-fluid solve) and the number of samples $s$. For the very first fixed point
iteration, because there is no previous iterate, $\dot{q}^{1}=\Phi^0$.
For $\alpha>1$, [eq:fp1]
is referred to as "over-relaxation" because more than a full step is taken in
the direction of the most recent iterate. Conversely, for $\alpha<1$, [eq:fp1] is
referred to as "under-relaxation" because less than a full step is taken in the
direction of the most recent iterate.

Several options are available for selecting $\alpha$ and $s$. These
are chosen with the `relaxation` parameter.
Options include:

- `constant`: choose a constant $\alpha$ and $s$
- `robbins_monro`: $\alpha=\frac{1}{n+1}$ and constant $s$; applying the recursion
  in [eq:fp1] shows that the solution at iteration $n+1$ is the average of the
  previous $n$ Monte Carlo solutions

\begin{equation}
\label{eq:rm}
\dot{q}^{n+1}=\frac{1}{n+1}\sum_{i=0}^n\Phi^n
\end{equation}

- `dufek_gudowski`: variable $\alpha$ and $s$; the step size is selected based
  on the fraction of samples in the $n$-th Monte Carlo solve relative to the total
  number of samples performed so far. The number of samples is then based on the notion
  that the step size $\alpha$ should be proportional to the convergence rate of the
  central limit theorem (inversely proportional to the square root of the total number
  of simulated particles):

\begin{equation}
\begin{aligned}
\label{eq:dg1}
\alpha^n=&\ \frac{s^n}{\sum_{i=0}^ns^i}\\
s^n=&\ \frac{s^0+\sqrt{s^0s^0+4s^0\sum_{i=0}^{n-1}s^i}}{2}
\end{aligned}
\end{equation}

#### Controlling OpenMC Termination

OpenMC's [tally triggers](https://docs.openmc.org/en/latest/pythonapi/generated/openmc.Trigger.html?highlight=trigger)
allow OpenMC to automatically end its active batches
once reaching certain criteria in $k$ and/or the tally uncertainties, including:

- $k$ standard deviation
- $k$ variance
- $k$ relative error
- tally relative error

Set the `k_trigger` parameter to activate a trigger based on $k$, and set
`tally_trigger` to activate a trigger based on the tally created
automatically as part of the wrapping setup. Then, the desired convergence
threshold is specified with the `k_trigger_threshold` and `tally_trigger_threshold`
parameters, respectively. Both $k$ and tally triggers can be used simultaneously.

#### Controlling the OpenMC Settings

This class provides minimal capabilities to control the OpenMC simulation
settings directly from the Cardinal input file:

- `batches`: the number of batches (inactive plus active)
- `inactive_batches`: the number of inactive batches
- `openmc_verbosity`: [verbosity level](https://docs.openmc.org/en/latest/io_formats/settings.html#verbosity) in OpenMC
- `particles`: the number of particles per batch

For all of the above, a setting in the Cardinal input file will override
any settings in the OpenMC XML files.

Setting the `batches` parameter will also set the maximum number of batches (for cases
that use triggers) and write a statepoint that includes the new total number
of batches.

For the `openmc_verbosity` parameter, because the verbosity setting
is used in the call to `openmc_init` (at which point `OpenMCCellAverageProblem` doesn't
exist yet), we cannot change the verbosity during *initialization*
through the Cardinal input files. However, setting `openmc_verbosity` will affect the
verbosity of all parts of OpenMC's simulation aside from initialization -
transporting of particles, accumulating tallies, and so on.

#### Outputting the OpenMC Solution

This class provides
minimal capabilities to extract other aspects of the OpenMC solution directly
onto the mesh mirror for postprocessing or visualization. A list of parameters to
output is provided to the `output` parameter:

- `unrelaxed_tally`: unrelaxed tally; this will append `_raw` to the tally name and output to the mesh mirror
- `unrelaxed_tally_std_dev`: unrelaxed tally standard deviation; this will append `_std_dev` to the tally and output to the mesh mirror

#### Collating Temperatures from Multiple Apps

OpenMC is often coupled to multiple MOOSE applications providing temperature -
fluid temperature might be provided by NekRS (wrapped via Cardinal), while the solid
temperature might be provided by BISON. In all situations, OpenMC always reads temperatures
from the `temp` variable. However, if you have multiple applications that are trying
to write into the same `temp` variable, because many of MOOSE's transfers are not
block-restricted, the temperature from one application can overwrite that of another
application. To get around this, you could use several [SelfAux](https://mooseframework.inl.gov/source/auxkernels/SelfAux.html)
auxiliary kernels. Suppose fluid temperature is provided by a variable
`nek_temp`, while the solid temperature is provided by a variable
`bison_temp` - a proper collation of temperatures into a single `temp` variable could
be achieved with:

```
[AuxVariables]
  [nek_temp]
    block = '1'
  []
  [bison_temp]
    block = '2 3'
  []
[]

[AuxKernels]
  [assign_fluid_temps]
    type = SelfAux
    variable = temp
    v = nek_temp
    block = '1'
  []
  [assign_solid_temps]
    type = SelfAux
    variable = temp
    v = bison_temp
    block = '2 3'
  []
[]
```

This class conveniently has shortcut syntax to do all of the above:

```
[Problem]
  type = OpenMCCellAverageProblem

  temperature_variables = 'nek_temp bison_temp bison_temp'
  temperature_blocks = '1 2 3'
[]
```

In other words, Cardinal will create the variables specified with the
`temperature_variables` parameter and then also set up the
[SelfAux](https://mooseframework.inl.gov/source/auxkernels/SelfAux.html)
auxiliary kernels to assign those variables into the `temp` variable that OpenMC
reads from.

#### Symmetric Data Transfers

Many neutronics problems are symmetric - such as half-core or quarter-core symmetry.
However, some MOOSE mesh generators (such as in the
[Reactor module](https://mooseframework.inl.gov/modules/reactor/index.html)) don't
yet support symmetric geometries by slicing along arbitrary planes. This class supports
both half-symmetric OpenMC models and general $n$-th symmetric rotational models by
coupling symmetric OpenMC models to full-system thermal-fluid models.

The symmetric mapping is specified with the `symmetry_mapper` parameter, a
[SymmetryPointGenerator](/userobjects/SymmetryPointGenerator.md) user object.
Note that if asymmetries exist in whatever
physics OpenMC is coupled to, they will be averaged on both sides of the plane
before sending temperatures and densities to OpenMC. Please consult the
[SymmetryPointGenerator](/userobjects/SymmetryPointGenerator.md) for more information.

!syntax parameters /Problem/OpenMCCellAverageProblem

!syntax inputs /Problem/OpenMCCellAverageProblem

!syntax children /Problem/OpenMCCellAverageProblem
