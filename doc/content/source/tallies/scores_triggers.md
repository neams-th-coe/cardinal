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
- `scatter`: the scatter reaction rate
- `fission`: the fission reaction rate

For more information on the specific meanings of these various scores,
please consult the [OpenMC tally documentation](https://docs.openmc.org/en/stable/usersguide/tallies.html). The names of
the tally auxvariables added by the [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md) can be modified by specifying a name
for each score in `name`.

This tally is capable of controlling OpenMC execution through the use of
[tally triggers](https://docs.openmc.org/en/latest/pythonapi/generated/openmc.Trigger.html?highlight=trigger). Tallies currently
support active batch termination through a relative error indicator, which can be enabled by setting `trigger` to `rel_err`
for each score in the tally. Scores which do not need to control active batch termination can have a value of `none` set. If enabled,
an appropriate relative error must be set for each score in `trigger_threshold`.

Certain aspects of tally results can be output as auxiliary variables to the mesh:

- `unrelaxed_tally`: unrelaxed tally; this will append `_raw` to the tally name and output to the mesh mirror
- `unrelaxed_tally_std_dev`: unrelaxed tally standard deviation; this will append `_std_dev` to the tally and output to the mesh mirror
