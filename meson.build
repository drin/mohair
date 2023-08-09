# ------------------------------
# Project Definition

project('mohair-faodel'
  ,'cpp'
  ,version         : '0.1'
  ,default_options : ['warning_level=3','cpp_std=c++17']
)

cpp_srcdir  = 'src' / 'cpp'
cpp_tooldir = cpp_srcdir / 'toolbox'

# ------------------------------
# Dependencies

dep_ompi   = dependency('ompi-cxx')
dep_faodel = dependency('faodel')
dep_arrow  = dependency('arrow')


# ------------------------------
# Source lists

# >> Sources for primary mohair binary
mohair_srclist = [
  cpp_tooldir / 'mohair.cpp'
]


# ------------------------------
# Binaries

# >> Executable for primary mohair binary
mohair = executable('mohair'
  ,mohair_srclist
  ,dependencies: [dep_ompi, dep_faodel, dep_arrow]
  ,install     : false
)
