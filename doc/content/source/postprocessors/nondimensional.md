If running NekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for the `[Problem]`, then the value of this postprocessor
is shown in *dimensional* units, *unless* the `field` is one of:
`scalar01`, `scalar02`, `scalar03`, or `function`,
because their dimensions are problem- and context-dependent.
When `field = function`, any non-dimensional
transformations to the $x$, $y$, $z$, and $t$ (dimensional) coordinates are
applied before evaluating on the NekRS mesh (which may be non-dimensional).
In other words, if the function is specified as $x+y$ in the Cardinal
input file, then this function is evaluated as $x/L_\text{ref}+y/L_\text{ref}$
when queried on the NekRS mesh.
