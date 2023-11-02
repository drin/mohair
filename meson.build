# ------------------------------
# Project Definition

# >> configuration
project('mohair-faodel'
  ,'cpp'
  ,version         : '0.1'
  ,default_options : ['warning_level=3','cpp_std=c++17']
)


# ------------------------------
# Meson dependencies (modules)

# use pkg-config to generate library build info
module_pkgcfg = import('pkgconfig')


# >> Determine the system we're building on
buildsys_type = 'unknown'

os_name = run_command('uname', '-s').stdout().strip()
if os_name == 'Darwin'
    buildsys_type = 'macosx'

elif os_name == 'Linux'
    os_is_arch   = run_command('grep', '--silent', 'NAME="Arch Linux"', '/etc/os-release')
    os_is_ubuntu = run_command('grep', '--silent', 'NAME="Ubuntu"'    , '/etc/os-release')

    if os_is_arch.returncode() == 0
        buildsys_type = 'archlinux'

    elif os_is_ubuntu.returncode() == 0
        buildsys_type = 'ubuntu'

    endif

endif


# ------------------------------
# Library dependencies

# >> System-dependent dependencies
if   buildsys_type == 'macosx'
  cpp_compiler = meson.get_compiler('cpp')
  prefix_local = '/usr' / 'local' / 'arrow-dev' / 'lib'

  #   |> Arrow
  dep_arrow  = cpp_compiler.find_library('arrow'          , dirs: prefix_local, static: false)
  dep_acero  = cpp_compiler.find_library('arrow-substrait', dirs: prefix_local, static: false)
  dep_flight = cpp_compiler.find_library('arrow-flight'   , dirs: prefix_local, static: false)

elif buildsys_type == 'archlinux'

  #   |> Arrow
  dep_arrow  = dependency('arrow')
  dep_acero  = dependency('arrow-substrait') # this is Acero + substrait
  dep_flight = dependency('arrow-flight')
endif


# >> Individual dependencies

#   |> Faodel
dep_ompi   = dependency('ompi-cxx')
dep_faodel = dependency('faodel')
# dep_fado   = dependency('faodel-arrow')

#   |> Protobuf (for substrait)
dep_proto   = dependency('protobuf')
dep_absllog = dependency('absl_log')

# >> Grouped dependencies
dep_service   = [dep_acero, dep_flight, dep_ompi, dep_faodel, dep_proto, dep_absllog]
dep_queryplan = [dep_arrow, dep_proto, dep_absllog]


# ------------------------------
# Build variables (convenience)

# base directory
cpp_srcdir    = 'src' / 'cpp'

# dir for core mohair sources
cpp_coredir   = cpp_srcdir / 'mohair'

# dir for generated protobuf code
cpp_proto_substrait = cpp_srcdir / 'mohair' / 'substrait'
cpp_proto_mohair    = cpp_srcdir / 'mohair' / 'mohair'

# dir for adapters (execution engines)
cpp_enginedir = cpp_srcdir / 'engines'

# dir for flight services
cpp_servicedir = cpp_srcdir / 'services'

# dir for binary sources (1-1 with binaries)
cpp_tooldir   = cpp_srcdir / 'toolbox'


# ------------------------------
# Composable lists of headers

# >> For substrait definitions
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

# >> For decomposable queries
queryplan_hdrlist = [
   cpp_coredir / 'mohair.hpp'
  ,cpp_coredir / 'plans.hpp'
  ,cpp_coredir / 'operators.hpp'
]

# >> For flight services
services_hdrlist = [
   cpp_coredir    / 'mohair.hpp'
  ,cpp_enginedir  / 'adapter_acero.hpp'
  ,cpp_servicedir / 'flight_internal.hpp'
  ,cpp_enginedir  / 'adapter_faodel.hpp'
  ,cpp_servicedir / 'service_faodel.hpp'
]


# ------------------------------
# Composable lists of sources

# >> For substrait implementations
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

# >> For decomposable queries
queryplan_srclist = [
   cpp_coredir / 'util.cpp'
  ,cpp_coredir / 'plans.cpp'
  ,cpp_coredir / 'operators.cpp'
]

# >> For flight services
services_srclist = [
   cpp_coredir / 'util.cpp'
  ,cpp_enginedir  / 'execution.cpp'
  ,cpp_enginedir  / 'faodel.cpp'
  ,cpp_servicedir / 'flight_internal.cpp'
  ,cpp_servicedir / 'service_faodel.cpp'
]


# ------------------------------
# Composed header and source lists (organized by library/binary)

# >> Library for mohair query processing
lib_mohairquery_hdrlist = (substrait_hdrlist + queryplan_hdrlist)
lib_mohairquery_srclist = (substrait_srclist + queryplan_srclist)

# >> Binary for mohair flight services
bin_mohairservice_hdrlist = services_hdrlist
bin_mohairservice_srclist = services_srclist


# ------------------------------
# Library definitions

lib_mohairquery = library('mohair_query'
  ,lib_mohairquery_srclist
  ,dependencies: dep_queryplan
  ,install     : true
)

module_pkgcfg.generate(lib_mohairquery)

# ------------------------------
# Installation

install_headers(queryplan_hdrlist, subdir: 'mohair')


# ------------------------------
# Binaries

# >> Executable for simple mohair binary (just query processing)
bin_mohairquery_srclist = (
    [ cpp_tooldir / 'mohair.cpp' ]
  + lib_mohairquery_srclist
)

bin_mohair = executable('mohair-query'
  ,bin_mohairquery_srclist
  ,dependencies: dep_queryplan
  ,install     : false
)

# >> Executable for faodel flight service
bin_faodelservice_srclist = (
    [ cpp_tooldir / 'faodel-service.cpp' ]
  + bin_mohairservice_srclist
)

bin_faodelservice = executable('faodel-service'
  ,bin_faodelservice_srclist
  ,dependencies: dep_service
  ,install     : false
)

# >> Executable for faodel flight client
bin_faodelclient_srclist = (
    [ cpp_tooldir / 'faodel-client.cpp' ]
  + bin_mohairservice_srclist
)

bin_faodelclient = executable('faodel-client'
  ,bin_faodelclient_srclist
  ,dependencies: dep_service
  ,install     : false
)
