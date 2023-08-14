#!/usr/bin/bash


# ------------------------------
# Overview

# This script is meant to be run from the repository root, where there should be two files that
# define builds: `meson.build` for C++ and `setup.py` for python/cython. Unlike
# `scripts/bootstrap-build.bash`, this script should still work regardless of the directory it is
# executed from.

# >> **IMPORTANT** <<
# This script absolutely must be ran in a context with the appropriate python interpreter.
# Specifically, the virtualenv that is to be used for this repository. As an example, this code was
# developed using poetry for python dependency management. So, using poetry we would run `poetry
# shell` and *then* execute this script, or we would run `poetry run bash
# scripts/recompile-pyarrow.bash` which would execute this script witht he appropriate python
# virtualenv activated.

# This script is located in the `scripts/` directory to keep the root directory relatively clean.


# ------------------------------
# Global variables

# NOTE: this should be a parameter
arrow_prefix="/usr/local/arrow-12.0.1"
arrow_version=">=12.0.0"

# where Ubuntu installs it
# default: arrow_prefix=/usr/local
arrow_python_dirpath="${arrow_prefix}/lib/cmake/arrow"

# where ArchLinux installs it
# default: arrow_prefix=/usr
if [[ -f "/etc/os-release" && -n $(grep "Arch Linux" "/etc/os-release") ]]; then
    arrow_python_dirpath="${arrow_prefix}/lib/cmake/arrow"
fi

# export PYARROW_CMAKE_OPTIONS="-DArrowPython_DIR=${arrow_python_dirpath}"


# ------------------------------
# Install logic

# uninstall existing binary
python3 -m pip uninstall pyarrow

# so that we can install a re-compiled binary
python3 -m pip install --no-cache-dir --no-binary :all: "pyarrow${arrow_version}"
