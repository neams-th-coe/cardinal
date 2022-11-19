This page describes how data is sent from NekRS to MOOSE with
`syncSolutions(FROM_EXTERNAL_APP)`.
These steps are summarized in [steps3].

!media steps3.png
  id=steps3
  caption=Data transfers that occur to the MOOSE-Wrapped Nek file from NekRS
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

Specifically, we
mean that we read from the internal arrays in NekRS and write into `AuxVariables`
in the NekRS-wrapped input file (say, `nek.i`) - NOT necessarily that NekRS is
sending new information to some coupled MOOSE application. A detailed breakdown
of the steps is:

- For conjugate heat transfer, write NekRS boundary temperature into the `temp` variable.
   This will print out something like the following. The printed "Interpolated max/min values"
   refer to the max/min values once the data has been mapped to the `NekRSMesh`.

```
Extracting NekRS temperature from boundary 1
Interpolated temperature min/max values: 0.0, 1.0
```

- For volumetric heating, write NekRS volume temperature into the `temp` variable.
   This will print out something like the following.  The printed "Interpolated max/min values"
   refer to the max/min values once the data has been mapped to the `NekRSMesh`.

```
Extracting NekRS temperature volume
Interpolated temperature min/max values: 0.0, 1.0
```

- If outputting additional quantities with `outputs`, write NekRS solution fields
   into additional variables in the Nek-wrapped input file. This will print out
   something like

```
Interpolating vel_x, vel_y NekRS solution onto mesh mirror
```
