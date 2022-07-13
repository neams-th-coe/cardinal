# HPC Systems

The Cardinal development team regularly builds and runs Cardinal on a number
of [!ac](HPC) systems. To help streamline the compilation
process for other users, this section collects instructions for the modules and any
systems-specific settings for a variety of [!ac](HPC) systems. Because default modules and
settings change on [!ac](HPC) systems with time, the instructions below may become deprecated.
The date at which the settings were last tested is given for each system.
Note that the absence of a particular [!ac](HPC) system from this list does not imply that Cardinal will not
build/run on that system - only that no instructions have yet been documented for it.

Also please note that in addition to these provided module and environment settings,
you must also follow the build instructions on the
[Getting Started](start.md) page. Two environment variables that are important to set
for your particular setup are:

- `NEKRS_HOME`: should point to NekRS install location, or `cardinal/install`; this
   is only needed for inputs that run NekRS
- `OPENMC_CROSS_SECTIONS`: should point to the location of `cross_sections.xml`; this
   is only needed for inputs that run OpenMC

!alert note
NekRS can sometimes fail to correctly pre-compile its kernels on these [!ac](HPC)
systems. We recommend precompiling NekRS (with the `nrspre` script) if you run into
issues. See the [NekRS documentation](https://nekrsdoc.readthedocs.io/en/latest/just_in_time_compilation.html)
for more information.

## Bebop

[Bebop](https://www.lcrc.anl.gov/systems/resources/bebop/)
is an [!ac](HPC) system at [!ac](ANL) with 1,024 nodes with an Intel Broadwell
partition with 36 cores/node and a Intel Knights Landing partition with
64 cores/node. Below are a bash script and sample job scripts to build
Cardinal and run the NekRS and OpenMC wrappings (*last updated 05/2022*).

!listing! language=bash caption=`~/.bashrc` to compile Cardinal id=bb1
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

!listing scripts/job_bebop language=bash caption=Job script to run OpenMC and Nek cases on one node of the 36-core partition with the `startup` project code id=bb2

## Eddy

[Eddy](https://wiki.inside.anl.gov/ne/The_Eddy_Cluster) is a cluster at
[!ac](ANL) with eleven 32-core nodes, five 40-core nodes, and six 80-core nodes. Below are
a bash script and sample job scripts to build Cardinal and run the NekRS and OpenMC
wrappings (*last updated 10/11/2021*)

!listing! language=bash caption=`~/.bashrc` to compile Cardinal id=e1
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

!listing! language=bash caption=Sample job script to run OpenMC coupled to MOOSE on one node of the 32-core partition with 32 OpenMP threads id=e3
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

## KOOKIE Cluster

The KOOKIE cluster at [!ac](ANL) (also called the VTR cluster) has 12
nodes with a variety of different CPUs and GPUs for each node.
Below is
a bash script to build Cardinal (*last updated 5/16/2022*)

!listing! language=bash caption=`~/.bashrc` to compile Cardinal id=k1
export CC=mpicc
export CXX=mpicxx
export FC=mpif90
export F77=mpif77
export F90=mpif90

module purge
module load advanced_modules
module load mpich-gcc

# Revise for your Cardinal repository location
export NEKRS_HOME=$HOME/cardinal/install

!listing-end!

## Nek5k

Nek5k is a cluster at [!ac](ANL) with 40 nodes, each with 40 cores.
We use conda to set up a proper environment on Nek5k for running Cardinal.
To use this environment, you will need to follow these steps *the first time*
you use Nek5k:

- The first time you log in, run from the command line:

```
$ module load openmpi/4.0.1/gcc/8.3.1 cmake openmpi/4.0.1/gcc/8.3.1-hdf5-1.10.6 anaconda3/anaconda3
$ conda init
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

!listing! language=bash caption=`~/.bashrc` to compile Cardinal id=n1
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
!listing-end!

Below are sample job scripts to run the NekRS and OpenMC
wrappings (*last updated 10/20/2021*).

!listing! language=bash caption=Sample job script to run OpenMC coupled to MOOSE on one node with 40 OpenMP threads id=n2
#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=40
#SBATCH --time=00:20:00
#SBATCH --output=pincell.log
#SBATCH -p compute

# Revise for your cross section data location
export OPENMC_CROSS_SECTIONS=$HOME/cross_sections/endfb71_hdf5/cross_sections.xml

# Revise for your input file and executable locations
cd $HOME/cardinal/test/tests/neutronics/feedback/lattice
mpirun -np 1 $HOME/cardinal/cardinal-opt -i openmc_master.i --n-threads=40 > logfile
!listing-end!

!listing! language=bash caption=Sample job script to run NekRS coupled to MOOSE on one node with 40 MPI processes id=n3
#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=40
#SBATCH --time=00:20:00
#SBATCH --output=sfr.log
#SBATCH -p compute

# Revise for your input file and executable locations
cd $HOME/cardinal/test/tests/cht/sfr_pincell
mpirun -np 40 $HOME/cardinal/cardinal-opt -i nek_master.i > logfile
!listing-end!

## Sawtooth

[Sawtooth](https://nsuf.inl.gov/Page/computing_resources)
 is an [!ac](HPC) system at [!ac](INL) with 99,792 cores. Each compute node contains
dual Xeon Platinum 8268 processors with 24 cores each, giving 48 cores per node. 27 nodes have
four NVIDIA V100 GPUs each. Below are a bash script and sample job scripts to build Cardinal and
run the NekRS and OpenMC wrappings (*last updated 06/14/2022*).

!listing! language=bash caption=`~/.bashrc` to compile Cardinal id=st1
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

!listing scripts/job_sawtooth language=bash caption=Job script to run OpenMC and Nek cases on one node with the `moose` project code id=st2

## Summit

[Summit](https://docs.olcf.ornl.gov/systems/summit_user_guide.html)
is an [!ac](HPC) system at [!ac](ORNL) with approximately
4,600 compute nodes, each of which has two IBM POWER9 processors
and six NVIDIA Tesla V100 GPUs. Below is a bash script to build
Cardinal on Summit (*last updated 8/01/2021*).

!listing! language=bash caption=`~/.bashrc` to compile Cardinal id=su1
module load gcc
module load cmake
module load cuda
module load hdf5
module load python/3.7.0-anaconda3-5.3.0
!listing-end!

Remember that in order to build Cardinal with GPU support, set the appropriate
variable in the `Makefile` to true (`1`):

!listing cardinal/Makefile
  start=OCCA_CUDA_ENABLED
  end=NEKRS_BUILDDIR
