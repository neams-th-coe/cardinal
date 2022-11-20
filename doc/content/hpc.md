# HPC Systems

This page collects
instructions for building and running Cardinal on a number of [!ac](HPC) systems.
Because default modules and
settings change on [!ac](HPC) systems with time, the instructions below may become deprecated, but
we try to keep this information up-to-date.
Note that the absence of a particular [!ac](HPC) system from this list does not imply that Cardinal will not
build/run on that system - only that documentation has not yet been created.

In addition to these provided module and environment settings,
you must follow the build instructions on the
[Getting Started](start.md) page.

!alert note
NekRS can sometimes fail to correctly pre-compile its kernels on these [!ac](HPC)
systems. We recommend precompiling NekRS (with the `nrspre` script) if you run into
issues. See the [NekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/just_in_time_compilation.html)
for more information.

## Bebop

[Bebop](https://www.lcrc.anl.gov/systems/resources/bebop/)
is an [!ac](HPC) system at [!ac](ANL) with an Intel Broadwell
partition (36 cores/node) and an Intel Knights Landing partition
(64 cores/node).

!alert note
Note that if you want to *build* Cardinal via a job script, you will also
need to `module load numactl/2.0.12-355ef36` because make can find `libnuma-dev`
on the login nodes, but you need to explicitly load it for compute nodes.

!listing! language=bash caption=Sample `~/.bashrc` for Bebop id=bb1
module purge
module load gcc/8.2.0-g7hppkz
module load openmpi/3.1.4
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
!listing-end!

!listing scripts/job_bebop language=bash caption=Sample job script for Bebop with the `startup` project code id=bb2

## Eddy

[Eddy](https://wiki.inside.anl.gov/ne/The_Eddy_Cluster) is a cluster at
[!ac](ANL) with eleven 32-core nodes, five 40-core nodes, and six 80-core nodes.

!listing! language=bash caption=Sample `~/.bashrc` for Eddy id=e1
module purge
module load moose/.mpich-3.3_gcc-9.2.0
module load miniconda moose-tools

export CC=mpicc
export CXX=mpicxx
export FC=mpif90
export F77=mpif77

export PETSC_ARCH=arch-moose
export HDF5_ROOT=/opt/moose/seacas

# Revise for your Cardinal repository location
export PETSC_DIR=$HOME/cardinal/contrib/moose/petsc
!listing-end!

!listing! language=bash caption=Sample job script for Eddy id=e3
#!/bin/bash
#PBS -k o
#PBS -l nodes=1:ppn=32
#PBS -l walltime=5:00
#PBS -M email@address.gov
#PBS -m ae
#PBS -N lattice
#PBS -j oe
#PBS -q eddy32core

module purge
module load moose/.mpich-3.3_gcc-9.2.0
module load miniconda moose-tools

# Revise for your cross section data location
export OPENMC_CROSS_SECTIONS=$HOME/cross_sections/endfb71_hdf5/cross_sections.xml

# Revise for your input file and executable locations
cd $HOME/cardinal/test/tests/neutronics/feedback/lattice
mpirun -np 1 $HOME/cardinal/cardinal-opt -i openmc_master.i --n-threads=32 > logfile
!listing-end!

!listing! language=bash caption=Sample job script to run Nek coupled to MOOSE on one node of the 32-core partition (`eddy32core`) with 32 MPI ranks id=e2
#!/bin/bash
#PBS -k o
#PBS -l nodes=1:ppn=32
#PBS -l walltime=5:00
#PBS -M email@address.gov
#PBS -m ae
#PBS -N sfr_pin
#PBS -j oe
#PBS -q eddy32core

module purge
module load moose/.mpich-3.3_gcc-9.2.0
module load miniconda moose-tools

# Revise for your Cardinal repository location
export NEKRS_HOME=$HOME/cardinal/install

# Revise for your input file and executable locations
cd $HOME/cardinal/test/tests/cht/sfr_pincell
mpirun $HOME/cardinal/cardinal-opt -i nek_master.i  > logfile
!listing-end!

## Nek5k

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

## Sawtooth

[Sawtooth](https://nsuf.inl.gov/Page/computing_resources)
 is an [!ac](HPC) system at [!ac](INL) with 99,792 cores (48 cores per node).

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
export HDF5_ROOT=/apps/moose/stack/miniconda3

export CC=mpicc
export CXX=mpicxx
export FC=mpif90

# Revise for your repository location
export NEKRS_HOME=$HOME/projects/cardinal/install
!listing-end!

!listing scripts/job_sawtooth language=bash caption=Sample job script with the `moose` project code id=st2

## Summit

[Summit](https://docs.olcf.ornl.gov/systems/summit_user_guide.html)
is an [!ac](HPC) system at [!ac](ORNL) with approximately
4,600 compute nodes, each of which has two IBM POWER9 processors
and six NVIDIA Tesla V100 GPUs.
Remember that in order to build Cardinal with GPU support, set the appropriate
variable in the `Makefile` to true (`1`):

```
OCCA_CUDA_ENABLED=0
OCCA_HIP_ENABLED=0
OCCA_OPENCL_ENABLED=0
```

!listing! language=bash caption=Sample `~/.bashrc` for Summit id=su1
module load gcc
module load cmake
module load cuda
module load hdf5
module load python/3.7.0-anaconda3-5.3.0

export HDF5_ROOT=/sw/summit/spack-envs/base/opt/linux-rhel8-ppc64le/gcc-9.1.0/hdf5-1.10.7-yxvwkhm4nhgezbl2mwzdruwoaiblt6q2
export HDF5_INCLUDE_DIR=$HDF5_ROOT/include
export HDF5_LIBDIR=$HDF5_ROOT/lib

# Revise for your Cardinal repository location
DIRECTORY_WHERE_YOU_HAVE_CARDINAL=$HOME

# This is needed because your home directory on Summit is actually a symlink
HOME_DIRECTORY_SYM_LINK=$(realpath -P $DIRECTORY_WHERE_YOU_HAVE_CARDINAL)
export NEKRS_HOME=$HOME_DIRECTORY_SYM_LINK/cardinal/install
!listing-end!

!listing scripts/job_summit language=bash caption=Sample job script for Summit id=sum2
