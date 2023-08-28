# ------------------------------
# Project Definition

# >> configuration
project('mohair-faodel'
  ,'cpp'
  ,version         : '0.1'
  ,default_options : ['warning_level=3','cpp_std=c++17']
)


# >> dependencies
dep_ompi   = dependency('ompi-cxx')
dep_faodel = dependency('faodel')
dep_arrow  = dependency('arrow')


# ------------------------------
# Meson dependencies (modules)

# use pkg-config to generate library build info
module_pkgcfg = import('pkgconfig')


# ------------------------------
# Build variables (convenience)

# base directory
cpp_srcdir    = 'src' / 'cpp'

# dir for headers
cpp_headerdir = cpp_srcdir / 'headers'

# dir for core mohair sources
cpp_coredir   = cpp_srcdir / 'mohair'

# dir for adapters (execution engines)
cpp_enginedir = cpp_srcdir / 'engines'

# dir for binary sources (1-1 with binaries)
cpp_tooldir   = cpp_srcdir / 'toolbox'


# ------------------------------
# Headers and Sources

# >> For primary mohair binary
#   |> include dirs and source list
mohair_hdrlist = [cpp_headerdir / 'arrow.hpp' , cpp_headerdir / 'faodel.hpp']
mohair_srclist = [
   cpp_enginedir /    'faodel.cpp'
  ,cpp_enginedir /     'acero.cpp'
  ,cpp_coredir   /      'util.cpp'
  ,cpp_coredir   / 'operators.cpp'
  ,cpp_coredir   /     'plans.cpp'
]

# >> For faodel testing binary
#   |> include dirs and source list
faodel_srclist = [cpp_enginedir / 'faodel.cpp']


# ------------------------------
# Library definitions

mohair_lib = library('mohair'
  ,mohair_srclist
  ,dependencies: [ dep_ompi, dep_faodel, dep_arrow ]
  ,include_directories: cpp_headerdir
  ,install            : true
)

module_pkgcfg.generate(mohair_lib)

# ------------------------------
# Installation

install_headers(mohair_hdrlist, subdir: 'mohair')


# ------------------------------
# Binaries

# >> Executable for primary mohair binary
mohair_binsources = (
    [ cpp_tooldir / 'mohair.cpp' ]
  + mohair_srclist
)

test_faodel = executable('mohair'
  ,mohair_binsources
  ,dependencies       : [dep_ompi, dep_faodel, dep_arrow]
  ,include_directories: cpp_headerdir
  ,install            : false
)

# >> Executable for faodel testing binary
testfaodel_binsources = (
    [ cpp_tooldir / 'test-faodel.cpp' ]
  + faodel_srclist
)

test_faodel = executable('test-faodel'
  ,testfaodel_binsources
  ,dependencies       : [dep_ompi, dep_faodel, dep_arrow]
  ,include_directories: cpp_headerdir
  ,install            : false
)
