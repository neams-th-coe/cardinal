NekRS is most often solved in nondimensional form, such that all solution variables
are of order unity by normalizing by problem-specific characteristic scales. However,
most other MOOSE applications use dimensional units. When transferring field data to/from
NekRS or when postprocessing the NekRS solution, it is important for the NekRS solution
to match the dimensional solution of the coupled MOOSE application. For physical intuition,
it is also often helpful in many cases to visualize and interpret a NekRS solution
in dimensional form. Cardinal automatically handles conversions between
dimensional and non-dimensional form if you add a [Dimensionalize](DimensionalizeAction.md) sub-block. Please consult the documentation for [Dimensionalize](DimensionalizeAction.md) for more information.
