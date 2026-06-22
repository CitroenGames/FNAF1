#!/usr/bin/env bash
set -euo pipefail

sighmake Fnaf1.buildscript -g makefile
sighmake --build . --config Release --parallel "$(sysctl -n hw.ncpu)"
