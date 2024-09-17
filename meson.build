# ------------------------------
# Project Definition

# >> configuration
project('libmohair'
  ,'cpp'
  ,version         : '0.1.0'
  ,default_options : ['warning_level=3','cpp_std=c++17']
)


# ------------------------------
# Build variables (convenience)

# base directory
cpp_srcdir    = 'src' / 'cpp'

# dir for core mohair sources
cpp_querydir  = cpp_srcdir / 'query'

# dir for generated protobuf code
cpp_proto_substrait = cpp_srcdir / 'query' / 'substrait'
cpp_proto_mohair    = cpp_srcdir / 'query' / 'mohair'

# dir for adapters (execution engines)
cpp_enginedir  = cpp_srcdir / 'engines'

# dir for flight services
cpp_servicedir = cpp_srcdir / 'services'

# dir for binary sources (1-1 with binaries)
cpp_tooldir    = cpp_srcdir / 'toolbox'


# ------------------------------
# Meson dependencies (modules)

# use pkg-config to generate library build info
module_pkgcfg = import('pkgconfig')

# >> Determine the system we're building on
buildsys_type = 'unknown'

os_name = run_command('uname', '-s', check: false).stdout().strip()
if os_name == 'Darwin'
    buildsys_type = 'macosx'

elif os_name == 'Linux'
    os_is_arch   = run_command(
      'grep', '--silent', 'NAME="Arch Linux"', '/etc/os-release', check: false
    )
    os_is_ubuntu = run_command(
      'grep', '--silent', 'NAME="Ubuntu"'    , '/etc/os-release', check: false
    )

    if os_is_arch.returncode() == 0
        buildsys_type = 'archlinux'

    elif os_is_ubuntu.returncode() == 0
        buildsys_type = 'ubuntu'

    endif

endif


# ------------------------------
# Library dependencies

default_includedir = get_option('prefix') / get_option('includedir')

# >> System-dependent dependencies

# Include directory for arrow headers
arrow_incdir = default_includedir

# Include directory for substrait and mohair headers
protocol_incdir = default_includedir / 'mohair-substrait'


# >> Required dependencies

#   |> Arrow
dep_arrow  = dependency('arrow')
dep_acero  = dependency('arrow-substrait') # this is Acero + substrait
dep_flight = dependency('arrow-flight')

#   |> Protobuf (for substrait)
dep_mohairsubstrait = dependency('mohair-substrait', static: true)

dep_proto  = dependency('protobuf')
dep_absl = dependency('absl'
  ,modules: [
      'absl::log_internal_check_op'
     ,'absl::status'
   ]
)


# >> Optional dependencies
#   |> Faodel
dep_ompi   = dependency('ompi-cxx', required: get_option('mpi'))
dep_faodel = dependency('faodel'  , required: get_option('faodel'))
dep_tiledb = dependency('tiledb'  , required: get_option('tiledb'))

dep_duckdb = dependency('duckdb'
  ,method: 'cmake'
  ,required: get_option('duckdb')
)


# >> Make configuration data available to source files
version_str    = meson.project_version()
version_fields = version_str.split('.')

is_debug       = get_option('buildtype') == 'debug' ? 1 : 0

mohair_cfgdata = configuration_data({
   'VERSION_STR'   : meson.project_version()
  ,'VERSION_MAJOR' : version_fields[0]
  ,'VERSION_MINOR' : version_fields[1]
  ,'VERSION_PATCH' : version_fields[2]
  ,'BUILD_IS_DEBUG': is_debug
  ,'MPI'           : dep_ompi.found()
  ,'FAODEL'        : dep_faodel.found()
  ,'TILEDB'        : dep_tiledb.found()
  ,'DUCKDB'        : dep_duckdb.found() ? 1 : 0
})

mohair_cfgfile = configure_file(
   input        : cpp_srcdir / 'mohair-config.hpp.in'
  ,output       : 'mohair-config.hpp'
  ,configuration: mohair_cfgdata
)


# >> Grouped dependencies
dep_query   = [dep_arrow, dep_acero, dep_absl, dep_proto]
# dep_query   = [dep_arrow, dep_acero, dep_mohairsubstrait]
dep_service = dep_query + [dep_flight]

#   |> add optional dependencies
if dep_faodel.found()
  dep_service += [dep_ompi, dep_faodel]

elif dep_ompi.found()
  dep_service += [dep_ompi]

endif

if dep_tiledb.found()
  dep_service += [dep_tiledb]
endif

if dep_duckdb.found()
  dep_service += [dep_duckdb]
endif


# ------------------------------
# Composable lists of headers

# >> For substrait definitions
# substrait_hdrlist = [
#    cpp_proto_mohair    / 'algebra.pb.h'
#   ,cpp_proto_mohair    / 'topology.pb.h'
#   ,cpp_proto_substrait / 'type.pb.h'
#   ,cpp_proto_substrait / 'function.pb.h'
#   ,cpp_proto_substrait / 'plan.pb.h'
#   ,cpp_proto_substrait / 'algebra.pb.h'
#   ,cpp_proto_substrait / 'capabilities.pb.h'
#   ,cpp_proto_substrait / 'parameterized_types.pb.h'
#   ,cpp_proto_substrait / 'type_expressions.pb.h'
#   ,cpp_proto_substrait / 'extended_expression.pb.h'
#   ,cpp_proto_substrait / 'extensions' / 'extensions.pb.h'
# ]

# >> For decomposable queries
query_hdrlist = [
   cpp_srcdir   / 'mohair.hpp'
  ,cpp_srcdir   / 'mohair_macros.hpp'
  ,cpp_srcdir   / 'mohair_codes.hpp'
  ,cpp_querydir / 'plans.hpp'
  ,cpp_querydir / 'operators.hpp'
  ,cpp_querydir / 'messages.hpp'

   # External dependencies
  ,cpp_srcdir   / 'apidep_standard.hpp'
  ,cpp_srcdir   / 'apidep_arrow.hpp'
  ,cpp_querydir / 'apidep_substrait.hpp'
]

# >> For flight services
services_hdrlist = [
   cpp_srcdir     / 'mohair.hpp'
  ,cpp_srcdir     / 'mohair_macros.hpp'
  ,cpp_srcdir     / 'mohair_codes.hpp'
  ,cpp_querydir   / 'plans.hpp'
  ,cpp_querydir   / 'messages.hpp'
  ,cpp_enginedir  / 'adapter_acero.hpp'
  ,cpp_enginedir  / 'adapter_faodel.hpp'
  ,cpp_servicedir / 'service_mohair.hpp'
  ,cpp_servicedir / 'service_topology.hpp'
  ,cpp_servicedir / 'service_faodel.hpp'
  ,cpp_servicedir / 'service_duckdb.hpp'

   # External dependencies
  ,cpp_srcdir     / 'apidep_standard.hpp'
  ,cpp_srcdir     / 'apidep_arrow.hpp'
  ,cpp_querydir   / 'apidep_substrait.hpp'
  ,cpp_servicedir / 'apidep_flight.hpp'
]


# ------------------------------
# Composable lists of sources

# >> For substrait implementations
# substrait_srclist = [
#    cpp_proto_mohair    / 'algebra.pb.cc'
#   ,cpp_proto_mohair    / 'topology.pb.cc'
#   ,cpp_proto_substrait / 'algebra.pb.cc'
#   ,cpp_proto_substrait / 'extensions' / 'extensions.pb.cc'
#   ,cpp_proto_substrait / 'type.pb.cc'
#   ,cpp_proto_substrait / 'function.pb.cc'
#   ,cpp_proto_substrait / 'type_expressions.pb.cc'
#   ,cpp_proto_substrait / 'parameterized_types.pb.cc'
#   ,cpp_proto_substrait / 'extended_expression.pb.cc'
#   ,cpp_proto_substrait / 'capabilities.pb.cc'
#   ,cpp_proto_substrait / 'plan.pb.cc'
# ]

# >> For decomposable queries
query_srclist = [
   cpp_srcdir   / 'util.cpp'
  ,cpp_querydir / 'messages.cpp'
  ,cpp_querydir / 'plans.cpp'
  ,cpp_querydir / 'operators.cpp'
]

# >> For flight services
services_srclist = [
   cpp_servicedir / 'adapter_flight.cpp'
  ,cpp_servicedir / 'service_mohair.cpp'
  ,cpp_servicedir / 'client_mohair.cpp'
  ,cpp_enginedir  / 'execution.cpp'
]


# ------------------------------
# Composed header and source lists (organized by library/binary)

# >> Library for mohair query processing
# libmohair_srclist = (substrait_srclist + query_srclist)
libmohair_srclist = query_srclist

# >> Binary for mohair flight services
# mohair_srv_srclist = (substrait_srclist + query_srclist + services_srclist)
mohair_srv_srclist = (query_srclist + services_srclist)


# ------------------------------
# Library definitions

libmohair = library('mohair'
  ,libmohair_srclist
  ,dependencies       : [dep_service, dep_mohairsubstrait]
  ,include_directories: [protocol_incdir]
  # ,link_with          : dep_mohairsubstrait
  ,install            : true
)

module_pkgcfg.generate(libmohair)

# ------------------------------
# Installation

install_headers(query_hdrlist, subdir: 'mohair')
libmohair_dep = declare_dependency(
   include_directories: [protocol_incdir]
  ,dependencies       : dep_service
  ,link_with          : libmohair
)


# ------------------------------
# Binaries

# >> Executables we always create

#   |> simple reader for Arrow files
bin_readarrow_srclist = [
   cpp_tooldir  / 'read-arrow.cpp'
  ,cpp_srcdir   / 'util.cpp'
]

if dep_duckdb.found()
  bin_readarrow_srclist += [cpp_enginedir / 'duckdb.cpp']
endif

bin_readarrow = executable('read-arrow'
  ,bin_readarrow_srclist
  ,dependencies       : [dep_arrow, dep_duckdb]
  ,include_directories: [protocol_incdir]
  ,install            : false
)

#   |> minimal query processor
bin_mohairquery_srclist = (
    [ cpp_tooldir / 'mohair.cpp' ]
  + libmohair_srclist
)

bin_mohairquery = executable('mohair'
  ,bin_mohairquery_srclist
  ,dependencies       : [dep_query, dep_duckdb]
  ,include_directories: [protocol_incdir]
  ,install            : true
)

#   |> minimal client for mohair services
bin_mohairclient_srclist = (
    [
       cpp_tooldir / 'mohair-client.cpp'
      ,cpp_tooldir / 'mohair_cli.cpp'
    ]
  + mohair_srv_srclist
)
bin_mohairclient   = executable('mohair-client'
  ,bin_mohairclient_srclist
  ,include_directories: [protocol_incdir]
  ,dependencies       : [dep_service, dep_mohairsubstrait]
  # ,link_with          : dep_mohairsubstrait
  ,install            : false
)

#   |> server-side entry point for mohair services
# NOTE: as this supports more engines, more sources need to be added
csd_srv_srclist = (
    [
       cpp_tooldir    / 'csd-service.cpp'
      ,cpp_tooldir    / 'mohair_cli.cpp'
      ,cpp_enginedir  / 'duckdb.cpp'
      ,cpp_servicedir / 'service_duckdb.cpp'
    ]
  + mohair_srv_srclist
)

bin_srv_csd = executable('csd-service'
  ,csd_srv_srclist
  ,include_directories: [protocol_incdir]
  ,dependencies       : [dep_service, dep_mohairsubstrait]
  # ,link_with          : dep_mohairsubstrait
  ,install            : false
)

#   |> simple metadata service for prototyping mohair services
topo_srv_srclist = (
    [
       cpp_tooldir    / 'topo-service.cpp'
      ,cpp_tooldir    / 'mohair_cli.cpp'
      ,cpp_servicedir / 'service_topology.cpp'
    ]
  + mohair_srv_srclist
)

bin_srv_topo = executable('topo-service'
  ,topo_srv_srclist
  ,dependencies       : dep_service
  ,include_directories: [protocol_incdir]
  ,install            : false
)

