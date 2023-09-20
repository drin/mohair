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

#   |> protobuf dependencies?
dep_proto   = dependency('protobuf')
dep_absllog = dependency('absl_log')


# ------------------------------
# Meson dependencies (modules)

# use pkg-config to generate library build info
module_pkgcfg = import('pkgconfig')


# ------------------------------
# Build variables (convenience)

# base directory
cpp_srcdir    = 'src' / 'cpp'

# dir for headers
cpp_headerdir       = cpp_srcdir / 'headers'
cpp_proto_substrait = cpp_srcdir / 'mohair' / 'substrait'
cpp_proto_mohair    = cpp_srcdir / 'mohair' / 'mohair'

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
substrait_hdrlist = [
   cpp_proto_mohair    / 'algebra.pb.h'
  ,cpp_proto_substrait / 'algebra.pb.h'
  ,cpp_proto_substrait / 'extensions' / 'extensions.pb.h'
  ,cpp_proto_substrait / 'type.pb.h'
  ,cpp_proto_substrait / 'function.pb.h'
  ,cpp_proto_substrait / 'type_expressions.pb.h'
  ,cpp_proto_substrait / 'parameterized_types.pb.h'
  ,cpp_proto_substrait / 'extended_expression.pb.h'
  ,cpp_proto_substrait / 'capabilities.pb.h'
  ,cpp_proto_substrait / 'plan.pb.h'
]

substrait_srclist = [
   cpp_proto_mohair    / 'algebra.pb.cc'
  ,cpp_proto_substrait / 'algebra.pb.cc'
  ,cpp_proto_substrait / 'extensions' / 'extensions.pb.cc'
  ,cpp_proto_substrait / 'type.pb.cc'
  ,cpp_proto_substrait / 'function.pb.cc'
  ,cpp_proto_substrait / 'type_expressions.pb.cc'
  ,cpp_proto_substrait / 'parameterized_types.pb.cc'
  ,cpp_proto_substrait / 'extended_expression.pb.cc'
  ,cpp_proto_substrait / 'capabilities.pb.cc'
  ,cpp_proto_substrait / 'plan.pb.cc'
]

mohair_hdrlist = (
    substrait_hdrlist
  + [
     cpp_headerdir / 'arrow.hpp'
    ,cpp_headerdir / 'faodel.hpp'
  ]
)

mohair_srclist = (
    substrait_srclist
  + [
     cpp_enginedir /    'faodel.cpp'
    ,cpp_enginedir /     'acero.cpp'
    ,cpp_coredir   /      'util.cpp'
    ,cpp_coredir   / 'operators.cpp'
    ,cpp_coredir   /     'plans.cpp'
  ]
)

# >> For faodel testing binary
#   |> include dirs and source list
faodel_srclist = [cpp_enginedir / 'faodel.cpp']


# ------------------------------
# Library definitions

mohair_lib = library('mohair'
  ,mohair_srclist
  ,dependencies: [ dep_ompi, dep_faodel, dep_arrow, dep_proto, dep_absllog ]
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

mohair_bin = executable('mohair'
  ,mohair_binsources
  ,dependencies       : [dep_ompi, dep_faodel, dep_arrow, dep_proto, dep_absllog ]
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
