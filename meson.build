# ------------------------------
# Project Definition

# >> project info
project('libskytether-mohair'
  ,'cpp'
  ,version         : '0.5.0'
  ,meson_version   : '1.6.0'
  ,license         : 'Apache-2.0'
  ,license_files   : 'LICENSE'
  ,default_options : ['warning_level=3','cpp_std=c++17']
)

# >> project configuration
version_str    = meson.project_version()
version_fields = version_str.split('.')


# ------------------------------
# Dependencies

# >> for finding alternate installations
cpp_compiler = meson.get_compiler('cpp')

# alternate paths
libduckdb_customdir = '/usr' / 'local' / 'duckdb-skytether'

libduckdb_libdir = libduckdb_customdir / 'lib'
libduckdb_incdir = libduckdb_customdir / 'include'

# flags so we know when to search for fallbacks
is_custom_libduckdb = false


# >> Backend dependency: tiledb
libtiledb_dep = dependency('tiledb', required: get_option('tiledb'))

# >> Backend dependency: duckdb
# search typical location first (never required)
libduckdb_dep = dependency('duckdb-skytether', method: 'cmake', required: false)

# search alternate location last (fail if duckdb is enabled)
if not libduckdb_dep.found()
  libduckdb_dep = cpp_compiler.find_library('duckdb'
    ,dirs    : libduckdb_libdir
    ,required: get_option('duckdb')
  )

  if libduckdb_dep.found()
    is_custom_libduckdb = true
    libduckdb_inc = include_directories(libduckdb_incdir)
  endif
endif


# >> configure build options based on libraries we found
mohair_cfgdata = configuration_data({
   'VERSION_STR'   : version_str
  ,'VERSION_MAJOR' : version_fields[0]
  ,'VERSION_MINOR' : version_fields[1]
  ,'VERSION_PATCH' : version_fields[2]
  ,'BUILD_IS_DEBUG': get_option('buildtype') == 'debug' ? 1 : 0
  ,'TILEDB'        : libtiledb_dep.found()   ? 1 : 0
  ,'DUCKDB'        : libduckdb_dep.found()   ? 1 : 0
})


# ------------------------------
# Reference paths

# base directory
cpp_srcdir  = 'src' / 'cpp'
cpp_tooldir = cpp_srcdir / 'toolbox'


# ------------------------------
# Build definitions

mohair_cfgfile = configure_file(
   input        : cpp_srcdir / 'mohair-config.hpp.in'
  ,output       : 'mohair-config.hpp'
  ,configuration: mohair_cfgdata
  ,install_dir  : get_option('includedir') / 'skytether-mohair'
)

# >> Invoke from subdirectories

# NOTE: this seems to be necessary if we generate a config file
configuration_inc = include_directories('.')

# creates target libskytether_mohair
subdir(cpp_srcdir)


# ------------------------------
# Package configurations

# use pkg-config to generate library build info
module_pkgcfg = import('pkgconfig')
module_pkgcfg.generate(libskytether_mohair, subdirs: 'skytether-mohair')


# ------------------------------
# Build artifacts (binaries)

# >> simple reader for Arrow files
readarrow_sources = [ cpp_tooldir  / 'read-arrow.cpp' ]

readarrow_bin = executable('read-arrow'
  ,readarrow_sources
  ,include_directories: [configuration_inc, libskytether_mohair_inc]
  ,dependencies       : libskytether_mohair_dep
  ,install            : true
)

# >> target binary for prototyping query processing
mohair_sources = [ cpp_tooldir / 'mohair.cpp' ]

mohair_bin = executable('mohair'
  ,mohair_sources
  ,include_directories: [configuration_inc, libskytether_mohair_inc]
  ,dependencies       : libskytether_mohair_dep
  ,install            : true
)

# >> minimal client for communicating with mohair services
mohair_client_sources = [
   cpp_tooldir / 'mohair-client.cpp'
  ,cpp_tooldir / 'mohair_cli.cpp'
]

mohair_client_bin = executable('mohair-client'
  ,mohair_client_sources
  ,include_directories: [configuration_inc, libskytether_mohair_inc]
  ,dependencies       : libskytether_mohair_dep
  ,install            : true
)

# >> service for CS device
csd_service_sources = [
   cpp_tooldir / 'csd-service.cpp'
  ,cpp_tooldir / 'mohair_cli.cpp'
]

csd_service_bin = executable('csd-service'
  ,csd_service_sources
  ,include_directories: [configuration_inc, libskytether_mohair_inc]
  ,dependencies       : libskytether_mohair_dep
  ,install            : true
)

# >> simple metadata service for prototyping mohair services
topo_service_sources = [
   cpp_tooldir / 'topo-service.cpp'
  ,cpp_tooldir / 'mohair_cli.cpp'
]

topo_service_bin = executable('topo-service'
  ,topo_service_sources
  ,include_directories: [configuration_inc, libskytether_mohair_inc]
  ,dependencies       : libskytether_mohair_dep
  ,install            : true
)

