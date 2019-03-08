# Cardinal

## Build Instructions

The top-level Makefile should build your app in one step.  You may build from another directory and
point it to Cardinal's makefile using `make -f`.  The Makefile variables you should set (either in
the file or through the `make` command) are Nek-related:

* `NEK_CASEDIR`:  The path to the Nek input files (.usr, SIZE, mesh, and restart  files)
* `NEK_CASENAME`:  The name of the Nek .usr file

After compiling, you should have the executable `cardinal-<casename>-opt` in the top-level cardinal
directory (no matter where you build everything else).  The `<casename>` is the `NEK_CASENAME` that
you provided to the makefile.  
