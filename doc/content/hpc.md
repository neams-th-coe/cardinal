# HPC Systems

This page collects
instructions for building and running Cardinal on a number of [!ac](HPC) systems.
Because default modules and
settings change on [!ac](HPC) systems with time, the instructions below may become deprecated, but
we try to keep this information up-to-date.
Note that the absence of a particular [!ac](HPC) system from this list does not imply that Cardinal will not
build/run on that system - only that documentation has not yet been created. If you can't find your system here,
we provide some [resources and tips for building / running Cardinal on HPC systems](hpc_build_run_tips.md).

In addition to these provided module and environment settings,
you must follow the build instructions on the
[Getting Started](start.md) page.

!alert note
NekRS can sometimes fail to correctly pre-compile its kernels on these [!ac](HPC)
systems. We recommend precompiling NekRS (with the `nrspre` script) if you run into
issues. See the [NekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/just_in_time_compilation.html)
for more information.

## Improv

Last updated on 01/15/2025

[Improv](https://docs.lcrc.anl.gov/improv/getting-started-improv/)
is an [!ac](HPC) system at [!ac](ANL) with 825 AMD EPYC dual-socket
nodes (128 cores per node).

!listing! language=bash caption=Sample `~/.bashrc` for Improv id=im1
module purge
module load gcc/11.4.0
module load openmpi/5.0.0-gcc-11.4.0
module load cmake/3.27.4
module load anaconda3/2024.10

export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# Revise for your Cardinal repository location
DIRECTORY_WHERE_YOU_HAVE_CARDINAL=$HOME

# This is needed because your home directory on Improv is actually a symlink
HOME_DIRECTORY_SYM_LINK=$(realpath -P $DIRECTORY_WHERE_YOU_HAVE_CARDINAL)
export NEKRS_HOME=$HOME_DIRECTORY_SYM_LINK/cardinal/install

# Revise for your cross sections location
export OPENMC_CROSS_SECTIONS=$HOME_DIRECTORY_SYM_LINK/cross_sections/endfb-vii.1-hdf5/cross_sections.xml
!listing-end!

!listing scripts/job_improv language=bash caption=Sample job script for Improv with the `startup` project code id=im2

## Bebop

Last updated on 07/09/2023

[Bebop](https://docs.lcrc.anl.gov/bebop/getting-started-bebop/)
is an [!ac](HPC) system at [!ac](ANL) with an Intel Broadwell
partition (36 cores/node) and an Intel Knights Landing partition
(64 cores/node).

!alert note
Note that if you want to *build* Cardinal via a job script, you will also
need to `module load numactl/2.0.12-355ef36` because make can find `libnuma-dev`
on the login nodes, but you need to explicitly load it for compute nodes.

!listing! language=bash caption=Sample `~/.bashrc` for Bebop id=bb1
module purge
module load gcc/9.2.0-pkmzczt
module load openmpi/4.1.1-ckyrlu7
module load cmake/3.20.3-vedypwm
module load python/intel-parallel-studio-cluster.2019.5-zqvneip/3.6.9

export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# Revise for your Cardinal repository location
DIRECTORY_WHERE_YOU_HAVE_CARDINAL=$HOME

# This is needed because your home directory on Bebop is actually a symlink
HOME_DIRECTORY_SYM_LINK=$(realpath -P $DIRECTORY_WHERE_YOU_HAVE_CARDINAL)
export NEKRS_HOME=$HOME_DIRECTORY_SYM_LINK/cardinal/install

# Revise for your cross sections location
export OPENMC_CROSS_SECTIONS=$HOME_DIRECTORY_SYM_LINK/cross_sections/endfb-vii.1-hdf5/cross_sections.xml
!listing-end!

!listing scripts/job_bebop language=bash caption=Sample job script for Bebop with the `startup` project code id=bb2

## Polaris

Last updated on 01/25/2025

[Polaris](https://docs.alcf.anl.gov/polaris/getting-started/)
is an [!ac](HPC) system at [!ac](ANL) with 560 AMD EPYC single-socket
nodes (32 cores per node). Each node also contains four NVIDIA A100
GPU accelerators. In order to build Cardinal with GPU support, set
the appropriate variable in the `Makefile` to true (`1`):

```
OCCA_CUDA_ENABLED=1
OCCA_HIP_ENABLED=0
OCCA_OPENCL_ENABLED=0
```

Additionally, Polaris does not provide an MPI-wrapped F77 compiler and
so the MOOSE solid mechanics module must be disabled in the `Makefile`:

```
SOLID_MECHANICS     := no
```

When building the PETSc, libMesh, and Wasp dependencies from the scripts, you'll also
need to pass some additional settings to libMesh to disable XDR output.

```
./contrib/moose/scripts/update_and_rebuild_petsc.sh
./contrib/moose/scripts/update_and_rebuild_libmesh.sh --disable-xdr-required --disable-xdr
./contrib/moose/scripts/update_and_rebuild_wasp.sh
```

!listing! language=bash caption=Sample `~/.bashrc` for Polaris id=po1
module restore
module use /soft/modulefiles
module load PrgEnv-gnu
module load nvhpc-mixed/23.9
module load craype-accel-nvidia80
module load cudatoolkit-standalone/12.5.0
module load craype-x86-milan
module load spack-pe-base cmake
module load cray-python/3.11.5

export CC=cc
export CXX=CC
export FC=ftn

export F77=ftn
export F90=ftn

export ENABLE_NEK=yes
export ENABLE_OPENMC=yes
export ENABLE_DAGMC=yes

# Revise for your Cardinal repository location
DIRECTORY_WHERE_YOU_HAVE_CARDINAL=$HOME

# This is needed because your home directory on Polaris is actually a symlink
HOME_DIRECTORY_SYM_LINK=$(realpath -P $DIRECTORY_WHERE_YOU_HAVE_CARDINAL)
export NEKRS_HOME=$HOME_DIRECTORY_SYM_LINK/cardinal/install

# Revise for your cross sections location
export OPENMC_CROSS_SECTIONS=$HOME_DIRECTORY_SYM_LINK/cross_sections/endfb-vii.1-hdf5/cross_sections.xml

export CARDINAL_DIR=$HOME_DIRECTORY_SYM_LINK/cardinal
!listing-end!

!listing scripts/job_polaris language=bash caption=Sample job script for Polaris id=po2

## Frontier

Last updated on 02/17/2024

Frontier is an [!ac](HPC) system at [!ac](ORNL) with 9408 AMD compute nodes, each with
4 AMD MI250X, each with 2 Graphics Compute Dies (GCDs), which you can think of as
representing 8 GPUs per node.
Remember that in order to build Cardinal with GPU support, set the appropriate
variable in the `Makefile` to true (`1`):

```
OCCA_CUDA_ENABLED=0
OCCA_HIP_ENABLED=1
OCCA_OPENCL_ENABLED=0
```

When building the PETSc, libMesh, and Wasp dependencies from the scripts, you'll also
need to pass some additional settings to libMesh.

```
./contrib/moose/scripts/update_and_rebuild_petsc.sh
./contrib/moose/scripts/update_and_rebuild_libmesh.sh --enable-xdr-required --with-xdr-include=/usr/include
./contrib/moose/scripts/update_and_rebuild_wasp.sh
```

!listing! language=bash caption=Sample `~/.bashrc` for Frontier id=fr1
if [ $LMOD_SYSTEM_NAME = frontier ]; then
    module purge
    module load PrgEnv-gnu craype-accel-amd-gfx90a cray-mpich rocm cray-python/3.9.13.1 cmake/3.21.3
    module unload cray-libsci

    # Revise for your Cardinal repository location
    DIRECTORY_WHERE_YOU_HAVE_CARDINAL=$HOME/frontier
    cd $DIRECTORY_WHERE_YOU_HAVE_CARDINAL

    HOME_DIRECTORY_SYM_LINK=$(realpath -P $DIRECTORY_WHERE_YOU_HAVE_CARDINAL)
    export NEKRS_HOME=$HOME_DIRECTORY_SYM_LINK/cardinal/install

    export OPENMC_CROSS_SECTIONS=/lustre/orion/fus166/proj-shared/novak/cross_sections/endfb-vii.1-hdf5/cross_sections.xml
fi
!listing-end!

!listing scripts/job_frontier language=bash caption=Sample job script for Frontier id=fr2


## Nek5k

Last updated on 08/25/2022

Nek5k is a cluster at [!ac](ANL) with 40 nodes, each with 40 cores.
We use conda to set up a proper environment on Nek5k for running Cardinal.
To use this environment, you will need to follow these steps *the first time*
you use Nek5k:

- The first time you log in, run from the command line:

```
module load openmpi/4.0.1/gcc/8.3.1 cmake openmpi/4.0.1/gcc/8.3.1-hdf5-1.10.6 anaconda3/anaconda3
conda init
```

- Log out, then log back in
- Add the following to your `~/.bashrc`:

```
module load openmpi/4.0.1/gcc/8.3.1 cmake openmpi/4.0.1/gcc/8.3.1-hdf5-1.10.6 anaconda3/anaconda3
conda activate
```

- Log out, then log back in

After following these steps, you should not require any further actions
to run Cardinal on Nek5k. Your `~/.bashrc` should look something like
below. The content within the `conda initialize` section is added
automatically by conda when you perform the steps above.

!listing! language=bash caption=Sample `~/.bashrc` for Nek5k id=n1
# Source global definitions
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi

module load openmpi/4.0.1/gcc/8.3.1 cmake openmpi/4.0.1/gcc/8.3.1-hdf5-1.10.6 anaconda3/anaconda3

conda activate

# Update for your Cardinal repository location
export NEKRS_HOME=$HOME/cardinal/install

# >>> conda initialize >>>
# !! Contents within this block are managed by 'conda init' !!
__conda_setup="$('/shared/anaconda3/bin/conda' 'shell.bash' 'hook' 2> /dev/null)"
if [ $? -eq 0 ]; then
    eval "$__conda_setup"
else
    if [ -f "/shared/anaconda3/etc/profile.d/conda.sh" ]; then
        . "/shared/anaconda3/etc/profile.d/conda.sh"
    else
        export PATH="/shared/anaconda3/bin:$PATH"
    fi
fi
unset __conda_setup
# <<< conda initialize <<<

# need to point to a newer CMake version
export PATH=/shared/cmake-3.24.2/bin:$PATH
!listing-end!

!listing scripts/job_nek5k language=bash caption=Sample job script for Nek5k id=nk

## Pinchot

Last updated on 02/24/2025

Pinchot is a small Ubuntu server at the University of Illinois hosted in Dr. Novak's lab. While not an [!ac](HPC) system per se, we include these instructions on this page to facilitate development among the Cardinal team. There is no job queue on Pinchot.

!listing! language=bash caption=Sample `~/.bashrc` for Pinchot id=p1

module load openmpi/ubuntu/5.0.0
module load hdf5/ubuntu/1.14.3
module load cmake/3.29.3

# change to your Cardinal location (either the shared location in /shared/data,
# or to a location in your home directory
CARDINAL_DIR=$(realpath -P /shared/data/cardinal)
#CARDINAL_DIR=$(realpath -P /home/ajnovak2/cardinal)
export NEKRS_HOME=$CARDINAL_DIR/install

export LIBMESH_JOBS=80
export MOOSE_JOBS=80
export JOBS=80
export HDF5_ROOT=/software/HDF5-1.14.3-ubuntu22

# revise for your cross section location
export OPENMC_CROSS_SECTIONS=/shared/data/endfb-vii.1-hdf5/cross_sections.xml

export ENABLE_DAGMC=true
export PATH=${PATH}:${NEKRS_HOME}/bin:${CARDINAL_DIR}
export MOOSE_DIR=$CARDINAL_DIR/contrib/moose
export PYTHONPATH=$MOOSE_DIR/python:${PYTHONPATH}
!listing-end!

## Sawtooth

Last updated on 02/04/2025

[Sawtooth](https://hpcweb.hpcondemand.inl.gov/hardware/sawtooth/)
 is an [!ac](HPC) system at [!ac](INL) with 99,792 cores (48 cores per node).
 The max number of cores a job can run is 80,000, resulting in a max number of
 nodes of 1666. Every job should use `ncpus=48` to maximize the power of each
 node requested. The below script requests 1 node (`select=1`). In order to
 maximizie performance on the node, always use `ncpus = mpiprocs * ompthreads = 48`,
 and make sure that `--n-threads` in the job launch equals the same value as
 `ompthreads`.

!listing! language=bash caption=Sample `~/.bashrc` for Sawtooth id=st1
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi

if [ -f  ~/.bashrc_local ]; then
       . ~/.bashrc_local
fi

module purge
module load use.moose
module load moose-dev

# Revise for your repository location
export NEKRS_HOME=$HOME/cardinal/install
export OPENMC_CROSS_SECTIONS=$HOME/cross_sections/endfb-vii.1-hdf5/cross_sections.xml
!listing-end!

!listing scripts/job_sawtooth language=bash caption=Sample job script with the `moose` project code id=st2

## Bitterroot

Last updated on 06/19/2025

[Bitterroot](https://hpcweb.hpcondemand.inl.gov/hardware/bitterroot/)
 is an [!ac](HPC) system at [!ac](INL). It has over 2 Petaflops of performance and has over 43,000 cores. It is a 43008-core Dell Commodity Technology Systems-2 (CTS-2) with 384 total nodes. Bitterroot has 90 TB of total memory.

!alert note
When building on Bitterroot, it is recommended that you build in interactive mode on a compute node in the build partition.
This can be done with an interactive job specifying `--partition build`. This allows you to build with more processes
then a login node while maintaining an internet connection.

!listing! language=bash caption=Sample `~/.bashrc` for Bitterroot id=bt1
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi

if [ -f  ~/.bashrc_local ]; then
       . ~/.bashrc_local
fi

# Bitterroot modules
if [[ "$SHORTHOST" ~= br[0-9]+ ]] || [[ "$SHORTHOST" =~ bitterroot* ]]
  echo "Loading Bitterroot modules..."
  module purge
  module load openmpi/4.1.5_ucx1.14.1 cmake/3.29.3 python/3.10-miniforge-24.3.0
fi

export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# Revise for your Cardinal location
export NEKRS_HOME=${HOME}/cardinal/install

# Revise for your cross section location
export OPENMC_CROSS_SECTIONS=${HOME}/cross_sections/endfb-vii.1-hdf5/cross_sections.xml

!listing-end!

This submission script is not optimized for any particular problem run with Cardinal:

!listing scripts/job_bitterriver language=bash caption=Sample job script with the `moose` project code id=bt2

This submission script optimizes performance for OpenMC on Bitterroot using shared memory parallelism and NUMBA bindings on a compute node:

!listing scripts/job_bitterriver_openmc language=bash caption=Sample OpenMC job script with the `moose` project code id=bt3

## WindRiver

Last updated on 06/19/2025

[WindRiver](https://hpcweb.hpcondemand.inl.gov/hardware/windriver/) is an [!ac](HPC) system at [!ac](INL). It has 843 compute nodes, each with
2 Intel Xeon Platinum 8480+ CPUs which results in 112 cores per node. The `.bashrc` and submission script are similar
to that of Bitterroot as the machines utilize the same compute hardware and modules.

!listing! language=bash caption=Sample `~/.bashrc` for WindRiver id=wnd1
# .bashrc template for users
# then source a bashrc_local file in home dir...

# Source global definitions

if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

# Source local definitions

if [ -f  ~/.bashrc_local ]; then
       . ~/.bashrc_local
fi

# WindRiver modules
if [[ "$SHORTHOST" ~=wr[0-9]+n[0-9]+ ]] || [[ "$SHORTHOST" =~ windriver* ]]
  echo "Loading WindRiver modules..."
  module purge
  module load openmpi/4.1.5_ucx1.14.1 cmake/3.29.3 python/3.10-miniforge-24.3.0
fi

export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# Revise for your Cardinal location
export NEKRS_HOME=${HOME}/cardinal/install

# Revise for your cross section location
export OPENMC_CROSS_SECTIONS=${HOME}/cross_sections/endfb-vii.1-hdf5/cross_sections.xml

!listing-end!

This submission script is not optimized for any particular problem run with Cardinal:

!listing scripts/job_bitterriver language=bash caption=Sample job script with the `moose` project code id=wnd2

This submission script optimizes performance for OpenMC on WindRiver using shared memory parallelism and NUMBA bindings on a compute node:

!listing scripts/job_bitterriver_openmc language=bash caption=Sample OpenMC job script with the `moose` project code id=wnd3

## Summit

Last updated on 02/26/2024

[Summit](https://docs.olcf.ornl.gov/systems/summit_user_guide.html)
is an [!ac](HPC) system at [!ac](ORNL) with approximately
4,600 compute nodes, each of which has two IBM POWER9 processors
and six NVIDIA Tesla V100 GPUs.
Remember that in order to build Cardinal with GPU support, set the appropriate
variable in the `Makefile` to true (`1`):

```
OCCA_CUDA_ENABLED=1
OCCA_HIP_ENABLED=0
OCCA_OPENCL_ENABLED=0
```

!listing! language=bash caption=Sample `~/.bashrc` for Summit id=su1
module purge
module load DefApps-2023
module unload xl/16.1.1-10 lsf-tools/2.0 xalt/1.2.1 hsi/5.0.2.p5
module load gcc/9.1.0
module load python/3.7.0-anaconda3-5.3.0
module load hdf5/1.10.7
module load cmake/3.27.7
module load eigen/3.3.9
module load nsight-compute/2021.2.1
module load nsight-systems/2021.3.1.54
module load cuda/11.0.3

# Revise for your Cardinal repository location
DIRECTORY_WHERE_YOU_HAVE_CARDINAL=$MEMBERWORK/nfi128/summit
cd $DIRECTORY_WHERE_YOU_HAVE_CARDINAL

# This is needed because your home directory on Summit is actually a symlink
HOME_DIRECTORY_SYM_LINK=$(realpath -P $DIRECTORY_WHERE_YOU_HAVE_CARDINAL)
export NEKRS_HOME=$HOME_DIRECTORY_SYM_LINK/cardinal/install
export OCCA_DIR=$NEKRS_HOME

export HDF5_ROOT=/sw/summit/spack-envs/base/opt/linux-rhel8-ppc64le/gcc-9.1.0/hdf5-1.10.7-yxvwkhm4nhgezbl2mwzdruwoaiblt6q2
export HDF5_INCLUDE_DIR=$HDF5_ROOT/include
export HDF5_LIBDIR=$HDF5_ROOT/lib

export OPENMC_CROSS_SECTIONS=$HOME_DIRECTORY_SYM_LINK/cross_sections/endfb-vii.1-hdf5/cross_sections.xml

!listing-end!

!listing scripts/job_summit language=bash caption=Sample job script for Summit id=sum2


## Eddy

Last updated on 02/21/2024

[Eddy](https://wiki.inside.anl.gov/ne/The_Eddy_Cluster) is a cluster at
[!ac](ANL) with eleven 32-core nodes, five 40-core nodes, and six 80-core nodes.

!listing! language=bash caption=Sample `~/.bashrc` for Eddy id=e1

module purge
module load moose-dev-gcc
module load cmake/3.25.0

export HDF5_ROOT=/opt/moose/seacas
export HDF5_INCLUDE_DIR=$HDF5_ROOT/include
export HDF5_LIBDIR=$HDF5_ROOT/lib

# Revise for your Cardinal repository location
DIRECTORY_WHERE_YOU_HAVE_CARDINAL=$HOME

# This is needed because your home directory on Eddy is actually a symlink
HOME_DIRECTORY_SYM_LINK=$(realpath -P $DIRECTORY_WHERE_YOU_HAVE_CARDINAL)
export NEKRS_HOME=$HOME_DIRECTORY_SYM_LINK/cardinal/install
export OPENMC_CROSS_SECTIONS=$HOME_DIRECTORY_SYM_LINK/cross_sections/endfb-vii.1-hdf5/cross_sections.xml
!listing-end!

!listing! scripts/job_eddy language=bash caption=Sample job script for Eddy with the 32-core partition id=e2
