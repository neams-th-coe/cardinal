#!/bin/bash

# Usage:
# 1. Update any needed environment variables and input file names in this script
# 2. qsub job_eddy

#PBS -k o
#PBS -l nodes=1:ppn=32
#PBS -l walltime=10:00
#PBS -m ae
#PBS -N lattice
#PBS -j oe
#PBS -q eddy32core

module purge
module load moose-dev-gcc
module load cmake/3.25.0

# Revise for your Cardinal repository location
DIRECTORY_WHERE_YOU_HAVE_CARDINAL=$HOME

# This is needed because your home directory on Eddy is actually a symlink
HOME_DIRECTORY_SYM_LINK=$(realpath -P $DIRECTORY_WHERE_YOU_HAVE_CARDINAL)
export CARDINAL_DIR=$HOME_DIRECTORY_SYM_LINK/cardinal

# The name of the input file you want to run
input_file=nek_master.i

# Revise for your input file location; you do actually need to have the 'cd ...' line
# in this script, you can't just place this job script in the same directory as your files
cd $CARDINAL_DIR/test/tests/cht/sfr_pincell
mpirun -np 32 $CARDINAL_DIR/cardinal-opt -i $input_file > logfile
