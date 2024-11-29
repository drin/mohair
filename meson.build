# ------------------------------
# Project Definition

# >> project info
project('libskytether-mohair'
  ,'cpp'
  ,version         : '1.1.3'
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

cpp_compiler = meson.get_compiler('cpp')

# >> Backend dependency: tiledb
libtiledb_dep = dependency('tiledb', required: get_option('tiledb'))

# >> Backend dependency: duckdb
# search typical location first (never required)
libduckdb_dep = dependency('duckdb', method: 'cmake', required: false)

if not get_option('duckdb').disabled() and not libduckdb_dep.found()
  is_duckdb_found = false

  # try to find duckdb using each provided cmake prefix path
  foreach lib_prefixdir : get_option('cmake_prefix_path')
    libduckdb = cpp_compiler.find_library('duckdb'
      ,dirs    : lib_prefixdir / 'lib'
      ,required: false
    )

    # if we didn't find it in this path, go to the next one
    if not libduckdb.found()
      continue
    endif

    # we found it, so update our dependency and stop searching
    libduckdb_inc = include_directories(lib_prefixdir / 'include')
    libduckdb_dep = declare_dependency(
       dependencies       : [libduckdb]
      ,include_directories: [libduckdb_inc]
    )

    is_duckdb_found = true
    break
  endforeach

  # check if the loop was able to find duckdb (and if we required it in the first place)
  if not is_duckdb_found and get_option('duckdb').enabled()
    error('Unable to find duckdb dependency with explicit fallback')
  endif

endif

# >> configure build options based on libraries we found
mohair_cfgdata = configuration_data({
   'VERSION_STR'   : version_str
  ,'VERSION_MAJOR' : version_fields[0]
  ,'VERSION_MINOR' : version_fields[1]
  ,'VERSION_PATCH' : version_fields[2]
  ,'BUILD_IS_DEBUG': get_option('buildtype') == 'debug' ? 1 : 0
  ,'TILEDB'        : libtiledb_dep.found()              ? 1 : 0
  ,'DUCKDB'        : libduckdb_dep.found()              ? 1 : 0
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
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : true
)

# >> writer for Arrow files (useful for format conversion)
writearrow_sources = [ cpp_tooldir  / 'write-arrow.cpp' ]

writearrow_bin = executable('write-arrow'
  ,writearrow_sources
  ,include_directories: [configuration_inc, libskytether_mohair_inc]
  ,dependencies       : libskytether_mohair_dep
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : true
)

# >> target binary for prototyping query processing
mohair_sources = [ cpp_tooldir / 'mohair.cpp' ]

mohair_bin = executable('mohair'
  ,mohair_sources
  ,include_directories: [configuration_inc, libskytether_mohair_inc]
  ,dependencies       : libskytether_mohair_dep
  ,install_rpath      : get_option('prefix') / get_option('libdir')
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
  ,install_rpath      : get_option('prefix') / get_option('libdir')
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
  ,install_rpath      : get_option('prefix') / get_option('libdir')
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
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : true
)

