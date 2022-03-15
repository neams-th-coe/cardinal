#!/usr/bin/env bash
set -ex

git submodule update --init contrib/sockeye
git submodule update --init contrib/sodium
git submodule update --init contrib/potassium
git submodule update --init contrib/iapws95
