#!/usr/bin/env bash
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DOWNLOAD_DIR=${1:-$(realpath $SCRIPT_DIR/../..)/cross_sections}
XS_VERSION=endfb-vii.1-hdf5
XS_DIR=${DOWNLOAD_DIR}/${XS_VERSION}

if [[ ! -d ${XS_DIR} ]]; then
  mkdir -p ${XS_DIR}
  echo "Downloading cross sections to ${XS_DIR}"
  wget -q -O - https://anl.box.com/shared/static/9igk353zpy8fn9ttvtrqgzvw1vtejoz6.xz | tar -C ${DOWNLOAD_DIR} -xJ --no-same-owner
else
  echo "Skipping cross section download because ${XS_DIR} already exists."
fi

set +ex

if [[ -z "${OPENMC_CROSS_SECTIONS}" ]]; then
  echo ""
  echo "You must now set:"
  echo ""
  echo "export OPENMC_CROSS_SECTIONS=${XS_DIR}/cross_sections.xml"
  echo ""
fi
