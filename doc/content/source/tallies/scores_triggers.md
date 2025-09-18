Cardinal supports several tally scores which can be specified in the `score` parameter when adding
a tally. Options include:

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
- `flux`: particle scalar flux
- `H3_production`: tritium production reaction rate
- `total`: the total reaction rate
- `absorption`: the absorption reaction rate
- `scatter`: the scattering reaction rate
- `nu_scatter`: the scattering reaction rate combined with the total (n, xn) reaction rate
- `fission`: the fission reaction rate
- `nu_fission`: the fission neutron production rate
- `prompt_nu_fission`: the prompt fission neutron production rate
- `delayed_nu_fission`: the delayed fission neutron production rate
- `decay_rate`: the neutron precursor decay constant weighted by the delayed fission neutron production rate
- `inverse_velocity`: one over the particle velocity weighted by the particle flux

!alert note
To obtain the true precursor decay constant or inverse velocity, you need to divide `decay_rate` by `delayed_nu_fission`
and `inverse_velocity` by `flux` (respectively). This requires the addition of either `delayed_nu_fission` or `flux` and some
additional post-processing.

For more information on the specific meanings of these various scores,
please consult the [OpenMC tally documentation](https://docs.openmc.org/en/stable/usersguide/tallies.html).
[tally_units] compares the units from OpenMC and the units of the AuxVariables created for all tally scores supported
by Cardinal. Note that for all area or volume units in [tally_units], that those units match whatever unit is used in the `[Mesh]`.
The names of the tally auxvariables added by the [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md) can be modified by
specifying a name for each score in `name`.

!table id=tally_units caption=Tally units from OpenMC and the conversion in Cardinal.
| Tally score | OpenMC Units | Cardinal Units |
| :- | :- | :- | :- |
| `heating` | eV / source particle | W / volume |
| `heating_local` | eV / source particle | W / volume |
| `kappa_fission` | eV / source particle | W / volume |
| `fission_q_prompt` | eV / source particle | W / volume |
| `fission_q_recoverable` | eV / source particle | W / volume |
| `damage_energy` | eV / source particle | eV / volume / second |
| `flux` | particle - cm / source particle | particle / area / second |
| `H3_production` | tritium / source particle | tritium / volume / second |
| `total` | total reactions / source particle | total reactions / volume / second |
| `absorption` | absorption reactions / source particle | absorption reactions / volume / second |
| `scatter` | scattering reactions / source particle | scattering reactions / volume / second |
| `nu_scatter` | nu-scattering reactions / source particle | nu-scattering reactions / volume / second |
| `fission` | fission reactions / source particle | fission reactions / volume / second |
| `nu_fission` | nu-fission reactions / source particle | nu-fission reactions / volume / second |
| `prompt_nu_fission` | prompt nu-fission reactions / source particle | prompt nu-fission reactions / volume / second |
| `delayed_nu_fission` | delayed nu-fission reactions / source particle | delayed nu-fission reactions / volume / second |
| `decay_rate` | delayed nu-fission reactions / source particle / s | delayed nu-fission reactions / volume / second^2 |
| `inverse_velocity` | particle-seconds / source particle | particles / volume |

This tally is capable of controlling OpenMC execution through the use of
[tally triggers](https://docs.openmc.org/en/latest/pythonapi/generated/openmc.Trigger.html?highlight=trigger). Tallies currently
support active batch termination through a relative error indicator, which can be enabled by setting `trigger` to `rel_err`
for each score in the tally. Scores which do not need to control active batch termination can have a value of `none` set. If enabled,
an appropriate relative error must be set for each score in `trigger_threshold`.

Certain aspects of tally results can be output as auxiliary variables to the mesh:

- `unrelaxed_tally`: unrelaxed tally; this will append `_raw` to the tally name and output to the mesh mirror
- `unrelaxed_tally_rel_error`: unrelaxed tally relative error; this will append `_rel_error` to the tally name and output to the mesh mirror
- `unrelaxed_tally_std_dev`: unrelaxed tally standard deviation; this will append `_std_dev` to the tally and output to the mesh mirror
