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
  ,dependency('protobuf'                  , static: true)
  ,dependency('mohair-substrait'          , static: false)
  ,dependency('arrow-substrait'           , static: false)
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
# Reference paths

# dir for core mohair sources
query_srcdir   = 'query'
engine_srcdir  = 'engines'
service_srcdir = 'services'


# ------------------------------
# Headers and Sources

# >> Headers
query_headers = [
   query_srcdir / 'apidep_substrait.hpp'
  ,query_srcdir / 'operators.hpp'
  ,query_srcdir / 'plans.hpp'
]

engine_headers = [
   engine_srcdir  / 'adapter_acero.hpp'
  ,engine_srcdir  / 'adapter_duckdb.hpp'
]

service_headers = [
   service_srcdir / 'apidep_flight.hpp'
  ,service_srcdir / 'ticket_mohair.hpp'
  ,service_srcdir / 'service_mohair.hpp'
  ,service_srcdir / 'service_topology.hpp'
  ,service_srcdir / 'service_duckdb.hpp'
]


# >> Sources
query_sources = [
   query_srcdir / 'operators.cpp'
  ,query_srcdir / 'plans.cpp'
]

# >> For flight services
service_sources = [
   service_srcdir / 'adapter_flight.cpp'
  ,service_srcdir / 'ticket_mohair.cpp'
  ,service_srcdir / 'service_mohair.cpp'
  ,service_srcdir / 'service_topology.cpp'
  ,service_srcdir / 'service_duckdb.cpp'
  ,service_srcdir / 'client_mohair.cpp'
  ,engine_srcdir  / 'execution.cpp'
  ,engine_srcdir  / 'duckdb.cpp'
]


# >> Composed header and source lists

# headers
libskytether_mohair_headers = (
    [
       'apidep_standard.hpp'
      ,'apidep_arrow.hpp'
      ,'mohair.hpp'
      ,'mohair_macros.hpp'
      ,'mohair_codes.hpp'
    ]
  + query_headers
  + engine_headers
  + service_headers
)

# sources
libskytether_mohair_sources = (
    [ 'util.cpp' ]
  + query_sources
  + service_sources
)


# ------------------------------
# Include directories

libskytether_mohair_inc += [include_directories('.')]


# ------------------------------
# Build targets

# >> Library targets

libskytether_mohair = library('skytether-mohair'
  ,libskytether_mohair_sources
  ,include_directories: [ libskytether_mohair_inc ]
  ,dependencies       : [ dataservice_dep, protocol_dep, engine_deps ]
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,pic                : true
  ,install            : true
)


# ------------------------------
# Declared dependencies

libskytether_mohair_dep = declare_dependency(
   include_directories: [ libskytether_mohair_inc ]
  ,dependencies       : [ dataservice_dep, protocol_dep, engine_deps ]
  ,link_with          : libskytether_mohair
)


# ------------------------------
# Installation

install_headers(
   libskytether_mohair_headers
  ,subdir       : 'skytether-mohair'
  ,preserve_path: true
)


# ------------------------------
# Package configurations

# use pkg-config to generate library build info
module_pkgcfg = import('pkgconfig')
module_pkgcfg.generate(libskytether_mohair, subdirs: 'skytether-mohair')
