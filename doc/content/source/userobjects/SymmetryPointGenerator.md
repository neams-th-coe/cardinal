# SymmetryPointGenerator

!syntax description /UserObjects/SymmetryPointGenerator

## Description

This user object maps from a point $(x_0, y_0, z_0)$ to a new point
$(x_1, y_1, z_1)$ according to either a mirror-symmetric or rotationally-symmetric
mapping. This class is most commonly used to transfer data between
a symmetric OpenMC model and a whole-domain `[Mesh]`.

!alert note
Elements with centroids that *perfectly* align with
one of the symmetry planes can occasionally not map correctly due
to roundoff errors. It is recommended to use meshes that avoid this possibility.
It can be helpful when setting up symmetric models to visualize the transformed
$x_1$, $y_1$, and $z_1$ coordinates - the
[PointTransformationAux](/auxkernels/PointTransformationAux.md) can be used for
this purpose.

### Mirror Symmetry

Mirror symmetry, also referred to as "half-symmetry," reflects a point across a general plane
with origin `(0.0, 0.0, 0.0)` and normal given by the `normal` parameter.
Points on the "positive"
side of the plane (the direction in which the normal points) are reflected across
the plane.
For example, [symmetry] shows a half-symmetric OpenMC model of a fuel assembly
and the mesh to which data is sent. The `normal` is shown as
$\hat{n}$.

!media symmetry_transfers.png
  id=symmetry
  caption=Illustration of symmetric data transfers to/from OpenMC

### Rotational Symmetry

This class also supports general rotational symmetry, such as to define quarter-symmetry
in the unit circle.
Again, you specify the `normal` to define *one*
of the planes bounding the symmetric region. Then, you must also provide a
`rotation_axis` about which to rotate, and a `rotation_angle` (in degrees) to indicate
the angle of the symmetry sector (sweeping clockwise from the vector $\hat{n}\times\hat{m}$, where $\hat{m}$
is the `rotation_axis`). For example, [symmetry_b] shows a 1/6th symmetric OpenMC model
of a fuel assembly and the mesh to which data is sent. The `normal` is
shown as $\hat{n}$. For this case, the `rotation_axis` is set to the positive $z$ axis,
pointing out of the picture. The `rotation_angle` is then 60 degrees.

!media symmetry_transfers_b.png
  id=symmetry_b
  caption=Illustration of symmetric angular data transfers to/from OpenMC

## Example Input Syntax

Below is an example input file that constructs the rotational symmetry
shown in [symmetry_b].

!listing test/tests/neutronics/symmetry/rotational/openmc.i
  block=UserObjects

!syntax parameters /UserObjects/SymmetryPointGenerator

!syntax inputs /UserObjects/SymmetryPointGenerator

!syntax children /UserObjects/SymmetryPointGenerator
