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


# >> Required dependencies
#   |> Protobuf (for substrait)
dep_proto   = dependency('protobuf')
dep_absllog = dependency('absl_log')


# >> Optional dependencies
#   |> Faodel
dep_ompi   = dependency('ompi-cxx', required: get_option('mpi'))
dep_faodel = dependency('faodel'  , required: get_option('faodel'))


# >> Make configuration data available to source files
mohair_cfgdata = configuration_data({
   'MPI'   : dep_ompi.found()
  ,'FAODEL': dep_faodel.found()
  ,'TILEDB': dep_tiledb.found()
  ,'DUCKDB': dep_duckdb.found()
})

mohair_cfgfile = configuration_file(
   input : 'mohair-config.hpp.in'
  ,output: 'mohair-config.hpp'
)


# >> Grouped dependencies
dep_service = [dep_acero, dep_flight, dep_ompi, dep_faodel, dep_proto, dep_absllog]
dep_query   = [dep_acero, dep_proto, dep_absllog]

#   |> add optional dependencies
if dep_faodel.found()
  dep_service += [dep_ompi, dep_faodel]

elif dep_ompi.found()
  dep_service += [dep_ompi]

endif


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
query_hdrlist = [
   cpp_coredir / 'mohair.hpp'
  ,cpp_coredir / 'plans.hpp'
  ,cpp_coredir / 'operators.hpp'
  ,cpp_coredir / 'messages.hpp'
]

# >> For flight services
services_hdrlist = [
   cpp_coredir    / 'mohair.hpp'
  ,cpp_coredir    / 'plans.hpp'
  ,cpp_coredir    / 'messages.hpp'
  ,cpp_enginedir  / 'adapter_acero.hpp'
  ,cpp_enginedir  / 'adapter_faodel.hpp'
  ,cpp_servicedir / 'service_mohair.hpp'
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
query_srclist = [
   cpp_coredir / 'util.cpp'
  ,cpp_coredir / 'plans.cpp'
  ,cpp_coredir / 'operators.cpp'
  ,cpp_coredir / 'messages.cpp'
]

# >> For flight services
services_srclist = [
   cpp_coredir    / 'util.cpp'
  ,cpp_coredir    / 'plans.cpp'
  ,cpp_coredir    / 'operators.cpp'
  ,cpp_enginedir  / 'execution.cpp'
  ,cpp_servicedir / 'service_mohair.cpp'
]


# ------------------------------
# Composed header and source lists (organized by library/binary)

# >> Library for mohair query processing
lib_mohair_srclist = (substrait_srclist + query_srclist)

# >> Binary for mohair flight services
mohair_srv_srclist = (substrait_srclist + services_srclist)


# ------------------------------
# Library definitions

lib_mohair = library('mohair'
  ,lib_mohair_srclist
  ,dependencies: dep_query
  ,install     : true
)

module_pkgcfg.generate(lib_mohair)

# ------------------------------
# Installation

install_headers(query_hdrlist, subdir: 'mohair')


# ------------------------------
# Binaries

# >> Executables we always create

#   |> simple reader for Arrow files
bin_readarrow_srclist = [
   cpp_tooldir / 'read-arrow.cpp'
  ,cpp_coredir / 'util.cpp'
]

bin_readarrow = executable('read-arrow'
  ,bin_readarrow_srclist
  ,dependencies: dep_arrow
  ,install     : false
)

#   |> minimal query processor
bin_mohairquery_srclist = (
    [ cpp_tooldir / 'mohair.cpp' ]
  + lib_mohair_srclist
)

bin_mohairquery = executable('mohair'
  ,bin_mohairquery_srclist
  ,dependencies: dep_query
  ,install     : false
)

#   |> minimal client for mohair services
mohair_srv_srcmain = [ cpp_tooldir / 'mohair-client.cpp' ]
bin_faodelclient   = executable('mohair-client'
  ,(mohair_srv_srcmain + mohair_srv_srclist)
  ,dependencies: dep_service
  ,install     : false
)


# ------------------------------
# Feature-based executables

# >> Faodel mohair service
if dep_faodel.found()

  # main source file and faodel-specific source files
  faodel_srv_srclist = [
     cpp_tooldir    / 'faodel-service.cpp'
    ,cpp_enginedir  / 'faodel.cpp'
    ,cpp_servicedir / 'service_faodel.cpp'
  ]

  bin_srv_faodel = executable('faodel-service'
    ,(faodel_srv_srclist + mohair_srv_srclist)
    ,dependencies: dep_service
    ,install     : false
  )

endif
