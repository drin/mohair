# ------------------------------
# Executable targets (binaries)

# simple reader for Arrow files
readarrow_bin = executable('read-arrow'
  ,'read-arrow.cpp'
  ,include_directories: [ libskytether_inc ]
  ,dependencies       : libskytether_dep
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : true
)

# writer for Arrow files (useful for format conversion)
writearrow_bin = executable('write-arrow'
  ,'write-arrow.cpp'
  ,include_directories: [ libskytether_inc ]
  ,dependencies       : libskytether_dep
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : true
)

# minimal client for communicating with skytether services
skytether_client_bin = executable('skytether-client'
  ,['skytether-client.cpp', 'skytether_cli.cpp']
  ,include_directories: [ libskytether_inc ]
  ,dependencies       : libskytether_dep
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : true
)

# service for CS device
csd_service_bin = executable('csd-service'
  ,['csd-service.cpp', 'skytether_cli.cpp']
  ,include_directories: [ libskytether_inc ]
  ,dependencies       : libskytether_dep
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : true
)

# simple metadata service for prototyping skytether services
topo_service_bin = executable('topo-service'
  ,['topo-service.cpp', 'skytether_cli.cpp']
  ,include_directories: [ libskytether_inc ]
  ,dependencies       : libskytether_dep
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : true
)


# ------------------------------
# Sandboxes for prototyping

# for duckdb internals
duck_bin = executable('duckdb-sandbox'
  ,'duckdb-sandbox.cpp'
  ,include_directories: [ libskytether_inc ]
  ,dependencies       : libduckdb_dep
  ,install            : false
)

# for a duckdb-enabled skytether client
duck_bin = executable('duckdb-mohair'
  ,['duckdb-mohair.cpp', 'skytether_cli.cpp']
  ,include_directories: [ libskytether_inc ]
  ,dependencies       : libskytether_dep
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : false
)

# experimental client for communicating with skytether services
skytether_client_bin = executable('experiment-client'
  ,['experiment-client.cpp', 'skytether_cli.cpp']
  ,include_directories: [ libskytether_inc ]
  ,dependencies       : libskytether_dep
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : true
)

# sandbox to validate parsing of topology config
skytether_client_bin = executable('read-topology'
  ,['read-topology.cpp', 'skytether_cli.cpp']
  ,include_directories: [ libskytether_inc ]
  ,dependencies       : libskytether_dep
  ,install_rpath      : get_option('prefix') / get_option('libdir')
  ,install            : false
)
