#!/bin/bash

# Usage:
# qsub job_sawtooth

#PBS -l select=1:ncpus=48:mpiprocs=24:ompthreads=2
#PBS -l walltime=5:00
#PBS -m ae
#PBS -N cardinal
#PBS -j oe
#PBS -P moose

module purge
module load openmpi/3.1.6-gcc-10.4.0-psnb
module load cmake/3.22.3-gcc-9.2.0-5mqh

# Revise for your repository location
export CARDINAL_DIR=$HOME/cardinal

# Run an OpenMC case
cd $CARDINAL_DIR/test/tests/neutronics/feedback/lattice
rm logfile
mpirun $CARDINAL_DIR/cardinal-opt -i openmc_master.i --n-threads=2 > logfile

# Run a NekRS case
cd $CARDINAL_DIR/test/tests/cht/sfr_pincell
rm logfile
mpirun $CARDINAL_DIR/cardinal-opt -i nek_master.i > logfile
