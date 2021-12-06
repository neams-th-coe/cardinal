#!/usr/bin/env bash
set -ex

if [[ ! -e ../cross_sections ]]; then
  mkdir ../cross_sections
  echo $PWD
  wget -q -O - https://anl.box.com/shared/static/9igk353zpy8fn9ttvtrqgzvw1vtejoz6.xz | tar -C ../cross_sections -xJ
fi
