# Getting NekRS Tools

The Nek development team provides many useful tools for streamlining
the use of NekRS. The binaries you can build, as well as their purpose, include:

- `exo2nek`: Convert between Exodus meshes to the custom NekRS `.re2` mesh format
- `gmsh2nek`: Convert between Gmsh meshes to the custom NekRS `.re2` mesh format

To build these tools, separately clone the Nek5000 repository and then
navigate to the `tools` directory and run the makefile to compile all the programs.

```
git clone https://github.com/Nek5000/Nek5000.git
cd Nek5000/tools
./maketools all
```

This will create executables in the `Nek5000/bin` directory. You may want to add
this to your path in order to quickly access these programs.
