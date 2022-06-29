# NekRSSeparateDomainProblem

This class allows for coupling of NekRS to a 1D T/H code such as SAM or THM.
This coupling is performed using the "separate-domain" coupling strategy,
where the coupling between codes is performed purely by
updating boundary conditions between the domains. For most applications where there's a large flow loop,
 the 1D T/H code should model most of the loop, and NekRS should model only a portion of the loop that
 is of interest for CFD simulation.

!include nek_classes.md

The coupling's setup is controlled using the `coupling_type`, which provides information for how the
 NekRS is coupled to the 1D T/H code, via NekRS's `inlet`, `outlet`, or `'inlet outlet'`.

!alert warning
This class currently only supports dimensional solutions coming from NekRS. Nondimensional support is in progress.

### Velocity, temperature, and scalar coupling

[inlet_outlet] shows the coupling of velocity (V), temperature (T), and scalar01 (S01)
 for `coupling_type = 'inlet outlet'`, where the 1D T/H code is coupled to NekRS's inlet and outlet boundaries.

Scalar coupling is allowed for up to 3 scalars (scalar01, scalar02, and scalar03) using the optional `coupled_scalars` parameter.
 Coupling scalars is useful when running a 1D T/H coupled to a NekRS RANS k-tau simulation with
 coupled passive scalar transport. For such a case, NekRS may use k and tau as scalar01 and scalar02, respectively,
 and the coupled passive scalar as scalar03. For coupling just scalar03, one would set `coupled_scalars = scalar03`.

!media NekRSSeparateDomainProblem_inletoutlet.png
  id=inlet_outlet
  caption=Example boundary transfers for 'inlet outlet' coupling.
  style=width:100%;margin-left:auto;margin-right:auto;halign:center

All `inlet_XX` are single values passed from the 1D T/H domain to the NekRS inlet boundary provided by
 `inlet_boundary`. For information on implementing these transferred values,
 refer to [this section](#using-transferred-data-to-inform-nekrs-inlet-boundary-conditions) of the documentation.

All `outlet_XX` values are automated [NekSideAverage](/postprocessors/NekSideAverage.md) postProcessors
 that are created on the NekRS boundary ID given by `outlet_boundary`.
These postProcessors can then be transferred to the 1D T/H code using the
 [MultiAppPostprocessorTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppPostprocessorTransfer.html)
system.

### Pressure coupling

Pressure information is transferred between NekRS and the 1D T/H code using a global pressure drop because
 the 1D T/H code only needs the overall pressure drop over the NekRS domain.
`inlet_P` and `outlet_P` [NekSideAverage](/postprocessors/NekSideAverage.md) postProcessors are created along
 with the pressure drop `dP` postProcessor.
[dP] shows this process, which is performed no matter what `coupling_type` is given. `dP` can be transferred
 to the 1D T/H code using the [MultiAppPostprocessorTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppPostprocessorTransfer.html) system.


!media NekRSSeparateDomainProblem_dP.png
  id=dP
  caption=Example pressure drop calculation over the NekRS domain.
  style=width:50%;margin-left:auto;margin-right:auto;halign:center

## Transferring data between NekRS and the 1D T/H code

A summary of postProcessors generated is shown in the following table for velocity, temperature, and scalar01 coupling.
 PostProcessors for scalar02 and scalar03 coupling follow the same pattern as scalar01.
The postProcessor created depends on (1) the `coupling_type` given, (2) whether or not NekRS is solving for temperature
 and (3) the optional `coupling_scalars` provided.

| postProcessor created | `coupling_type = inlet` | `coupling_type = outlet` | `coupling_type = 'inlet outlet'` |
| - | - | - | - |
| dP | $\checkmark$ | $\checkmark$ | $\checkmark$ |
| inlet_V | $\checkmark$ | &nbsp; | $\checkmark$ |
| outlet_V | &nbsp; | $\checkmark$ | $\checkmark$ |
| inlet_T | $\checkmark$, if temperature solved | &nbsp; | $\checkmark$, if temperature solved |
| outlet_T | &nbsp; | $\checkmark$, if temperature solved | $\checkmark$, if temperature solved |
| inlet_S01 | $\checkmark$, if scalar01 in `coupled_scalars` | &nbsp; | $\checkmark$, if scalar01 in `coupled_scalars` |
| outlet_S01 | &nbsp; | $\checkmark$, if scalar01 in `coupled_scalars` | $\checkmark$, if scalar01 in `coupled_scalars` |



Example MultiApp transfers for velocity, temperature and scalar01 coupling are shown below for
 `coupling_type = 'inlet outlet'` and `coupled_scalars = 'scalar01'`. NekRS can run as either
 the MainApp or SubApp, but these example transfers assume NekRS is acting as the MainApp and the `1D_TH_code` as the SubApp.

```
[Transfers]
  [dP_transfer]
    type = MultiAppPostprocessorTransfer
    to_multi_app = 1D_TH_code
    from_postprocessor = dP
    to_postprocessor   = fromNekRS_pressureDrop
  []
  [inlet_V_transfer]
    type = MultiAppPostprocessorTransfer
    from_multi_app = 1D_TH_code
    reduction_type = average
    from_postprocessor = toNekRS_velocity
    to_postprocessor   = inlet_V
  []
  [inlet_T_transfer]
    type = MultiAppPostprocessorTransfer
    from_multi_app = 1D_TH_code
    reduction_type = average
    from_postprocessor = toNekRS_temperature
    to_postprocessor   = inlet_T
  []
  [inlet_S01_transfer]
    type = MultiAppPostprocessorTransfer
    from_multi_app = 1D_TH_code
    reduction_type = average
    from_postprocessor = toNekRS_scalar01
    to_postprocessor   = inlet_S01
  []
  [outlet_V_transfer]
    type = MultiAppPostprocessorTransfer
    to_multi_app = 1D_TH_code
    from_postprocessor = outlet_V
    to_postprocessor   = fromNekRS_velocity
  []
  [outlet_T_transfer]
    type = MultiAppPostprocessorTransfer
    to_multi_app = 1D_TH_code
    from_postprocessor = outlet_T
    to_postprocessor   = fromNekRS_temperature
  []
  [outlet_S01_transfer]
    type = MultiAppPostprocessorTransfer
    to_multi_app = 1D_TH_code
    from_postprocessor = outlet_S01
    to_postprocessor   = fromNekRS_scalar01
  []
[]
```

## Using transferred data for NekRS inlet boundary conditions

Data is "sent" into NekRS by writing into the `nrs->usrwrk` scratch space array,
which NekRS makes available within the boundary condition functions in the `.oudf` file
(on device, this array is technically called the `nrs->o_usrwrk` array).
[usrwrk_nrssdp] shows the assignment of "slots" in the `nrs->usrwrk` scratch space
array with quantities written by Cardinal. Because different quantities are written into
Cardinal depending on the problem setup, if a particular slice is not needed for a
case, it will just hold zero values. That is, the *order* of the various quantities
is always the same in `nrs->usrwrk`.

!table id=usrwrk_nrssdp caption=Quantities written into the scratch space array by Cardinal
| Slice | Quantity | When Will There be Non-Zero Values? | How to Access in the `.oudf` File |
| :- | :- | :- | :- |
| 0 | Inlet velocity | if `coupling_type` includes `inlet`  | `bc->wrk[0 * bc->fieldOffset + bc->idM]` |
| 1 | Inlet temperature | if `coupling_type` includes `inlet` and NekRS's case files include a temperature solve | `bc->wrk[1 * bc->fieldOffset + bc->idM]` |
| 2 | Inlet scalar01 | if `coupling_type` includes `inlet` and `coupled_scalars` includes `scalar01` | `bc->wrk[2 * bc->fieldOffset + bc->idM]` |
| 3 | Inlet scalar02 | if `coupling_type` includes `inlet` and `coupled_scalars` includes `scalar02` | `bc->wrk[3 * bc->fieldOffset + bc->idM]` |
| 4 | Inlet scalar03 | if `coupling_type` includes `inlet` and `coupled_scalars` includes `scalar03` | `bc->wrk[4 * bc->fieldOffset + bc->idM]` |

The total number of slots in the scratch space that are allocated by Cardinal
is controlled with the `n_usrwrk_slots` parameter.
If you need to use extra slices in `nrs->usrwrk` for other custom user actions,
simply set `n_usrwrk_slots` to be greater than the number of slots strictly needed
for coupling. At the start of your Cardinal simulation, a table will be printed to the
screen to explicitly tell you what each slice in the scratch space holds.
Any extra slots are noted as `unused`, and are free for non-coupling use.

For example, if your case couples NekRS to MOOSE with `coupling_type = inlet`
and `coupled_scalars = scalar02`, but has a NekRS case without a temperature solve,
the slices normally dedicated to storing inlet temperature and scalar01 are still
allocated (because we keep the order the same in `nrs->usrwrk`), but just won't hold
any meaningful information.
A table similar to the following would print out at the start of your simulation.
You could use slices 4 onwards for custom purposes.

!listing id=l11 caption=Table printed at start of Cardinal simulation that describes available scratch space for a case that couples NekRS to MOOSE via an inlet and with the second passive scalar, but without the first passive scalar or a temperature solve. A total of 7 slots are allocated by setting `n_usrwrk_slots` to 7
------------------------------------------------------------------
| Slice |  Quantity   |        How to Access in NekRS BCs        |
------------------------------------------------------------------
|     0 | velocity    |  bc->wrk[0 * bc->fieldOffset + bc->idM]  |
|     1 | temperature |  bc->wrk[1 * bc->fieldOffset + bc->idM]  |
|     2 | scalar01    |  bc->wrk[2 * bc->fieldOffset + bc->idM]  |
|     3 | scalar02    |  bc->wrk[3 * bc->fieldOffset + bc->idM]  |
|     4 | unused      |  bc->wrk[4 * bc->fieldOffset + bc->idM]  |
|     5 | unused      |  bc->wrk[5 * bc->fieldOffset + bc->idM]  |
|     6 | unused      |  bc->wrk[6 * bc->fieldOffset + bc->idM]  |
------------------------------------------------------------------

!alert warning
Allocation of `nrs->usrwrk` and `nrs->o_usrwrk` is done automatically by
`NekRSSeparateDomainProblem`. If you attempt to run a NekRS input file that accesses `bc->wrk` in the
`.oudf` file *without* a Cardinal executable (i.e. using something like
`nrsmpi case 4`), then that scratch space will have to be manually allocated in
the `.udf` file, or else your input will seg fault. This use case will not be typically
encountered by most users, but if you really do want to run the NekRS input files
intended for a Cardinal case with the NekRS executable (perhaps for debugging),
we recommend simply replacing `bc->wrk` by a dummy value, such as `bc->s = 0.0`
for the inlet scalar use case. This just replaces a value that normally comes from MOOSE by a fixed
value. All other aspects of the NekRS case files should not require modification.

In other words, the scratch space slots contain pointwise values for each (singly-valued)
postprocessor value on all the [!ac](GLL) points on the corresponding boundary.
So, the values in each "slot" correspond to the following postprocessors:

- Slot 0: `inlet_V` postprocessor value
- Slot 1: `inlet_T` postprocessor value
- Slot 2: `inlet_S01` postprocessor value
- Slot 3: `inlet_S02` postprocessor value
- Slot 4: `inlet_S03` postprocessor value

This allows the user the freedom to choose what type of profile they want to implement at the inlet boundary, i.e. flat or fully-developed. Below are a few example implementations.

Example for a flat velocity(x) profile:

!listing language=cpp
void velocityDirichletConditions(bcData *bc)
{
  bc->u = bc->wrk[bc->idM];
  bc->v = 0.0;
  bc->w = 0.0;
}

Example for a flat temperature and scalar profiles:

!listing language=cpp
void scalarDirichletConditions(bcData *bc)
{
  if(bc->scalarId==0) bc->s  = bc->wrk[bc->idM + bc->fieldOffset];   // temperature
  if(bc->scalarId==1) bc->s  = bc->wrk[bc->idM + 2*bc->fieldOffset]; // scalar01
  if(bc->scalarId==2) bc->s  = bc->wrk[bc->idM + 3*bc->fieldOffset]; // scalar02
  if(bc->scalarId==3) bc->s  = bc->wrk[bc->idM + 4*bc->fieldOffset]; // scalar03
}

Example for fully-developed velocity(x) profile for laminar pipe flow:

!listing! language=cpp
void velocityDirichletConditions(bcData *bc)
{
  dfloat yi = bc->y;
  dfloat zi = bc->z;
  dfloat rsq  = yi*yi + zi*zi;
  dfloat Rsq  = 0.5*0.5; // Radius^2
  dfloat Uavg  = bc->wrk[bc->idM];

  bc->u = 2*Uavg*(1-rsq/Rsq);
  bc->v = 0.0;
  bc->w = 0.0;
}
!listing-end!

!syntax parameters /Problem/NekRSSeparateDomainProblem

!syntax inputs /Problem/NekRSSeparateDomainProblem

!syntax children /Problem/NekRSSeparateDomainProblem

