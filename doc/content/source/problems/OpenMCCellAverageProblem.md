# OpenMCCellAverageProblem

This class couples OpenMC cell-based models (e.g. [!ac](CSG) or [!ac](DAGMC)) to MOOSE. The crux
is to identify a mapping between OpenMC cells and
a [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).
The data flow between OpenMC and MOOSE contains two major steps:

- Temperature and/or density field data on the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
  are volume-averaged and applied to the corresponding OpenMC cells.
- Tallies are mapped from OpenMC into `CONSTANT MONOMIAL` fields on the
  [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).

The smallest possible input file to run OpenMC
is shown below.
The remainder of this page describes this syntax, plus more advanced settings.

!listing /smallest_openmc_input.i
  id=openmc1
  caption=Smallest possible OpenMC wrapped input file.

## Initializing Variables

When coupling OpenMC to MOOSE, we first
initialize MOOSE-type variables
([MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html))
needed to
communicate OpenMC's solution with a general MOOSE application.
Depending on the user settings,
the following `CONSTANT MONOMIAL` variables will be added:

- Variable(s) representing the OpenMC tally(s)
- Variable(s) representing the temperature to read into OpenMC. Temperature will be read
  from the mesh subdomains indicated by `temperature_blocks`.
- Variable(s) representing the density to read into OpenMC. Density will be read
  from the mesh subdomains indicated by `density_blocks`.
- A variable (`cell_id`) storing the mapping of OpenMC cell IDs onto the mesh
- A variable (`cell_instance`) storing the mapping of OpenMC cell instances onto the mesh

!alert tip
These variables all have default names, but you can also control their names using the
`temperature_variables`, `density_variables`, and `tally_name` parameters. If you want
to see the names, run with `verbose = true` and tables will print out on initialization
with this information.

!alert note
You do not need to add/initialize ANY of these variables manually - it all happens behind the scenes!

As an example, let's start with a complicated case first.
Suppose our `[Problem]` block looks like the following. In this problem,
we want to apply temperature feedback from four blocks (`fuel`, `cladding`, `water`, `helium`). We also want to apply density feedback, but only from two blocks
(`water` and `fluid`). Suppose we also want to keep separate track of
different variables representing each of these physical fields, so we will want
to read temperature from a variable named `temp0` in the `fuel` and `cladding` blocks, but from
a variable named `nek_temp` in the `helium` block (and so on for density).

```
[Problem]
  type = OpenMCCellAverageProblem

  temperature_variables = 'temp0;         t_water;  nek_temp'
  temperature_blocks    = 'fuel cladding;   water;    helium'

  density_variables = 'rho_water; rho_helium'
  density_blocks = 'water; helium'

  tally_score = 'heating flux'
  tally_name = 'power openmc_flux'
[]
```

Then Cardinal is building the following automatically for you:

```
[AuxVariables]
  [temp0] # added for all corresponding 'temperature_blocks'
    family = MONOMIAL
    order = CONSTANT
    block = 'fuel cladding'
  []
  [t_water] # added for all corresponding 'temperature_blocks'
    family = MONOMIAL
    order = CONSTANT
    block = 'water'
  []
  [nek_temp] # added for all corresponding 'temperature_blocks'
    family = MONOMIAL
    order = CONSTANT
    block = 'helium'
  []
  [rho_water] # added for all corresponding 'density_blocks'
    family = MONOMIAL
    order = CONSTANT
    blocks = 'water'
  []
  [rho_helium] # added for all corresponding 'density_blocks'
    family = MONOMIAL
    order = CONSTANT
    blocks = 'helium'
  []
  [power] # the first tally we added (score is 'heating', but we set a custom name)
    family = MONOMIAL
    order = CONSTANT
  []
  [openmc_flux] # the second tally we added (score is 'flux', but we set a custom name)
    family = MONOMIAL
    order = CONSTANT
  []
[]
```

By default, Cardinal will name *all* temperature feedback with the `temp` variable,
*all* density feedback with the `density` variable, and all
tally scores with the same name as the score. Suppose we instead wanted to rely
on defaults; we would set our `[Problem]` block as:

```
[Problem]
  type = OpenMCCellAverageProblem
  density_blocks = 'water helium'
  temperature_blocks = 'fuel cladding water helium'
  tally_score = 'heating flux'
[]
```

Then Cardinal is instead building the following automatically for you:

```
[AuxVariables]
  [temp] # added for all temperature_blocks
    family = MONOMIAL
    order = CONSTANT
    block = 'fuel cladding water helium'
  []
  [density] # added for all density_blocks
    family = MONOMIAL
    order = CONSTANT
    blocks = 'water helium'
  []
  [heating] # the first tally we added (score is 'heating')
    family = MONOMIAL
    order = CONSTANT
  []
  [flux] # the second tally we added (score is 'flux')
    family = MONOMIAL
    order = CONSTANT
  []
[]
```

## Cell to Element Mapping

Next, a mapping from the elements in the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
(i.e. the mesh in the `[Mesh]` block) is established to the OpenMC geometry.

!alert note
The OpenMC geometry is *always* specified in length units of centimeters.
Most other MOOSE applications use SI units, i.e. meters for the length unit.
See [#scaling] to learn how to couple OpenMC models to non-centimeter-based MOOSE applications.

In the `[Mesh]` block, you should provide a mesh onto which OpenMC will write its
tally values, as well as read temperature and density. This mesh is only used for
reading/writing data, so there are no requirements on node continuity
across elements. `OpenMCCellAverageProblem` will loop over all the elements
and map each to an OpenMC cell according to the element centroid.
[openmc_mesh] depicts an OpenMC geometry, a mesh on which temperature feedback is to be applied,
and a visualization of the mapping from these elements to the OpenMC cells.

!media nc.png
  id=openmc_mesh
  caption=Illustration of OpenMC cells and the mapping to a mesh.

!alert tip
You can visualize how the OpenMC cells map to the mesh by viewing the `cell_id`
and `cell_instance` variables Cardinal automatically outputs. For more information
on how to interpret these variables, see:
[CellIDAux](https://cardinal.cels.anl.gov/source/auxkernels/CellIDAux.html)
and [CellInstanceAux](https://cardinal.cels.anl.gov/source/auxkernels/CellInstanceAux.html).

There are no requirements on alignment of elements/cells or on preserving volumes -
the OpenMC cells and mesh mirror elements do not need to be conformal. Elements
that don't map to an OpenMC cell simply do not participate in the multiphysics
coupling (and vice versa for the cells). This feature can be used to exclude regions
such as reflectors from multiphysics feedback.

The `temperature_blocks` and `density_blocks` are used to indicate which
blocks in the mesh mirror correspond to temperature and/or density feedback.
Any mesh blocks which are excluded are ignored in the coupling.
Despite this flexibility, the cell-to-element mapping should be established
with care. There are two general behavior patterns that are typically undesirable.
In [openmc_coarser], consider the case where the
`[Mesh]` has four equal-sized elements, each of volume $V$, while the OpenMC
domain has three equal-sized cells. Assume that
the power produced by each OpenMC cell is $Q$, so that the total power of the OpenMC
domain is $3Q$. By nature of the centroid mapping, one OpenMC cell will map to
a much larger region of space than the other two cells. Even though the fission
power in each OpenMC cell is $Q$, the power density will differ by a factor of
two once mapped to the `[Mesh]`.

!media openmc_coarser.png
  id=openmc_coarser
  caption=Element to cell mapping for OpenMC cells coarser than the `[Mesh]`
  style=width:60%;margin-left:auto;margin-right:auto

!alert! tip
You can
monitor the mapping by checking the mesh volumes that each OpenMC cell maps to
by setting `verbose = true`, which will print a mapping table. You can also
add an [OpenMCVolumeCalculation](https://cardinal.cels.anl.gov/source/userobjects/OpenMCVolumeCalculation.html) which will run a stochastic volume calculation
and print out the *actual* volume of the OpenMC cells in the "Actual Vol"
column (not just the volume of the mesh elements the cells map to in the "Mapped Vol" column!).

```
---------------------------------------------------------------------------
|         Cell          |   T   | T+rho | Other | Mapped Vol | Actual Vol |
---------------------------------------------------------------------------
| 1, instance 0 (of 10) |   168 |     0 |     0 | 5.298e-01  |            |
| 1, instance 1 (of 10) |   168 |     0 |     0 | 5.298e-01  |            |
| 1, instance 2 (of 10) |   168 |     0 |     0 | 5.298e-01  |            |
---------------------------------------------------------------------------
```

If you know that all the tallied OpenMC cells are actually
the same volume, you can also use the `check_equal_mapped_tally_volumes`
parameter to automatically check that the mapped volue for each tally bin is identical.

!alert-end!

In [openmc_finer], consider the case where the `[Mesh]` has two equal-sized elements,
while the OpenMC domain has three equal-sized cells. By nature of the centroid mapping,
one OpenMC cell will not participate in coupling because no elements in the `[Mesh]`
have a centroid within the cell. This situation is more difficult to detect, but can
be monitored by setting `verbose = true` and monitoring the table outputs.

!media openmc_finer.png
  id=openmc_finer
  caption=Element to cell mapping for OpenMC cells finer than the `[Mesh]`
  style=width:60%;margin-left:auto;margin-right:auto

Of course, the situations shown in [openmc_coarser] and [openmc_finer] can occur even if the number
of mesh elements exactly match the number of OpenMC cells, depending on where
the element centroids are located. Therefore,
we recommend using `verbose = true` when getting started.

#### Cell Levels

A unique temperature and density will be assigned to each OpenMC cell that has
a unique *combination* of a cell ID and instance. As such,
the cell "level" on which to apply multiphysics feedback must be defined, using
either the `cell_level` or `lowest_cell_level` parameter.

The cell "level" is the coordinate "level" in the OpenMC geometry to "stop at" for
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

The `cell_level` and `lowest_cell_level` parameters
indicate the coordinate level (relative to 0, the highest level in the geometry)
at which to "stop" the find cell routine. In other words, if a spherical cell
is defined at level 2, and a [!ac](TRISO) particle lattice fills that cell (i.e. at level 3), then
`cell_level` should be set to 2 in order to apply homogenized pebble feedback,
and to 3 in order to apply individual [!ac](TRISO) feedback.

!alert tip
You can visualize your OpenMC model at different geometry hierarchies using
OpenMC [slice plots](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.Plot.html)
and setting the `level`.

In many cases, the coordinate levels on which you would like to couple OpenMC
to MOOSE are not the same everywhere in the OpenMC geometry. Consider
the case of a reactor core constructed with lattices of fuel bundles, each of which
is itself a lattice of pincells. To couple
on the pincell level, the `cell_level` would be set to 2 (level 0 is
the highest level, level 1 is the bundle lattice, and level 2 is the pincell lattice).
However, the model might contain a core
vessel on level 0 that you would like to couple
without having to double-nest it below the root universe. To allow coupling across multiple
coordinate levels, use the `lowest_cell_level`
parameter in place of the `cell_level` parameter. When set, this
indicates that coupling should be performed on the specified level *except* in
regions where there are no cells at that level. In other words, for the previous
full core example, by setting `lowest_cell_level = 2`, coupling will be done
on coordinate level 2 unless there are no cells at that level. This means that in the
vessel region, coupling will be performed on coordinate level 0 because that is the
locally "deepest" coordinate level.

## Adding Tallies

This class automatically creates tallies with scores specified by the user. There
are two spatial options:

1. cell tallies
2. libMesh unstructured mesh tallies

The tally type is specified with the `tally_type` parameter.
The tally is normalized according to the specified `power` or `source_strength`
(depending on whether you are running a $k$-eigenvalue or fixed-source problem). By default,
the normalization is done against a global tally added over the entire
OpenMC domain. By setting `normalize_by_global_tally` to false, however, the tally is instead
normalized by the sum of the tally itself.

You can customize the type of score that OpenMC uses for its tally with the
`tally_score` parameter. Options include:

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

Cardinal transforms these tallies into "physically meaningful" units.
A full list of the tally units in OpenMC can be found [here](https://docs.openmc.org/en/stable/usersguide/tallies.html). For the scores supported in Cardinal, the [tally_units] compares the units from OpenMC
and the units of the AuxVariables created by Cardinal. Note that for all area or
volume units in [tally_units], that those units match whatever unit is used in the `[Mesh]`.

!table id=tally_units caption=Tally units from OpenMC and the conversion in Cardinal.
| Tally score | OpenMC Units | Cardinal Units |
| :- | :- | :- | :- |
| `heating` | eV / source particle | W / volume |
| `heating_local` | eV / source particle | W / volume |
| `kappa_fission` | eV / source particle | W / volume |
| `fission_q_prompt` | eV / source particle | W / volume |
| `fission_q_recoverable` | eV / source particle | W / volume |
| `damage_energy` | eV / source particle | W / volume |
| `flux` | particle - cm / source particle | particle / area / second |
| `H3_production` | tritium / source particle | tritium / volume / second |

This units-transformation
process involves *division by a volume*. In Cardinal, there are two different notions of volume:

- The volume of the `[Mesh]` elements which *map* to a tally bin region
- The actual volume of the tally bin region in OpenMC

Take [sphere_compare] as an example; on the left is a `[Mesh]`, and on the right is
a spherical OpenMC cell we are mapping to the `[Mesh]`. For this case, the two volumes are
different, because of the faceted nature of the sphere surface. The tallies in Cardinal
are all normalized by the `[Mesh]` volume (0.96 cm$^3$ in this example).
This is a good choice for heating-related tallies, because this will ensure
we properly preserve a total integral (power) when integrated on the `[Mesh]`.
For heating tallies, it is much more important
to preserve the total power, as opposed to the raw pointwise value of the heat deposition (units
of W/m$^3$).

!media sphere_compare.png
  id=sphere_compare
  caption=Illustration of OpenMC particle transport geometry and the mapping of OpenMC cells to a user-supplied "mesh mirror."
  style=width:60%;margin-left:auto;margin-right:auto

However, for some other tallies (e.g. the 'flux' tallies),
dividing by the `[Mesh]` volume indicates instead that we're normalizing so as
to preserve a reaction rate (reactions/sec), as if you integrated $\int \Sigma\phi dV$ over the
`[Mesh]`. This will not give the same raw pointwise tally value as what OpenMC is actually predicting,
if the volumes of the `[Mesh]` and the tally bin regions are significantly different.
If the pointwise value is very important for your use case, you can post-multiply
your AuxVariables by $V_\text{moose}/V_\text{openmc}$ (in this example, 0.96 / 1.00), where $V_\text{moose}$ is the volume
of the `[Mesh]` elements which map to the tally bin, and $V_\text{openmc}$ is the volume of the
corresponding tally region.

!alert note
If your OpenMC tally bins and corresponding `[Mesh]` elements
already are exactly the same volume, then no special thought is needed for the tally
normalization, and the value will be exactly consistent with the interpretation
used in OpenMC.

#### Cell Tallies

With cell tallies, `tally_blocks` specifies which blocks
in the `[Mesh]` should be tallied. Then, any OpenMC cells that map to those blocks
are added to a cell tally, with one bin for each unique cell ID/instance combination.

#### Unstructured Mesh Tallies
  id=um

There are two options with unstructured mesh tallies:

- Do nothing, in which case OpenMC will tally on the `[Mesh]`
- Specify a `mesh_template`, which provides a path to a mesh file

For the `mesh_template` option, it is possible
to translate the same mesh to multiple locations in the OpenMC geometry
(while only taking up the memory needed to store a single mesh) using
the `mesh_translations` or `mesh_translations_file` parameters. This is a useful feature for
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

Then the following setup for the mesh tallies would be correct because the element
ordering of the tally bins would match their corresponding regions on the `[Mesh]`.
Because meshes for each phase (solid pins, fluid, other structural regions)
are usually created separately anyways, this requirement has in practice not been
a significant limitation.

!listing
[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = 1
  cell_level = 0
  tally_type = mesh
  mesh_template = pincell.e
  mesh_translations = '1.0 0.0 0.0
                       3.0 0.0 0.0
                       5.0 0.0 0.0'
[]

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

In the `TO_EXTERNAL_APP` data transfer, [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html) representing temperature, density, and other
coupled fields
are read from the `[Mesh]` and volume-averaged over all elements corresponding to cell
$i$ and then applied to cell $i$.

#### Transfers from OpenMC

In the `FROM_EXTERNAL_APP` data transfer, [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
are written to on the `[Mesh]` by writing a tally. For cell tallies,
all elements that mapped to cell $i$ are written with the same cell
tally value. For mesh tallies, each tally bin is written to the corresponding
element in the `[Mesh]`.

## Other Features

While this class mainly facilitates data transfers to and from OpenMC, a number of
other features are implemented. These are described in this section.

#### CAD Geometry Skinning
  id=skinning

OpenMC is currently limited to setting a single
constant temperature and a single constant density in each cell. For CSG geometries,
the user needs to manually set up sub-divisions in the geometry in order to capture
spatial variation in temperature and density.
For DAGMC geometries, you can instead optionally re-generate the OpenMC cells after
each Picard iteration according to contours in temperature and/or density. With this approach, the OpenMC model can receive spatially-varying temperature and density without the user needing to manually subdivide regions of space *a priori*.

!alert note
This skinning feature is only available for DAGMC geometries for which the `[Mesh]`
obeys the same material boundaries in the starting `.h5m` file.

To use this feature, provide the `skinner` user
object parameter, of type [MoabSkinner](/userobjects/MoabSkinner.md). After each OpenMC
run, this object will group elements in the `[Mesh]` into "bins" for temperature, density,
and/or subdomain (mesh block). A new DAGMC cell will then be created by "skinning" the mesh
elements according to the boundaries between the unique bin regions.
For example, suppose the starting DAGMC model consists of two
cells, each with a different material, as shown in [dagmc_model].

!media dagmc_model.png
  id=dagmc_model
  style=width:50%;margin-left:auto;margin-right:auto
  caption=Example illustration of a DAGMC model before a skinning operation is applied.

Suppose this geometry is skinned with 4 temperature bins between 500 K and 700 K.
The input syntax would look something like

!listing test/tests/neutronics/dagmc/mesh_tallies/openmc.i
  start=Problem
  end=Postprocessors

Elements will then be categorized as falling into one of these bins:

- Bin 0, for $500\ K \leq T< 550\ K$.
- Bin 1, for $550\ K \leq T< 600\ K$.
- Bin 2, for $600\ K \leq T< 650\ K$.
- Bin 3, for $650\ K \leq T\leq 700\ K$.

The geometry will also be skinned according to the subdomain IDs, which align with the
materials in the original DAGMC geometry. For this problem with two materials, elements
will be categorized according to falling into one of two subdomain bins:

- Bin 0, for elements in material A
- Bin 1, for elements in material B

Then, if the following temperature distribution is sent to OpenMC, the DAGMC geometry
will be re-skinned into 8 unique cells, as shown in [skinning]. Depending on the
temperature and material definitions, the problem would be skinned into more or less
cells, depending on which temperatures are observed and whether those element "clumps"
are contiguous or disjoint from one another.
A "graveyard" volume is automatically built around the new DAGMC model between two bounding boxes. This
region is used to apply vacuum boundary conditions by killing any particles that
enter this region.
Once the geometry has been re-created, the rest of the coupling proceeds as normal -
the average temperature and density of each new DAGMC cell is set as a volume-average
of the elements corresponding to each cell.

!media skinning.png
  id=skinning
  caption=Example illustration of a DAGMC model after skinning

#### Mesh Scaling
  id=scaling

OpenMC always uses a length unit of centimeters, but a coupled MOOSE application
often uses a length unit of meters. When transferring field data,
it is important to account for the length units
of the coupled MOOSE application. The `scaling` parameter
is used to apply a multiplicative factor to the `[Mesh]` and `mesh_template`
(if using a file-based mesh tally) to get to units of
centimeters assumed by OpenMC. This multiplicative factor is then used to

- "Find cell" routines in OpenMC in order to map a centimeters-based
  OpenMC geometry to a different length unit`
- Scale the fission power in OpenMC (units of W/cm$^3$) to a different volume unit
- If an unstructured mesh tally is used, scale the `mesh_template` and
  `mesh_translations`/`mesh_translations_file` to units of centimeters

For instance, if the `[Mesh]` is specified in units of meters,
then `scaling` should be set to 100.0 to indicate that the `[Mesh]` is
specified in a unit 100.0 times larger than the OpenMC unit of centimeters.

#### Tally Optimizations

"Spatially separate" tallies are tallies where a particle can only score to one
bin for a given event - for instance, a tally with one bin per fuel pebble in a pebble bed reactor is a
spatially separate tally because a particle scoring to fission in pebble $A$ cannot
also score to fission in pebble $B$. OpenMC allows you
to specify that tallies are [spatially separate](https://docs.openmc.org/en/latest/io_formats/tallies.html#assume-separate-element), which can offer a big performance improvement. If your OpenMC input files do not contain any of their own tallies
(i.e. in a `tallies.xml`), you can try using this feature to speed up your calculation.

#### Relaxation

OpenMC is coupled to MOOSE via fixed point iteration, also referred to
as Picard iteration. For many problems, instabilities can exist between OpenMC
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
the $n$-th Monte Carlo solve,
and the number of samples is $s$. For the very first fixed point
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
\dot{q}^{n+1}=\frac{1}{n+1}\sum_{i=0}^n\Phi^i
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

For each of the quantities listed in the `output` parameter, a summary table of the volumetric tally mean and/or tally standard deviation
for each OpenMC cell will be generated and outputted to console.

#### Volume Calculations

It can be helpful for debugging problem setup to compare actual OpenMC cell volumes against
the `[Mesh]` element volumes to which they map. A well-designed mapping should have good
agreement between *actual* cell volumes and the `[Mesh]` volumes they map to. To add a
volume calculation, set the `volume_calculation` parameter to the name of a
[OpenMCVolumeCalculation](/userobjects/OpenMCVolumeCalculation.md) object. If you then set
`verbose = true`, you will be able to compare the cell volumes with the MOOSE elements.

#### Symmetric Data Transfers

Many neutronics problems are symmetric - such as half-core or quarter-core symmetry.
However, some MOOSE mesh generators don't
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
