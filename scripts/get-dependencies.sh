#!/usr/bin/env bash
set -ex

git submodule update --init contrib/moose
git submodule update --init contrib/nekRS
git submodule update --init --recursive contrib/openmc
git submodule update --init contrib/DAGMC
git submodule update --init contrib/moab
