# ------------------------------
# Overview


# ------------------------------
# Dependencies

# >> Arrow and Flight (RPC via protobuf)
dataservice_dep = [
   dependency('arrow')
  ,dependency('arrow-flight')
]

# >> Protobuf, Substrait, and Mohair
protocol_dep = [
   dependency('absl_status'               , static: true)
  ,dependency('absl_log_internal_check_op', static: true)
  ,dependency('absl_log_internal_message' , static: true)
  ,dependency('protobuf'                  , static: true)
  ,dependency('arrow-substrait'           , static: false)
  ,dependency('mohair'                    , static: false)
]

# >> Execution engines (many are optional)
engine_deps = []

# add optional dependencies to dependency groups
if libtiledb_dep.found()
  engine_deps += [libtiledb_dep]
endif

if libduckdb_dep.found()
  engine_deps += [libduckdb_dep]
endif


# ------------------------------
# Headers and Sources

# >> Headers
libskytether_headers = [
   'skytether.hpp'
  ,'skytether_macros.hpp'
  ,'skytether_codes.hpp'
]

# >> Sources
libskytether_sources = [ 'util.cpp' ]


# >> From subdirectories
subdir('skytether')
subdir('services')


# ------------------------------
# Build targets

# >> Include targets
libskytether_inc = [
   config_inc
  ,include_directories('.')
]


# >> Library targets
libskytether = library('skytether'
  ,libskytether_sources
  ,include_directories: [ libskytether_inc ]
  ,dependencies       : [ dataservice_dep, protocol_dep, engine_deps ]
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,pic                : true
  ,install            : true
)


# >> Dependency targets
libskytether_dep = declare_dependency(
   include_directories: [ libskytether_inc ]
  ,dependencies       : [ dataservice_dep, protocol_dep, engine_deps ]
  ,link_with          : libskytether
)


# ------------------------------
# Installation

# >> Headers
install_headers(libskytether_headers, subdir: 'skytether', preserve_path: true)


# >> Package configurations
# use pkg-config to generate library build info
module_pkgcfg = import('pkgconfig')
module_pkgcfg.generate(libskytether, subdirs: 'skytether')
