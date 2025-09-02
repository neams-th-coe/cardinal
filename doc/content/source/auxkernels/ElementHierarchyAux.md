## ElementHierarchyAux

!syntax description /AuxKernels/ElementHierarchyAux

## Description

ElementHierarchyAux stores the element hierarchy (how many times the element went through
refinement process during AMR) in an aux variable.

## Example Input Syntax

As an example, the syntax below stores the element hierarchy in an aux variable.

!listing test/tests/auxkernels/element_hierarchy/element_hierarchy_test.i
block=AuxKernels

!syntax parameters /AuxKernels/ElementHierarchyAux

!syntax inputs /AuxKernels/ElementHierarchyAux