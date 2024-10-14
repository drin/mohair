# ------------------------------
# Overview


# ------------------------------
# Dependencies

# >> Arrow
libarrow_dep           = dependency('arrow')
libarrow_substrait_dep = dependency('arrow-substrait')
libarrow_flight_dep    = dependency('arrow-flight')

# >> Substrait
libmohair_substrait_dep = dependency('mohair-substrait', static: true)

libprotobuf_dep = dependency('protobuf')
libabsl_dep     = dependency('absl'
  ,modules: [
      'absl::log_internal_check_op'
     ,'absl::status'
   ]
)

# >> Grouped dependencies
query_group_dep = [
   libarrow_dep
  ,libarrow_substrait_dep
  ,libabsl_dep
  ,libprotobuf_dep
  ,libmohair_substrait_dep
]

service_group_dep = query_group_dep + [libarrow_flight_dep]

# add optional dependencies to dependency groups
if libfaodel_dep.found()
  service_group_dep += [libompi_cxx_dep, libfaodel_dep]

elif libompi_cxx_dep.found()
  service_group_dep += [libompi_cxx_dep]

endif

if libtiledb_dep.found()
  service_group_dep += [libtiledb_dep]
endif

if libduckdb_dep.found()
  service_group_dep += [libduckdb_dep]
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
  ,query_srcdir / 'messages.hpp'
  ,query_srcdir / 'operators.hpp'
  ,query_srcdir / 'plans.hpp'
]

engine_headers = [
   engine_srcdir  / 'adapter_acero.hpp'
  ,engine_srcdir  / 'adapter_duckdb.hpp'
   #,engine_srcdir  / 'adapter_faodel.hpp'
   #,engine_srcdir  / 'adapter_mpi.hpp'
]

service_headers = [
   service_srcdir / 'apidep_flight.hpp'
  ,service_srcdir / 'service_mohair.hpp'
  ,service_srcdir / 'service_topology.hpp'
  ,service_srcdir / 'service_duckdb.hpp'
  #,service_srcdir / 'service_faodel.hpp'
]


# >> Sources
query_sources = [
   query_srcdir / 'messages.cpp'
  ,query_srcdir / 'operators.cpp'
  ,query_srcdir / 'plans.cpp'
]

# >> For flight services
service_sources = [
   service_srcdir / 'adapter_flight.cpp'
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

libskytether_mohair_inc = include_directories('.')


# ------------------------------
# Build artifacts (libraries)

libskytether_mohair = library('skytether-mohair'
  ,libskytether_mohair_sources
  ,include_directories: configuration_inc
  ,dependencies       : service_group_dep
  ,pic                : true
  ,install            : true
)

libskytether_mohair_dep = declare_dependency(
   include_directories: configuration_inc
  ,dependencies       : service_group_dep
  ,link_with          : libskytether_mohair
)

# ------------------------------
# Installation

install_headers(
   libskytether_mohair_headers
  ,subdir       : 'skytether-mohair'
  ,preserve_path: true
)

