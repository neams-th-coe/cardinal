# MoabSkinner

!syntax description /UserObjects/MoabSkinner

## Description

This user object bins the spatial domain according to three characteristics:

- temperature (by grouping elements into `n_temperature_bins` with equal spacing between `temperature_min` and `temperature_max`)
- density (by grouping elements into `n_density_bins` with equal spacing between `density_min` and `density_max`)
- subdomain

For example, suppose the domain consists of two subdomains, with temperature and density
distributions as shown in the top row of [moab_skinner]. The elements are then grouped into
individualized bins, according to unique combinations of the temperature bin, the density
bin, and the subdomain bin. In the lower right of [moab_skinner] is then shown the *surface*
skins produced internally, which can be used to define the boundaries of new regions on
which to track particles.

!media moab_skinner.png
  id=moab_skinner
  caption=Mesh skinning operation
  style=width:90%;margin-left:auto;margin-right:auto

!alert tip
You can visualize this user object (i.e., the bin
indices) using a [SpatialUserObjectAux](SpatialUserObjectAux.md).

### Supported Mesh Element Types

`MoabSkinner` natively supports meshes consisting of TET4 and TET10 elements, since DAGMC
geometry requires triangulated surfaces (i.e. skins of tetrahedra). Meshes containing HEX, PYRAMID, PRISM
elements are also supported: when these elements are detected, an internal all-TET4 copy of
the mesh is built automatically, and all skinning
and geometry operations are performed on that copy. The original MOOSE mesh is never
modified; temperature, density, and subdomain binning are always evaluated on the original
mesh where the auxiliary variables live. Higher order versions of these element types are also supported.

## Example Input Syntax

Below is an example input file that skins a mesh, generating the bin distributions
shown in [moab_skinner].

!listing test/tests/userobjects/moab_skinner/all_bins.i
  block=UserObjects

The binning, colored on a per-element basis as shown in [moab_skinner] can be visualized
with the [SkinnedBins](SkinnedBins.md) auxiliary kernel.

## Material Names and Blocks

For the vast majority of use cases, you will be using the `MoabSkinner` in tandem with
[OpenMCCellAverageProblem](OpenMCCellAverageProblem.md). In this case, you can rely on
OpenMC to auto-detect the mapping between materials and subdomains from the loaded `.h5m`
model, so that when the geometry is re-skinned, OpenMC will re-apply the original materials
corresponding to the regions of space. However, if you'd like to instead force the use of a
_different_ material from what was present in the original `.h5m` file, you can provide
`material_blocks` and `material_names` to have the skinner apply specific materials on
each block.

The `MoabSkinner` can also be used independently of `OpenMCCellAverageProblem` to generate
an initial DAGMC geometry (`.h5m` file) directly from a MOOSE mesh, without needing any
pre-existing DAGMC model. In this mode, `material_blocks` and `material_names` are required
in order to tag the generated volumes with OpenMC material assignments, and `output_full`
should be set to `true` to write the generated geometry to a file, then point
OpenMC's `geometry.xml` at the generated file for subsequent coupled runs.

Both mesh subdomains (in `material_blocks`) and materials (in `material_names`) may be
specified either by name or by numeric ID. The two parameters must be the same length and
are matched in order. Any mesh subdomain *not* listed in `material_blocks` receives no
material assignment, and the corresponding region will be void in OpenMC; a warning listing
these subdomains is printed at setup.

Below is an example of generating an initial DAGMC geometry directly from a MOOSE mesh

!listing test/tests/neutronics/dagmc/assign_materials/generate_dagmc.i
  block=UserObjects

!syntax parameters /UserObjects/MoabSkinner

!syntax inputs /UserObjects/MoabSkinner
