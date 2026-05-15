# AddCriticalitySearchAction

## Overview

The `AddCriticalitySearchAction` is responsible for performing a complete criticality
search during the OpenMC solve. Examples of parameters which can be used for the
criticality search include:

- [OpenMCMaterialDensity](OpenMCMaterialDensity.md), to change a material's total density
- [BoratedWater](BoratedWater.md), to change the boron weight ppm in water
- [RotationSearch](RotationSearch.md), to rotate OpenMC cell(s) fill

The converged value of the criticality search will automatically be populated into
a postprocessor named `critical_value`.

!alert warning
There are two tolerances that must be specified `root_tol` (the tolerance on the root the search is looking for, e.g. a critical drum angle or boron appm concentration) and `k_tol`(the tolerance on the actual tallied k-eigenvalue).  If the `root_tol` is too large, the solver might stop short of the actual critical value. If the `k_tol` is too small (on the order of or smaller than the statistical standard deviation), convergence may not be possible. If the search fails to converge, use a looser `k_tol` or increase the number of particles. Both of these tolerances are absolute tolerances, so it is important, especially on `root_tol` to input something reasonable for the problem at hand. For example, specifying a `root_tol = 0.001` is a very strict tolerance for finding a critical drum angle, which may span from 0 to 180. Conversely, a `root_tol` of 50 would likely be too large of a tolerance for drum angle but may be acceptable for critical boron concentration.

## Example Input File Syntax

As an example, a [OpenMCMaterialDensity](OpenMCMaterialDensity.md) object is used to perform
a criticality search based on the total density of material 1.

!listing /tests/criticality/material_density/openmc.i
  block=Problem

When running this case, Cardinal will print out a table showing a summary of the results of
the criticality search.

```
---------------------------------------------------------------------------
| Iteration | material 1 density [kg/m3] |   k (mean)   |   k (std dev)   |
---------------------------------------------------------------------------
|         0 |               1.000000e+03 | 6.620000e-02 |    1.098139e-03 |
|         1 |               3.000000e+04 | 1.621666e+00 |    2.077345e-02 |
|         2 |               1.840971e+04 | 1.102440e+00 |    1.108761e-02 |
|         3 |               1.634907e+04 | 9.942194e-01 |    1.174444e-02 |
|         4 |               1.645914e+04 | 9.961657e-01 |    1.040603e-02 |
|         5 |               1.666816e+04 | 1.008298e+00 |    1.162806e-02 |
|         6 |               1.652520e+04 | 1.013949e+00 |    1.304124e-02 |
|         7 |               1.647338e+04 | 9.847013e-01 |    1.192703e-02 |
|         8 |               1.650048e+04 | 9.890565e-01 |    1.018419e-02 |
|         9 |               1.651135e+04 | 1.010180e+00 |    1.071604e-02 |
|        10 |               1.650611e+04 | 1.001364e+00 |    1.163008e-02 |
|        11 |               1.650539e+04 | 9.971522e-01 |    1.117813e-02 |
|        12 |               1.650588e+04 | 1.020902e+00 |    1.023352e-02 |
|        13 |               1.650545e+04 | 9.994847e-01 |    1.192011e-02 |
|        14 |               1.650546e+04 | 1.002346e+00 |    1.093321e-02 |
|        15 |               1.650546e+04 | 9.943650e-01 |    7.631514e-03 |
---------------------------------------------------------------------------
```

!syntax list /Problem/CriticalitySearch actions=false subsystems=false heading=Available CriticalitySearch Objects

!syntax parameters /Problem/CriticalitySearch/AddCriticalitySearchAction
