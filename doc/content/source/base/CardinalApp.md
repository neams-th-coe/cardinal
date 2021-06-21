# CardinalApp

`CardinalApp` is the MOOSE application that can initialize nekRS and run
problems of type [FEProblem](https://mooseframework.inl.gov/source/problems/FEProblem.html), i.e. the
default problem of all MOOSE applications that represents a normal
finite element solution.

Cardinal is a MOOSE application that wraps both nekRS and OpenMC.
Because nekRS and OpenMC require different initialization procedures
(i.e. to initialize nekRS, we call a function `nekrs::setup`, but
to initialize OpenMC, we call a function `openmc_init`), Cardinal
technically consists of three different MOOSE applications -

- `CardinalApp`, which is used to run any Cardinal input file
  that does *not* use [NekRSProblem](/problems/NekRSProblem.md) or
  [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md). In other
  words, if you want to run an input representing heat conduction with capabilities
  from the MOOSE heat conduction module, with a nekRS sub-application, you need to
  run the input with the `CardinalApp`. This is the default application used
  when using `cardinal-opt`.
- [NekApp](/base/NekApp.md), which is used to run any Cardinal input file with a
  [NekRSProblem](/problems/NekRSProblem.md). To run an input file using
  `NekApp`, you need to pass `--app nek --nekrs-setup <case>` on the command line
  to indicate that the default of `CardinalApp` is not used, and by what name to
  find the nekRS input files.
- [OpenMCApp](/base/OpenMCApp.md), which is used to run any Cardinal input file
  with a [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md). To run
  an input file using `OpenMCApp`, you need to pass `--app openmc` on the command line
  to indicate that the default of `CardinalApp` is not used. Because all of OpenMC's
  input files are always named `settings.xml`, `geometry.xml`, etc., no additional
  flag is needed like for `NekApp` to point to the OpenMC input files. The files in
  the current directory are used.
