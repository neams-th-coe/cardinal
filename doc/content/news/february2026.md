# February 2026 News

## NekRS Version Update

The NekRS version in Cardinal has been updated to version 26. This results in breaking changes to
all Cardinal files. Please consult the tutorials and tests for examples for how to update your
files. Briefly, some of the main aspects to be aware of in this update include:

- Mesh boundary IDs no longer need to be ordered sequentially beginning from 1. Use syntax like
  the following to indicate how the boundary IDs are ordered when listing the boundary types
  in the `boundaryTypeMap` fields.

```
[MESH]
boundaryIDMap = 389, 231, 4  # boundary IDs from the mesher
```
