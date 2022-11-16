#!/usr/bin/env bash
set -ex

if [[ ! -e ../test_suite_cross_sections ]]; then
  mkdir ../test_suite_cross_sections
  echo $PWD
  wget -q -O - https://anl.box.com/shared/static/p1b26vfjrwkxmuezy93ohf5tbah9scft.xz | tar -C ../test_suite_cross_sections -xJ
fi
