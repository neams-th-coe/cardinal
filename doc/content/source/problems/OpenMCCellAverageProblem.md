# OpenMCCellAverageProblem

This class performs all activities related to solving OpenMC as a MOOSE
application. This class also facilitates data transfers to/from
[MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
to exchange field data from/to OpenMC. The crux of this OpenMC wrapping
is to identify a mapping between OpenMC cells and, if present, unstructured
mesh tallies, to a [MooseMesh](https://mooseframework.inl.gov/source/mesh/MooseMesh.html).
Then, field data on the `MooseMesh` is volume-averaged and applied to the
corresponding OpenMC cells, while field data in OpenMC is directly applied
as `CONSTANT MONOMIAL` fields on the `MooseMesh`.

## Initializing MOOSE-type Field Interfaces

When beginning a coupling of OpenMC within the MOOSE framework, the first
action taken by this class is to initialize MOOSE-type variables needed to
communicate OpenMC's solution with a general MOOSE application.

First, `OpenMCCellAverageProblem` initializes [MooseVariables](https://mooseframework.inl.gov/source/variables/MooseVariable.html)
to receive data necessary for multiphysics coupling. Depending on the settings
for this class, the following variables will be added:

- `heat_source`, the OpenMC fission heat source to be sent to MOOSE
- `temp`, the MOOSE temperature to be sent to OpenMC
- `density`, the MOOSE density to be sent to OpenMC

The order of all of these variables is `CONSTANT MONOMIAL` to simplify the
averaging performed in space (for data going *in* to OpenMC) and to simplify the
application of cell/element-average heat source (for data going *out* of OpenMC)
to the `MooseMesh`. The initialization of MOOSE variables happens behind the scenes -
for instance, below is a complete input file that will run OpenMC as a MOOSE
application.

!listing /smallest_openmc_input.i

In this particular example, we indicated that we are going to be coupling OpenMC
through both temperature (by setting `solid_blocks` and/or `fluid_blocks`) and
density (by setting `fluid_blocks`) coupling to MOOSE. Heat source coupling is
always assumed. Therefore, the first thing that `OpenMCCellAverageProblem` does is
to essentially add the following to the input file:

!listing
[AuxVariables]
  [temp]
    order = CONSTANT
    family = MONOMIAL
  []
  [density]
    order = CONSTANT
    family = MONOMIAL
  []
  [heat_source]
    order = CONSTANT
    family = MONOMIAL
  []
[]

This auxiliary variable addition happens automatically to simplify input file creation.

## Cell to Element Mapping

After adding the variables to receive/extract coupling data from,
a mapping from the elements in the `MooseMesh` (i.e. the mesh in the `[Mesh]` block
in the OpenMC-wrapped input file) is established to the OpenMC geometry
(as specified in a `geometry.xml` file).

## Overall Calculation Methodology

`OpenMCCellAverageProblem` inherits from the [ExternalProblem](https://mooseframework.inl.gov/source/problems/ExternalProblem.html)
class. For each time step, the calculation proceeds according to the `ExternalProblem::solve()` function.
Data gets sent into OpenMC, OpenMC runs a "time step"
(actually a k-eigenvalue calculation), and data gets extracted from OpenMC.
`OpenMCCellAverageProblem` mostly consists of defining the `syncSolutions` and `externalSolve` methods.

!listing /framework/src/problems/ExternalProblem.C
  re=void\sExternalProblem::solve.*?^}

### External Solve
  id=solve

The actual solve of a "time step" by OpenMC is peformed within the
`externalSolve` method, which performs the following.

!listing /src/base/OpenMCCellAverageProblem.C
  re=void\sOpenMCCellAverageProblem::externalSolve.*?^}

