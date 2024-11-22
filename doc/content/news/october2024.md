# October 2024 News

- We have enabled Adaptive Mesh Refinement (AMR) on the OpenMC mesh mirror for simulations using an [OpenMCCellAverageProblem](/problems/OpenMCCellAverageProblem.md)

  - All AMR capabilities included in MOOSE are now natively supported for temperature/density feedback and tallies
  - Mesh tallies will automatically use the adapted mesh when tallying on the mesh mirror

- There are a few limitations to [MeshTally](/tallies/MeshTally.md) AMR, namely:

  - Only replicated meshes are supported
  - The mesh mirror must be tallied on directly when AMR is active and a mesh tally is included in the input file
