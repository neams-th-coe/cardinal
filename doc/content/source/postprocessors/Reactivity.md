# Reactivity

!syntax description /Postprocessors/Reactivity

## Description

This postprocessor extracts the combined $k$ eigenvalue from the latest OpenMC eigenvalue
calculation and calculates reactivity as:
\begin{equation}
\label{line-eq2}
reactivity = \frac{k-1}{k}
\end{equation}

## Example Input Syntax

Shown below is an example for the reactivity estimators.

!listing test/tests/postprocessors/reactivity/openmc.i
  block=Postprocessors

!syntax parameters /Postprocessors/Reactivity

!syntax inputs /Postprocessors/Reactivity
