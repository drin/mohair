#!/usr/bin/env python

# ------------------------------
# License

# Copyright 2023 Aldrin Montana
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# ------------------------------
# Module Docstring
"""
Build script for the mohair module.

This includes how to compile cython bindings for prototyping integration with Faodel.
"""


# ------------------------------
# Dependencies

# >> Modules
from pathlib import Path

# >> Third-party
import pkgconfig
import numpy
import pyarrow

# >> Classes
from setuptools import Extension

# >> Functions
from setuptools import setup
from Cython.Build import cythonize


# ------------------------------
# Module variables

# >> reference variables
cppsrc_dirpath = Path('src') / 'cpp'
pysrc_dirpath  = Path('src') / 'python'

# TODO: these will eventually be migrated into skytether components
#       and mohair will be more of a leaf dependency
pkgsrc_dirpath  = pysrc_dirpath / 'mohair' / 'cybindings'


# ------------------------------
# Extension definitions

# >> Mohair bindings to faodel and acero wrappers

#   |> Resources for the extension
pkgsrc_filelist = [
     pkgsrc_dirpath /             'scytether.pyx'
    ,cppsrc_dirpath / 'engines' /    'faodel.cpp'
    ,cppsrc_dirpath / 'engines' /     'acero.cpp'
]

pkgdep_inclist = [
     numpy.get_include()
    ,pyarrow.get_include()
    ,pkgconfig.variables('arrow-acero')['includedir']
    ,pkgconfig.variables('mohair')['includedir']
    ,f"{pkgconfig.variables('faodel')['includedir']}/faodel"
]

pkgdep_liblist = [
     'arrow'
    ,'arrow_acero'
    ,'arrow_substrait'
    ,'arrow_python'
    ,'mpi'
    ,'mpi_cxx'
    ,'faodel-common'
    ,'faodel-services'
    ,'dirman'
    ,'lunasa'
    ,'kelpie'
]

pkgdep_libdirs = (
      pyarrow.get_library_dirs()
    + [
           '/usr/lib'
          ,'/usr/local/faodel-dev/lib'
      ]
)

#    |> The extension itself
scytether_extension = Extension('scytether'
    ,list(map(str, pkgsrc_filelist))
    ,include_dirs=pkgdep_inclist
    ,library_dirs=pkgdep_libdirs
    ,libraries=pkgdep_liblist
)


# ------------------------------
# Main build logic

# >> use README.md for long description
with open('README.md', mode='r', encoding='utf-8') as readme_handle:
    readme_content   = readme_handle.read()


# >> simple variables to capture cython configuration
cythonize_modulelist = [ scytether_extension ]
cythonize_compdirs   = { 'language_level': 3 }

setup(
     name             = 'mohair'
    ,version          = '0.1.0'
    ,zip_safe         = False
    ,author           = 'Aldrin Montana'
    ,author_email     = 'octalene.dev@pm.me'
    ,description      = ('Query processing for decomposable queries')
    ,license          = 'MPL 2.0'
    ,keywords         = 'example documentation tutorial'
    ,url              = 'https://github.com/drin/mohair-faodel/'
    ,long_description = readme_content

    ,package_dir      = { '': str(pysrc_dirpath) }
    ,packages         = [
          'mohair'
         ,'mohair.query'
         ,'mohair.storage'
         ,'mohair.mohair'
         ,'mohair.substrait'
         ,'mohair.substrait.extensions'
     ]
    ,ext_modules      = cythonize(
          module_list=cythonize_modulelist
         ,compiler_directives=cythonize_compdirs
     )

    ,classifiers      = [
          'Development Status :: 1 - Planning'
         ,'Topic :: Utilities'
         ,'License :: OSI Approved :: Apache Software License'
     ]
)
