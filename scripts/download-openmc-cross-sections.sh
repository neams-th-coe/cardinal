#!/usr/bin/env bash
set -e

SCRIPT_DIR=$( dirname $(dirname $(cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )))
DOWNLOAD_ROOT=${1:-$(realpath $SCRIPT_DIR/..)/cross_sections}
DOWNLOAD_DIR=${DOWNLOAD_ROOT}/cross_sections

if [[ ! -d ${DOWNLOAD_DIR} ]]; then
  echo "Downloading cross sections to ${DOWNLOAD_DIR}"
  mkdir ${DOWNLOAD_DIR}
  wget -q -O - https://anl.box.com/shared/static/9igk353zpy8fn9ttvtrqgzvw1vtejoz6.xz | tar -C ${DOWNLOAD_DIR} -xJ
else
  echo "Skipping cross section download because ${DOWNLOAD_DIR} already exists."
fi

set +ex

if [[ -z "${OPENMC_CROSS_SECTIONS}" ]]; then
  echo ""
  echo "You must now set:"
  echo ""
  echo "export OPENMC_CROSS_SECTIONS=${DOWNLOAD_DIR}/endfb-vii.1-hdf5/cross_sections.xml"
  echo ""
fi
