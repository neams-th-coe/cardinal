#!/usr/bin/env bash
SCRIPT_DIR=$( dirname $(cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd ))

set -ex

git submodule update --init contrib/moose
git submodule update --init contrib/nekRS
git submodule update --init --recursive contrib/openmc
git submodule update --init contrib/DAGMC
git submodule update --init contrib/moab
git submodule update --init test/tests/nek_ci

set +ex

if [[ -z "${OPENMC_CROSS_SECTIONS}" ]]; then
  echo ""
  echo -e "\e[31mIf you are using OpenMC, remember that you need to set OPENMC_CROSS_SECTIONS to point to a cross_sections.xml file! To get the ENDF/b-7.1 dataset, please run:\e[0m"
  echo ""
  echo "$SCRIPT_DIR/scripts/download-openmc-cross-sections.sh"
  echo ""
fi

if [[ -z "${NEKRS_HOME}" ]]; then
  echo ""
  echo -e "\e[31mIf you are using NekRS, remember that you need to set NEKRS_HOME to point to your NekRS install! If you are using the NekRS submodule, please set:\e[0m"
  echo ""
  echo "export NEKRS_HOME=$SCRIPT_DIR/install"
  echo ""
fi

if [[ -z "${LIBMESH_JOBS}" ]]; then
  echo ""
  echo -e "\e[32mTo save time and build in parallel, you can build libMesh in parallel with 8 cores (just an example) by setting:\e[0m"
  echo ""
  echo "export LIBMESH_JOBS=8"
  echo ""
fi
