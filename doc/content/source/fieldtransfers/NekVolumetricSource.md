# NekVolumetricSource

## Description

`NekVolumetricSource` is a [FieldTransfer](AddFieldTransferAction.md) that sends a volumetric source between NekRS and MOOSE. For instance,
in the context of conservation of energy a volumetric source is the power density (units of
energy/volume). First, this object
creates an [AuxVariable](AuxVariable.md) using the name of the object; this variable will hold
the volumetric source which NekRS reads (for `direction = to_nek`) or will be written by NekRS (for
`direction = from_nek`).

The internal NekRS mesh is based on the spectral element method (with Guass-Lobatto
quadrature), whereas a coupled MOOSE applcation which wants to write/use a boundary flux will
likely be using a different quadrature rule. In order to ensure perfect conservation of the
volumetric source as it is exchanged between NekRS and MOOSE, a [Receiver](Receiver.md) postprocessor is automatically added with
name taken as `<name of the object>_integral`. For `direction = to_nek`, this postprocessor
should be populated by a parent/sub application with the total integrated volumetric source in the originating
MOOSE application. For `direction = from_nek`, this postprocessor will be filled by the
total integrated volumetric source computed internally in NekRS.

## Example Input File Syntax

As an example, the example below couples NekRS to MOOSE via [!ac](CHT). A coupled MOOSE
application sends a heat source variable and a postprocessor representing the total
integral heat source (i.e. power) into a NekRS-wrapped case.
The `NekVolumetricSource` object creates automatically
an auxiliary variable named `heat_source` and a postprocessor by the name of `source_integral`
(the default postprocessor name would have been `heat_source_integral` but you can also
specify a custom name if desired as shown here). The `NekVolumetricSource` object then handles
writing this volumetric source into usrwrk slot 0.

!listing tests/conduction/nonidentical_volume/cylinder/nek.i
  block=Problem

Then, all that is required to use a volumetric heat source transferred by MOOSE is to
apply it with a custom source kernel in the `.oudf` file. Below is an example
of a custom heat source kernel, arbitrarily named `mooseHeatSource` - this code is
executed on the [!ac](GPU), and is written in OKL, a decorated C++ kernel language.
This code loops over all the NekRS elements, loops over all the [!ac](GLL) points on
each element, and then sets the volumetric heat source equal to a heat source passed into
the `mooseHeatSource` function.

!listing /test/tests/conduction/nonidentical_volume/cylinder/cylinder.oudf language=cpp
  re=\@kernel void\smooseHeatSource.*?^}

The actual passing of the `nrs->usrwrk` scratch space (that `NekRSProblem` writes into)
into this custom kernel
occurs in the `.udf` file. In the `.udf` file, you need to define a custom function,
named artbirarily here to `userq`, with a signature that matches the user-defined source
function expected by NekRS. This function should then pass the scratch space
into the [!ac](OCCA) kernel we saw in the `.oudf` file. Note that the "offset"
here should match the slice in the scratch space array which holds the volumetric heat source.

!listing /test/tests/conduction/nonidentical_volume/cylinder/cylinder.udf language=cpp
  re=void\suserq.*?^}

Finally, be sure
to "load" the custom heat source kernel and create the pointer needed by NekRS
to call that function. These two extra steps are quite small - the entire `.udf`
file required to apply a MOOSE heat source is shown below (a large part of this
file is applying initial conditions, which is unrelated to the custom heat source).

!listing /test/tests/conduction/nonidentical_volume/cylinder/cylinder.udf language=cpp

If needed,
please consult the [NekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/detailed_usage.html#setting-custom-source-terms)
for more information on applying custom sources.

!syntax parameters /Problem/FieldTransfers/NekVolumetricSource

!syntax inputs /Problem/FieldTransfers/NekVolumetricSource
