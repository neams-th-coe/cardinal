#/bin/bash

project_id=nfi114
NEKRS_HYPRE_NUM_THREADS=1

: ${CPUONLY:=0}
#export OCCA_CACHE_DIR=/gpfs/alpine/proj-shared/${project_id}/${USER}/.occa
export OCCA_CACHE_DIR=.cache
export NEKRS_HYPRE_NUM_THREADS

export OCCA_CXX="/sw/summit/xl/16.1.1-3/xlC/16.1.1/bin/xlc" 
export OCCA_CXXFLAGS="-O3 -qarch=pwr9 -qhot -DUSE_OCCA_MEM_BYTE_ALIGN=64" 
export OCCA_LDFLAGS="/sw/summit/xl/16.1.1-3/xlC/16.1.1/lib/libibmc++.a"

#export OMPI_LD_PRELOAD_POSTPEND=$OLCF_SPECTRUM_MPI_ROOT/lib/libmpitrace.so

if [ $# -ne 3 ]; then
  echo "usage: [CPUONLY=1] $0 <casename> <number of nodes> <hh:mm>"
  exit 0
fi

module load gcc

bin=../../cardinal-opt
case=$1
nodes=$2
gpu_per_node=6
cores_per_socket=21
let nn=$nodes*$gpu_per_node
let ntasks=nn
time=$3

if [ $CPUONLY -eq 1 ]; then
  let nn=2*$nodes
  let ntasks=$nn*$cores_per_socket
  export NEKRS_BACKEND="CPU"
fi 

if [ ! -f $bin ]; then
  echo "Cannot find" $bin
  exit 1
fi

if [ ! -f $case.par ]; then
  echo "Cannot find" $case.par
  exit 1
fi

if [ ! -f $case.co2 ]; then
  echo "Cannot find" $case.co2
  exit 1
fi

if [ ! -f $case.udf ]; then
  echo "Cannot find" $case.udf
  exit 1
fi

if [ ! -f $case.oudf ]; then
  echo "Cannot find" $case.oudf
  exit 1
fi

if [ ! -f $case.re2 ]; then
  echo "Cannot find" $case.re2
  exit 1
fi

mkdir -p $OCCA_CACHE_DIR 2>/dev/null

while true; do
  read -p "Do you want run nrspre? [N]" yn
  case $yn in
    [Yy]* )
      mpirun -pami_noib -np 1 $NEKRS_HOME/bin/nekrs --setup $case --build-only $ntasks;
      if [ $? -ne 0 ]; then
        exit 1
      fi
      break ;;
    * )
      break ;;
  esac
done

if [ $CPUONLY -eq 1 ]; then
  cmd="bsub -nnodes $nodes -W $time -P $project_id -J cardinal_$case jsrun -X 1 -n$nn -a$cores_per_socket -c$cores_per_socket -g0 -b packed:1 -d packed $bin --app nek -i nek.i --nekrs-setup $case"
else
  cmd="bsub -nnodes $nodes -W $time -P $project_id -J cardinal_$case jsrun -X 1 -n$nn -r$gpu_per_node -a1 -c2 -g1 -b packed:1 -d packed $bin --app nek -i nek.i --nekrs-setup $case"
fi
echo $cmd
echo $CPUONLY > qq
echo $cmd >> qq

$cmd
