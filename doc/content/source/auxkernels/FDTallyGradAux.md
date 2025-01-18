# FDTallyGradAux

## Description

`FDTallyGradAux` approximates tally gradients using forward finite differences between the current element and it's neighbors.
The approach used is based on the derivative approximation presented in [!cite](stolte):

\begin{equation}
\label{tally_grad}
\vec{\nabla}u(\vec{x}_{i}) \approx Y_{i}^{-1}\sum_{i'} \frac{\vec{x}_{i'} - \vec{x}_{i}}{||\vec{x}_{i'} - \vec{x}_{i}||}\frac{u(\vec{x}_{i'}) - u(\vec{x}_{i})}{||\vec{x}_{i'} - \vec{x}_{i}||}\text{,}
\end{equation}
\begin{equation}
\label{tally_grad_Y}
Y_{i} = \sum_{i'} (\vec{x}_{i'} - \vec{x}_{i})\otimes (\vec{x}_{i'} - \vec{x}_{i}) \text{,}
\end{equation}
where $i$ indicates the current element, $i'$ is a neighboring element, $\vec{x}$ is an element's centroid, and $u$ is the tally field. A tally `score` must be specified, and if any external filter bins have been added through the use of the [filter system](AddFilterAction.md) `ext_filter_bin` index should be provided.

## Example Input Syntax

This example showcases how `FDTallyGradAux` can be used to approximate the gradient of `kappa_fission`:

!listing test/tests/neutronics/tally_grad/finite_diff/openmc.i
  block=AuxKernels

!syntax parameters /AuxKernels/FDTallyGradAux

!syntax inputs /AuxKernels/FDTallyGradAux
