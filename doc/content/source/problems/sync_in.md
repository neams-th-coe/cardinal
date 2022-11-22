This page describes how data is sent from MOOSE to NekRS with
`syncSolutions(TO_EXTERNAL_APP)`.
These steps are summarized in [steps1].

!media steps1.png
  id=steps1
  caption=Data transfers that occur from the MOOSE-Wrapped Nek file into NekRS
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

Specifically, we read from
`AuxVariables` in the NekRS-wrapped input file (say, `nek.i`) into NekRS's
internal arrays - NOT necessarily that new information is being sent into
NekRS from some coupled MOOSE application. A detailed breakdown of the steps is:

- For conjugate heat transfer, send boundary heat flux from `avg_flux` variable into NekRS's internal arrays. This will print out something like:

```
Sending heat flux to NekRS boundary 1
Normalizing total NekRS flux of 1.5 to the conserved MOOSE value of 1.52
```

- For volumetric heating, send volumetric power density from `heat_source` variable into NekRS's internal arrays. This will print out something like:

```
Sending volumetric heat source to NekRS
Normalizing total NekRS heat source of 1.5 to the conserved MOOSE value of 1.52
```

- Copy the scratch space to the device.
- For mesh deformation coupling, send the boundary displacement from the `displacement_x`, `displacement_y`, and `displacement_z` variables into NekRS's internal arrays. This will print out something like the following, after which we call various functions in NekRS to update the geometric terms associated with the new mesh.

```
Sending volume deformation to NekRS
```

- For mesh deformation coupling, copy the mesh deformation information from host to device.
