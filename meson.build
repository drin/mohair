# ------------------------------
# Project Definition

# >> project info
project('libskytether'
  ,'cpp'
  ,version         : '1.1.4'
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
   input        : cpp_srcdir / 'skytether-config.hpp.in'
  ,output       : 'skytether-config.hpp'
  ,configuration: mohair_cfgdata
  ,install_dir  : get_option('includedir') / 'skytether'
)


# ------------------------------
# Build targets

# >> Include targets

libskytether_inc = [
  include_directories('.')
]


# >> Invoke from subdirectories

# creates library and dependency targets
subdir(cpp_srcdir)


# creates executable targets (binaries)
subdir(cpp_tooldir)

