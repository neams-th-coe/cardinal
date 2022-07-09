This class (and all other NekRS wrappings in Cardinal)
allows you to write slots in the `nrs->usrwrk` scratch space array
to NekRS field files. This can be useful for viewing the data sent from MOOSE
to NekRS (for problem classes that involve multiphysics), as well as to
visualize custom user usage of `nrs->usrwrk`, such as for fetching a wall distance
computation from the Nek5000 backend. To write the scratch space to a field file,
set `usrwrk_output` to an array with each "slot" in the `nrs->usrwrk` array that
you want to write. Then, specify a filename prefix to use to name each field file.

In the example below, the first two "slots" in the `nrs->usrwrk` array will be
written to field files on the same interval that NekRS writes its usual field files.
These files will be named `aaabrick0.f00001`, etc. and `cccbrick0.f00001`, etc.

!listing test/tests/nek_file_output/usrwrk/nek.i
  block=Problem
