!alert warning
Allocation of `nrs->usrwrk` and `nrs->o_usrwrk` is done automatically.
If you attempt to run a NekRS input file that accesses `bc->usrwrk` in the
`.oudf` file *without* a Cardinal executable (e.g. like
`nrsmpi case 4`), then that scratch space will have to be manually allocated in
the `.udf` file, or else your input will seg fault. This will not be typically
encountered by most users, but if you really do want to run the NekRS input files
intended for a Cardinal case with the NekRS executable (perhaps for debugging),
we recommend simply replacing `bc->usrwrk` by a dummy value, such as `bc->flux = 0.0`
for the boundary heat flux use case. This just replaces a value that normally comes from MOOSE by a fixed
value. All other aspects of the NekRS case files should not require modification.
