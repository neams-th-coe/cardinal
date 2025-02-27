# Building Cardinal on HPC Systems

Cardinal has several dependancies, and so compiling it on HPC systems can be a daunting task. This page contains common resources and tips to streamline the process of getting Cardinal built on HPC systems.

## Resources and Tips for Building

Almost all HPC systems use [Lmod](https://lmod.readthedocs.io/en/latest/), which is a Lua-based environment management system. You'll use Lmod to load the modules you need to build Cardinal - some helpful commands include:

- `module list MOD_NAME`, shows you any modules currently loaded which include `MOD_NAME`. If you don't specify a module name it shows all currently loaded modules.
- `module spider MOD_NAME`, shows you all available modules which include `MOD_NAME`. If you don't specify a module name it will show you all available modules on the HPC (not recommended!).
- `module load MOD_NAME1 MOD_NAME2`, loads the module (makes it available to your shell session) with the names `MOD_NAME1` and `MOD_NAME2`.
- `module reset`, restores your shell session to the HPC system default modules.
- `module purge`, unloads all currently loaded modules. Some modules are mandatory and so Lmod will refuse to unload them.

Before you start loading modules, it's best to consult the minimum requirements of MOOSE, NekRS, OpenMC, and DAGMC:

1. MOOSE: [https://mooseframework.inl.gov/getting_started/installation/hpc_install_moose.html](https://mooseframework.inl.gov/getting_started/installation/hpc_install_moose.html)
2. NekRS: [https://github.com/Nek5000/nekRS?tab=readme-ov-file#build-instructions](https://github.com/Nek5000/nekRS?tab=readme-ov-file#build-instructions)
3. OpenMC: [https://docs.openmc.org/en/stable/usersguide/install.html#prerequisites](https://docs.openmc.org/en/stable/usersguide/install.html#prerequisites)
4. DAGMC: [https://svalinn.github.io/DAGMC/install/dependencies.html](https://svalinn.github.io/DAGMC/install/dependencies.html)

Out of the above requirements MOOSE's are the strictest. They can be found below:

!include sqa/minimum_requirements.md

From here, you will need to find a set of modules that satisfy these minimum requirements. This tends to involve a substantial amount of trial and error which depends a lot on what each HPC system offers. It's best to consult the documentation for your specific HPC system to see which modules they recommend for specific requirements of the MOOSE software stack.

Once you've loaded a set of modules that you believe will compile Cardinal, you need to set up a collection of environment variables. Assuming your Cardinal installation is located in a folder called `projects` in your home directory, they are:

```bash
# These are hints for the compiler that should be used while building.
export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# This lets the build scripts know that you want all of Cardinal's optional dependencies.
export ENABLE_NEK=yes
export ENABLE_OPENMC=yes
export ENABLE_DAGMC=yes

# This points to the actual location where your Cardinal directory is stored on the HPC's file system.
# Most HPC systems use symbolic links for directories and files to hide their true paths (to make life
# easier for users), which is why this is necessary.
DIRECTORY_WHERE_YOU_HAVE_CARDINAL=$HOME/projects
HOME_DIRECTORY_SYM_LINK=$(realpath -P $DIRECTORY_WHERE_YOU_HAVE_CARDINAL)
export NEKRS_HOME=$HOME_DIRECTORY_SYM_LINK/cardinal/install

# These environment variables indicate how many processes you want to build with. Keep in mind that the
# more processes you specify the more memory is consumed during a build. Be careful with the number
# of jobs, most HPC systems limit the number of processes and memory you can consume when
# on a login node. If you find that your builds are crashing with an exit code of 1 while building,
# you've likely exceeded either the CPU or memory limit on the machine - reducing the number of processes
# should mitigate those crashes.
export JOBS=8
export LIBMESH_JOBS=8
export MOOSE_JOBS=8
```

The best place to put these exports is your `.bashrc` to avoid needing to export these environment variables every time you spin up a new shell. Once these variables have been exported and you've loaded a set of modules that you believe will compile Cardinal, you can run MOOSE diagnostics to make sure you haven't missed anything:

```bash
./contrib/moose/scripts/diagnostics.sh
```

If critical errors are reported at this stage, it is likely because your current module set does not meet MOOSE's requirements. If you receive an error stating that `jinja2` is missing, you can safely move on.

**Note:** If you want to build NekRS with GPU support, you need GPU-enabled compilers (CUDA compilers, OpenCL compilers, etc.) on the login node. Some HPC systems only allow users to load those modules on nodes which contain GPUs. If that is the case for you, you'll need to build with a job script (see the next section). To enable a GPU build, set one of the following environment variables in Cardinal's makefile to `1` (depending on the specific GPUs on your system): `OCCA_CUDA_ENABLED`, `OCCA_HIP_ENABLED`, or `OCCA_OPENCL_ENABLED`. You'll also need to make sure you load modules with GPU compilers.

From here, you can run the commands below to build MOOSE's submodules. We recommend building with `nohup` or `screen` to avoid getting timed out while you're connected to a login node.

```
nohup ./contrib/moose/scripts/update_and_rebuild_petsc.sh &
```

```
nohup ./contrib/moose/scripts/update_and_rebuild_libmesh.sh &
```

```
nohup ./contrib/moose/scripts/update_and_rebuild_wasp.sh &
```

Building libMesh may fail due to a lack of libraries that are normally included by default on Linux machines. The most common culprit is `libxdr`, which is required for XDR binary file output in libMesh/MOOSE. The vast majority of the time you'll use Exodus binary output when running Cardinal, and so XDR can be disabled by adding two flags to `update_and_rebuild_libmesh.sh`.

```
./contrib/moose/scripts/update_and_rebuild_libmesh.sh --disable-xdr-required --disable-xdr
```
There are many similar flags like this, that can disable parts of libMesh or ask libMesh to install dependencies if it couldn't find them itself. If you encounter persistent errors, ask on the [MOOSE discussion page](https://github.com/idaholab/moose/discussions) for help, and they may be able to recommend a flag for you to set.
If you didn't get any build errors when building PETSc/libMesh/WASP, you can run

```
nohup make -j8 MAKEFLAGS=-j8 &
```

to build MOOSE, Cardinal, and all of Cardinal's dependencies. Occasionally the MOOSE Solid Mechanics module will fail to build due to missing `F77` compilers. If that is the case, you can either find mpif77 compilers in a different module set or disable the Solid Mechanics module by setting `SOLID_MECHANICS := no` in Cardinal's makefile.
The only MOOSE module you absolutely must have enabled is the `REACTOR` module (some of Cardinal's source files link against utilities in that module). Otherwise, feel free to disable all others.
## Building on a Compute Node

Some GPU systems will force you to build on a compute node to use GPU-specific compilers. This means you'll need to add all of the commands to a job script to be executed; an example of what this looks like can be found below:

```bash
#!/bin/bash -l

# The job script header goes here. What's provided below is an example for PBSClusterPro (used at ANL), yours will be different.
#PBS -A startup
#PBS -l select=1:ncpus=128:mpiprocs=128
#PBS -l walltime=01:00:00
#PBS -q compute
#PBS -j oe
#PBS -N build_cardinal

#PBS -m bea
#PBS -M <your_email_address>
# End header

# Load your modules here...
modules load MOD_1 MOD_2 ... MOD_N
# End loading modules.

export CC=mpicc
export CXX=mpicxx
export FC=mpif90

export ENABLE_NEK=yes
export ENABLE_OPENMC=yes
export ENABLE_DAGMC=yes

DIRECTORY_WHERE_YOU_HAVE_CARDINAL=$HOME/projects
HOME_DIRECTORY_SYM_LINK=$(realpath -P $DIRECTORY_WHERE_YOU_HAVE_CARDINAL)
export NEKRS_HOME=$HOME_DIRECTORY_SYM_LINK/cardinal/install

# Change these to the number of processes you want to build with on a compute node.
export JOBS=32
export LIBMESH_JOBS=32
export MOOSE_JOBS=32

# You might need to change directories into the location where the job was launched,
# which should be the Cardinal repo directory.
cd $PBS_O_WORKDIR

./contrib/moose/scripts/update_and_rebuild_petsc.sh > petsc_log.txt
./contrib/moose/scripts/update_and_rebuild_libmesh.sh > libmesh_log.txt
./contrib/moose/scripts/update_and_rebuild_wasp.sh > wasp_log.txt

# Change this to the number of processes you want to build with on a compute node.
make -32 MAKEFLAGS=-32 > cardinal_log.txt
```

You'd then submit your build job script to the HPC queue (for PBS the command is `qsub job_script_name`). We dump the output of each component of the build to different logfiles; this allows us to follow the build in real-time and makes it easier to parse for errors.

In addition to building from inside a job script, you can also build in interactive mode. This is equivalent to submitting a job with no script and an interactive flag, which will open a shell terminal on a compute node after waiting in queue. For PBS the command is:

```
qsub -I -A <PROJECT_NAME> -l select=1:ncpus=PROC:mpiprocs=PROC,walltime=01:00:00 -q compute
```

which will open an interactive job with `PROC` cores in the compute queue for one hour. After getting your interactive shell, you can build Cardinal as if you were on a login node using the instructions in the previous section.
