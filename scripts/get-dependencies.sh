#!/usr/bin/env bash
set -ex

git submodule update --init contrib/moose
git submodule update --init contrib/nekRS
git submodule update --init --recursive contrib/openmc
