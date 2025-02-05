If running NekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for the `[Problem]`, then the value of this postprocessor
is shown in *dimensional* units, *unless* the `field` is one of:
`scalar01`, `scalar02`, or `scalar03`,
because their dimensions are problem- and context-dependent.

If a `function` is provided to shift the field, then the
$x$, $y$, $z$, and $t$ (dimensional) coordinates are transformed to non-dimensional form
before evaluating on the NekRS mesh (which may be non-dimensional).
In other words, if the function is specified as $x+\sin(t)$ in the Cardinal
input file, then this function is evaluated as $x/L_\text{ref}+\sin(t/t_\text{ref})$
when queried on the NekRS mesh. You should specify the `function` in dimensional form.
