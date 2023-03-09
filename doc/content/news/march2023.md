# March 2023 News

- We had to re-gold the entire Cardinal-OpenMC test suite due to a
  [fix upstream in OpenMC](https://github.com/openmc-dev/openmc/pull/2381) (anisotropic fission). During this re-gold, we also took the opportunity to change the cross section library required for the test suite. Previously, we were using an older ENDF library which had known wrong-values at certain temperatures (but which we used for testing, because at the time the tests were created, we weren’t aware of the issue).
  Anyone who wants to run Cardinal’s test suite will need to:

```
./scripts/download-openmc-cross-sections.sh
```

and then point `OPENMC_CROSS_SECTIONS` to that new location, `../cross_sections/endfb-vii.1-hdf5/cross_sections.xml`.
- You can now use OpenMC's stochastic volume calculations to directly compare the volumes of OpenMC cells which map to MOOSE. This can be useful for debugging model preparation. Simply set `volume_calculation` on `OpenMCCellAverageProblem` to the [OpenMCVolumeCalculation](/userobjects/OpenMCVolumeCalculation.md) user object.
