# Tutorial 10: Assessing Convergence

In this tutorial, you will learn how to:

- Use helper scripts in Cardinal to assess convergence of the OpenMC solution
  and of a general MOOSE application

This tutorial provides a description of several helper scripts
available in the `cardinal/scripts` directory that can be used for:

- Determining an appropriate number of inactive batches for OpenMC to ensure
  convergence of the fission source and $k$ eigenvalue
- Determining an appropriate cell discretization in OpenMC in the presence
   of thermal feedback
- Determining appropriate mesh convergence of a generic MOOSE application

## Convergence of the OpenMC Model

When building an OpenMC model, there are several parameters that affect
the convergence:

- Number of inactive batches, which must be sufficiently high to ensure
  that the fission distribution and $k$ are converged before beginning
  to tally in the active batches
- Number of active batches, which must be sufficiently high to reach a
  desired uncertainty in $k$ and the tallies
- Cell discretization, such as the number of axial layers in a pincell-type
  geometry or the number of radial cell divisions in a pebble-type geometry,
  such that the actual temperature and density distributions passed into
  OpenMC are converged in terms of the effect on the Monte Carlo transport physics

To ensure that a sufficient number of active batches are used, we recommend
using the [trigger system](https://cardinal.cels.anl.gov/source/problems/OpenMCCellAverageProblem.html)
in the OpenMC wrapping, which will terminate the number of active batches once
reaching a desired uncertainty in $k$ and/or the `kappa-fission` tally. To
determine an appropriate nuber of inactive batches and a cell discretization,
Cardinal contains several helper scripts. This tutorial describes how to use
these scripts.

!alert note
You will need to install OpenMC's Python API to use the OpenMC convergence
helper scripts. Please follow the instructions [here](https://docs.openmc.org/en/stable/usersguide/install.html#installing-python-api).

### Inactive Batches

To determine an appropriate number of inactive batches, you can use the
`scripts/inactive_study.py` script. To run this script, you need:

- A Python script that creates your OpenMC model for you; this Python
  script *must* take `-s` as a boolean argument that indicates a Shannon
  entropy mesh will be added, and `-n <layers>` to indicate the number
  of cell divisions to create
- A Cardinal input file that runs your OpenMC model given some imposed
  temperature and density distribution

For example, to run an inactive cycle study for the model developed
for [Tutorial 6C](https://cardinal.cels.anl.gov/tutorials/gas_compact.html),
we would run:

```
$ cd tutorials/gas_compact
$ python ../../scripts/inactive_study.py -i unit_cell -input openmc.i
```

since the Python script used to generate the OpenMC model is named
`unit_cell.py` and our Cardinal input file is named `openmc.i`. You should edit
the beginning of the `inactive_study.py` script to select the number
of cell layers and number of inactive cycles to consider:

!listing /scripts/inactive_study.py language=python

When the script finishes running, plots of Shannon entropy and $k$, along
with the saved statepoint files from the simulations, will be available
in a directory named `inactive_study`. Below are shown example images of
the Shannon entropy and $k$ plots. You would then select the number of
inactive cycles to be the point at which both the Shannon entropy and $k$
reach stationarity.

!media example_entropy_plots.png
  id=entropy
  caption=Example plots that are generated as part of the inactive study, showing Shannon entropy as a function of inactive batch and number of cell divisions
  style=width:80%;margin-left:auto;margin-right:auto;halign:center

!media example_k_plots.png
  id=k
  caption=Example plots that are generated as part of the inactive study, showing $k$ as a function of inactive batch and number of cell divisions; the horizontal line shows the average of $k$ over the last 100 cycles
  style=width:80%;margin-left:auto;margin-right:auto;halign:center
