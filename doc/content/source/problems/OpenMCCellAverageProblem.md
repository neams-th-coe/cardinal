# OpenMCCellAverageProblem

This class couples OpenMC cell-based models (e.g. [!ac](CSG) or [!ac](DAGMC)) to MOOSE. The crux is to identify a mapping between OpenMC cells and
a [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).
The data flow contains two major steps:

- Temperature and/or density field data on the [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html)
  are volume-averaged and applied to the corresponding OpenMC cells.
- Tallies are mapped from OpenMC into `CONSTANT MONOMIAL` fields on the
  [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html) through the [tally system](AddTallyAction.md).

The smallest possible input file to run OpenMC is shown below.
This page describes this syntax, plus more advanced settings.

!listing /smallest_openmc_input.i
  id=openmc1
  caption=Smallest possible OpenMC wrapped input file.

## Initializing Variables

We first initialize
([MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html))
to communicate OpenMC's solution with MOOSE. These variables will be viewable in
the MOOSE output files (e.g., via Paraview).
Depending on the user settings, the following `CONSTANT MONOMIAL` variables will be added:

- Variable(s) representing the OpenMC tally(s) added by the `[Tallies]` block
- Variable(s) representing the temperature to read into OpenMC. Temperature will be read
  from the mesh subdomains indicated by `temperature_blocks`.
- Variable(s) representing the density to read into OpenMC. Density will be read
  from the mesh subdomains indicated by `density_blocks`.
- A variable (`cell_id`) storing the mapping of OpenMC cell IDs onto the mesh
- A variable (`cell_instance`) storing the mapping of OpenMC cell instances onto the mesh

!alert tip
These variables have default names, but you can also control their names using the
`temperature_variables`, and `density_variables` parameters. The names of the tally variables
can be customized with the `tally_name` parameter in each [tally object](AddTallyAction.md).
If you want to see the names, run with `verbose = true` and tables will print out on initialization
with this information.

!alert note
You do not need to add/initialize ANY of these variables manually - it all happens behind the scenes!

As an example, let's start with a complicated case first.
Suppose our `[Problem]` block looks like the following.
We want to apply temperature feedback from four blocks (`fuel`, `cladding`, `water`, `helium`). We also want to apply density feedback, but only from two blocks
(`water` and `fluid`). Suppose we also want to keep separate track of
different variables representing each of these physical fields, so we will want
to read temperature from a variable named `temp0` in the `fuel` and `cladding` blocks, but from
a variable named `nek_temp` in the `helium` block (and so on for density).

```
[Tallies]
  [cell_tally]
    type = CellTally
    tally_score = 'heating flux'
    tally_name = 'power openmc_flux'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  temperature_variables = 'temp0;         t_water;  nek_temp'
  temperature_blocks    = 'fuel cladding;   water;    helium'

  density_variables = 'rho_water; rho_helium'
  density_blocks = 'water; helium'
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
[Tallies]
  [cell_tally]
    type = CellTally
    tally_score = 'heating flux'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  density_blocks = 'water helium'
  temperature_blocks = 'fuel cladding water helium'
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
(i.e. the mesh in the `[Mesh]` block) is established to the OpenMC cell geometry.

!alert note
The OpenMC geometry is *always* specified in length units of centimeters.
Most other MOOSE applications use SI units, i.e. meters for the length unit.
See [#scaling] to learn how to couple OpenMC models to non-centimeter-based MOOSE applications.

In the `[Mesh]` block, you should provide a mesh onto which OpenMC will write its
tally values, as well as read temperature and density. This mesh is only used for
reading/writing data, so there are no requirements on node continuity
across elements. We loop over all the elements
and map each to an OpenMC cell according to the element centroid. As an example,
[openmc_mesh] depicts an OpenMC geometry, a mesh on which temperature feedback is to be applied,
and a visualization of the mapping from these elements to the OpenMC cells.
There are no requirements on alignment of elements/cells or on preserving volumes -
the OpenMC cells and mesh mirror elements do not need to be conformal. Elements
that don't map to an OpenMC cell simply do not participate in the multiphysics
coupling (and vice versa for the cells). This feature can be used to exclude regions
such as reflectors from multiphysics feedback.

!media nc.png
  id=openmc_mesh
  style=width:70%;margin-left:auto;margin-right:auto
  caption=Illustration of OpenMC cells and the mapping to a mesh.

!alert tip
You can visualize how the OpenMC cells map to the mesh by viewing the `cell_id`
and `cell_instance` variables Cardinal automatically outputs. For more information
on how to interpret these variables, see:
[CellIDAux](https://cardinal.cels.anl.gov/source/auxkernels/CellIDAux.html)
and [CellInstanceAux](https://cardinal.cels.anl.gov/source/auxkernels/CellInstanceAux.html).

The cell-to-element mapping should be established
with care. There are two general behavior patterns that are typically undesirable.
In [openmc_coarser], consider the case where the
`[Mesh]` has four equal-sized elements, each of volume $V$, while the OpenMC
domain has three equal-sized cells. Assume that
the power produced by each OpenMC cell is $Q$, so that the total power of the OpenMC
domain is $3Q$. By nature of the centroid mapping, one OpenMC cell will map to
a much larger region of space than the other two cells. Even though the fission
power in each OpenMC cell is $Q$, the power density will differ by a factor of
two once mapped to the `[Mesh]`. In practice, this might not have a detrimental
or even noticeable effect on temperature solutions due to the smoothing nature
of heat conduction, but you should be aware of it.

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
at the lowest level to MOOSE. This feature was added to Cardinal specifically for
TRISO applications, where heat conduction feedback is usually *not* explicitly
resolving TRISO particles (instead, you'd prefer to set each fuel pebble
or compact to a single temperature, requiring Cardinal to figure out the list
of cells (TRISO layers, matrix) filling some higher-level cell (the pebble surface,
the compact cylinder surface). The `cell_level` and `lowest_cell_level` parameters
indicate the coordinate level (relative to 0, the highest level in the geometry)
at which to "stop" the find cell routine.

!alert tip
You can visualize your OpenMC model at different geometry hierarchies using
OpenMC [plots](https://docs.openmc.org/en/stable/pythonapi/generated/openmc.Plot.html)
and setting the `level`.

As an example, the figure below shows a TRISO-fueled [!ac](HTGR) fuel assembly
from the [gas assembly](https://cardinal.cels.anl.gov/tutorials/openmc_fluid.html)
tutorial. This image shows the geomtery, colored by cell, at different levels in the
geometry. If we wanted to apply a single temperature value to all the TRISO
particles nested inside a compact, we would set `cell_level = 1` for this problem.
If we chose `cell_level = 2`, we'd be trying to set a unique temperature
for every individual TRISO particle. If we chose `cell_level = 3`, we'd be trying
to set a unique temperature for every layer in every TRISO particle.

!media triso_map.png
  id=triso_map
  caption=Cell level concept, using a TRISO assembly as an example
  style=width:100%;margin-left:auto;margin-right:auto

In many cases, the coordinate levels on which you would like to couple OpenMC
to MOOSE are not the same everywhere in the OpenMC geometry. This was the
case in [triso_map] (levels 2 and 3 only exist in parts of the domain). You can use
`lowest_cell_level` to
indicate that coupling should be performed on the specified level *except* in
regions where there are no cells at that level.

## Adding Tallies

This class takes the tally objects initialized by the `[Tallies]` block and use them to construct
tally auxvariables. At the moment there are two options for discretizing tallies spatially in Cardinal:

1. cell tallies ([CellTally](CellTally.md))
2. libMesh unstructured mesh tallies ([MeshTally](MeshTally.md))

If no tallies are specified by the `[Tallies]` block, this class adds no tally auxvariables. Each is normalized
according to the specified `power` or `source_strength` (depending on whether you are running a
$k$-eigenvalue or fixed-source problem). By default, the normalization is done against a global
tally added over the entire OpenMC domain. By setting `normalize_by_global_tally` to false, however,
the tally is instead normalized by the sum of the tally itself.

You can customize the type of score that Cardinal uses to normalize tallies to `power` with the `source_rate_normalization`
parameter. Options include:

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

For more information on the specific meanings of these various scores,
please consult the [OpenMC tally documentation](https://docs.openmc.org/en/stable/usersguide/tallies.html). All
of the tallies added are normalized with the same `source_rate_normalization` score when running in eigenvalue mode.
[tally_units] compares the units from OpenMC and the units of the AuxVariables created for all tally scores supported
by Cardinal. Note that for all area or volume units in [tally_units], that those units match whatever unit is used in the `[Mesh]`.

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
  style=width:40%;margin-left:auto;margin-right:auto

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

## Other Features

While this class mainly facilitates data transfers to and from OpenMC, a number of
other features are implemented.

#### CAD Geometry Skinning
  id=skinning

OpenMC is currently limited to setting a single
constant temperature and a single constant density in each cell. For CSG geometries,
the user needs to manually set up sub-divisions in the geometry in order to capture
spatial variation in temperature and density.
For DAGMC geometries, you can instead optionally re-generate the OpenMC cells after
each Picard iteration according to contours in temperature and/or density. With this approach, the OpenMC model can receive spatially-varying temperature and density without the user needing to manually subdivide regions of space *a priori*. To use this feature,
set the `skinner` parameter to the name of a [MoabSkinner](/userobjects/MoabSkinner.md) user object. For more information, consult the documentation for [MoabSkinner](/userobjects/MoabSkinner.md).

!alert note
This skinning feature is only available for DAGMC geometries for which the `[Mesh]`
obeys the same material boundaries in the starting `.h5m` file.

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

Setting `k_trigger` enables triggers based on $k$, and setting `k_trigger_threshold` sets the required convergence criteria.
Individual tally triggers can be set in the `tally_trigger` and `tally_trigger_threshold` parameters of the tally classes
([CellTally](CellTally.md) and [MeshTally](MeshTally.md)). Both $k$ and multiple tally triggers can be used simultaneously.

#### Controlling the OpenMC Settings

This class provides minimal capabilities to control the OpenMC simulation
settings directly from the Cardinal input file:

- `batches`: the number of batches (inactive plus active)
- `inactive_batches`: the number of inactive batches
- `openmc_verbosity`: [verbosity level](https://docs.openmc.org/en/latest/io_formats/settings.html#verbosity) in OpenMC
- `particles`: the number of particles per batch

For all of the above, a setting in the Cardinal input file will override
any settings in the OpenMC XML files.

#### Outputting the OpenMC Solution

Certain aspects of the OpenMC solution can be output as auxiliary variables to the mesh:

- `unrelaxed_tally`: unrelaxed tally; this will append `_raw` to the tally name and output to the mesh mirror
- `unrelaxed_tally_std_dev`: unrelaxed tally standard deviation; this will append `_std_dev` to the tally and output to the mesh mirror

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
