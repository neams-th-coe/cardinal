# NekRSSeparateDomainProblem

This class allows for coupling of NekRS to a 1D T/H code such as SAM or THM. This coupling is performed using the "separate-domain" coupling strategy, where the coupling between codes is performed purely by
updating boundary conditions between the domains. For most applications where there's a large flow loop, the 1D T/H code should model most of the loop, and NekRS should model only a portion of the loop that is of interest for CFD simulation.

The coupling's setup is controlled using the `coupling_type`, which provides information for how the NekRS is coupled to the 1D T/H code, via NekRS's `inlet`, `outlet`, or `inlet outlet`.

!alert warning
This class currently only supports dimensional solutions coming from NekRS. Nondimensional support is in progress.

### Velocity and temperature coupling

[inlet_outlet] shows the coupling of velocity and temperature for `coupling_type = inlet outlet`, where the 1D T/H code is coupled to NekRS's inlet and outlet boundaries.

!media NekRSSeparateDomainProblem_inletoutlet.png
  id=inlet_outlet
  caption=Example velocity and temperature transfers for 'inlet outlet' coupling.
  style=width:100%;margin-left:auto;margin-right:auto;halign:center

`inlet_V` and `inlet_T` are both single values passed from the 1D T/H domain to the NekRS inlet boundary given by `inlet_boundary`. For information on implementing these transferred values, refer to [this section](#using-transferred-data-to-inform-nekrs-inlet-boundary-conditions) of the documentation.

`outlet_V` and `outlet_T` are automated [NekSideAverage](/postprocessors/NekSideAverage.md) postProcessors that are created on the NekRS boundary ID given by `outlet_boundary`.
These postProcessors can then be transferred to the 1D T/H code using the [MultiAppPostprocessorTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppPostprocessorTransfer.html)
system.

### Pressure coupling

Pressure information is transferred between NekRS and the 1D T/H code using a global pressure drop because the 1D T/H code only needs the overall pressure drop over the NekRS domain.
`inlet_P` and `outlet_P` [NekSideAverage](/postprocessors/NekSideAverage.md) postProcessors are created along with the pressure drop `dP` postProcessor.
[dP] shows this process, which is performed no matter what `coupling_type` is given.`dP` can be transferred to the 1D T/H code using the [MultiAppPostprocessorTransfer](https://mooseframework.inl.gov/source/transfers/MultiAppPostprocessorTransfer.html) system.


!media NekRSSeparateDomainProblem_dP.png
  id=dP
  caption=Example pressure drop calculation over the NekRS domain.
  style=width:50%;margin-left:auto;margin-right:auto;halign:center

## Transferring data between NekRS and the 1D T/H code

A summary of postProcessors generated is shown in the following table. The creation of these postProcessors depends on (1) the `coupling_type` given and (2) whether or not NekRS is solving for temperature.

| postProcessor created | inlet | outlet | inlet outlet |
| - | - | - | - |
| dP | $\checkmark$ | $\checkmark$ | $\checkmark$ |
| inlet_V | $\checkmark$ | &nbsp; | $\checkmark$ |
| outlet_V | &nbsp; | $\checkmark$ | $\checkmark$ |
| inlet_T | $\checkmark$, if temperature solved | &nbsp; | $\checkmark$, if temperature solved |
| outlet_T | &nbsp; | $\checkmark$, if temperature solved | $\checkmark$, if temperature solved |


Example MultiApp transfers are shown below for `coupling_type = 'inlet outlet'` with transfers between the NekRS MainApp and the `1D_TH_code` SubApp. These transfers include all possible postProcessors created.

```
[Transfers]
  [dP_transfer]
    type = MultiAppPostprocessorTransfer
    multi_app = 1D_TH_code
    direction = to_multiapp
    from_postprocessor = dP
    to_postprocessor   = fromNekRS_pressureDrop
  []
  [outlet_V_transfer]
    type = MultiAppPostprocessorTransfer
    multi_app = 1D_TH_code
    direction = to_multiapp
    from_postprocessor = outlet_V
    to_postprocessor   = fromNekRS_velocity
  []
  [outlet_T_transfer]
    type = MultiAppPostprocessorTransfer
    multi_app = 1D_TH_code
    direction = to_multiapp
    from_postprocessor = outlet_T
    to_postprocessor   = fromNekRS_temperature
  []
  [inlet_V_transfer]
    type = MultiAppPostprocessorTransfer
    multi_app = 1D_TH_code
    direction = from_multiapp
    reduction_type = average
    from_postprocessor = toNekRS_velocity
    to_postprocessor   = inlet_V
  []
  [oulet_V_transfer]
    type = MultiAppPostprocessorTransfer
    multi_app = 1D_TH_code
    direction = from_multiapp
    reduction_type = average
    from_postprocessor = toNekRS_temperature
    to_postprocessor   = inlet_T
  []
[]
```

## Using transferred data for NekRS inlet boundary conditions

All transferred velocity and temperature information can be accessed and implemented from within the NekRS simulation's `.oudf` file using the following syntax:

- `inlet_V` is accessed via `bc->wrk[bc->idM]`
- `inlet_T` is accessed via `bc->wrk[bc->idM + bc->fieldOffset]`

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

Example for a flat temperature profile:

```
void scalarDirichletConditions(bcData *bc)
{
  bc->s  = bc->wrk[bc->idM + bc->fieldOffset];
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

