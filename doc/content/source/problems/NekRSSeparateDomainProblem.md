# NekRSSeparateDomainProblem

This class allows for coupling of NekRS to a 1D T/H code such as SAM or THM.
This coupling is performed using the "separate-domain" coupling strategy,
where the coupling between codes is performed purely by
updating boundary conditions between the domains. For most applications where there's a large flow loop,
 the 1D T/H code should model most of the loop, and NekRS should model only a portion of the loop that
 is of interest for CFD simulation.

The coupling's setup is controlled using the `coupling_type`, which provides information for how the
 NekRS is coupled to the 1D T/H code, via NekRS's `inlet`, `outlet`, or `inlet outlet`.

!alert warning
This class currently only supports dimensional solutions coming from NekRS. Nondimensional support is in progress.

### Velocity, temperature, and scalar coupling

[inlet_outlet] shows the coupling of velocity (V), temperature (T), and scalar01 (S01)
 for `coupling_type = inlet outlet`, where the 1D T/H code is coupled to NekRS's inlet and outlet boundaries.

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

| postProcessor created | `coupling_type = inlet` | `coupling_type = outlet` | `coupling_type = inlet outlet` |
| - | - | - | - |
| dP | $\checkmark$ | $\checkmark$ | $\checkmark$ |
| inlet_V | $\checkmark$ | &nbsp; | $\checkmark$ |
| outlet_V | &nbsp; | $\checkmark$ | $\checkmark$ |
| inlet_T | $\checkmark$, if temperature solved | &nbsp; | $\checkmark$, if temperature solved |
| outlet_T | &nbsp; | $\checkmark$, if temperature solved | $\checkmark$, if temperature solved |
| inlet_S01 | $\checkmark$, if scalar01 in `coupled_scalars` | &nbsp; | $\checkmark$, if scalar01 in `coupled_scalars` |
| outlet_S01 | &nbsp; | $\checkmark$, if scalar01 in `coupled_scalars` | $\checkmark$, if scalar01 in `coupled_scalars` |



Example MultiApp transfers for velocity, temperature and scalar01 coupling are shown below for
 `coupling_type = 'inlet outlet'` and `coupled_scalars = 'scalar01'`. These transfers assume NekRS
 is acting as the MainApp and the `1D_TH_code` as the SubApp.

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
    from_postprocessor = toNekRS_velocity
    to_postprocessor   = inlet_V
  []
  [inlet_T_transfer]
    type = MultiAppPostprocessorTransfer
    from_multi_app = 1D_TH_code
    from_postprocessor = toNekRS_temperature
    to_postprocessor   = inlet_T
  []
  [inlet_S01_transfer]
    type = MultiAppPostprocessorTransfer
    from_multi_app = 1D_TH_code
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

All transferred velocity, temperature, and scalar information can be accessed and implemented from within the NekRS simulation's `.oudf` file using the following syntax:

- `inlet_V` is accessed via `bc->wrk[bc->idM]`
- `inlet_T` is accessed via `bc->wrk[bc->idM + bc->fieldOffset]`
- `inlet_S01` is accessed via `bc->wrk[bc->idM + 2*bc->fieldOffset]`
- `inlet_S02` is accessed via `bc->wrk[bc->idM + 3*bc->fieldOffset]`
- `inlet_S03` is accessed via `bc->wrk[bc->idM + 4*bc->fieldOffset]`

This allows the user the freedom to choose what type of profile they want to implement at the inlet boundary, i.e. flat or fully-developed. Below are a few example implementations.

Example for a flat velocity(x) profile:

```
void velocityDirichletConditions(bcData *bc)
{
  bc->u = bc->wrk[bc->idM];
  bc->v = 0.0;
  bc->w = 0.0;
}
```

Example for a flat temperature and scalar profiles:

```
void scalarDirichletConditions(bcData *bc)
{
  if(bc->scalarId==0) bc->s  = bc->wrk[bc->idM + bc->fieldOffset];   // temperature
  if(bc->scalarId==1) bc->s  = bc->wrk[bc->idM + 2*bc->fieldOffset]; // scalar01
  if(bc->scalarId==2) bc->s  = bc->wrk[bc->idM + 3*bc->fieldOffset]; // scalar02
  if(bc->scalarId==3) bc->s  = bc->wrk[bc->idM + 4*bc->fieldOffset]; // scalar03
}
```

Example for fully-developed velocity(x) profile for laminar pipe flow:

```
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
```

!syntax parameters /Problem/NekRSSeparateDomainProblem

!syntax inputs /Problem/NekRSSeparateDomainProblem

!syntax children /Problem/NekRSSeparateDomainProblem

