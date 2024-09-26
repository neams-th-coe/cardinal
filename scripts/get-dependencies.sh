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
  echo "If you are using OpenMC, remember that you need to set OPENMC_CROSS_SECTIONS to point to a cross_sections.xml file!"
  echo "To get the ENDF/b-7.1 dataset, please run:"
  echo ""
  echo "$SCRIPT_DIR/scripts/download-openmc-cross-sections.sh"
  echo ""
fi

if [[ -z "${NEKRS_HOME}" ]]; then
  echo ""
  echo "If you are using NekRS, remember that you need to set NEKRS_HOME to point to your NekRS install!"
  echo "If you are using the NekRS submodule, please set:"
  echo ""
  echo "export NEKRS_HOME=$SCRIPT_DIR/install"
  echo ""
fi
