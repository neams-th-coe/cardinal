#!/usr/bin/env bash
SCRIPT_DIR=$( dirname $(dirname $(cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )))

if [[ ! -e $SCRIPT_DIR/cross_sections/endfb-vii.1-hdf5 ]]; then
  mkdir ../cross_sections
  echo $PWD
  wget -q -O - https://anl.box.com/shared/static/9igk353zpy8fn9ttvtrqgzvw1vtejoz6.xz | tar -C ../cross_sections -xJ
else
  echo "Skipping cross section download because $SCRIPT_DIR/cross_sections/endfb-vii.1-hdf5 already exists."
fi

set +ex

if [[ -z "${OPENMC_CROSS_SECTIONS}" ]]; then
  echo ""
  echo "You must now set:"
  echo ""
  echo "export OPENMC_CROSS_SECTIONS=$SCRIPT_DIR/cross_sections/endfb-vii.1-hdf5/cross_sections.xml"
  echo ""
fi
