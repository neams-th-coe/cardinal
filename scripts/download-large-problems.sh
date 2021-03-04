#!/usr/bin/env bash

if [ ! -e problems/large-problems ]; then
  wget -O - https://xgitlab.cels.anl.gov/neams-th-coe/large-problems/-/archive/master/large-problems-master.tar.gz | tar -C problems -xz
fi
