# Overview

"Mohair" is a project to prototype the use of [substrait][web-substrait] and
[arrow][web-arrow] to handle pushdown of partial queries to remote storage. Initially,
this project will largely inspired by [Skytether][repo-skytether]--an extension of
[SkyhookDM][repo-skyhookdm] for single-cell gene expression analysis and computational
storage. However, work on skytether narrowly started before [Arrow Flight][docs-flight],
[Acero][docs-acero], and substrait were started. So, a lot of skytether is low-level and
specific, where mohair will try to be higher-level and more generic.

# Goals

The overall goal is to be able to delegate part of a query plan to remote storage, execute
the remaining query plan on the intermediate results (from remote storage), and then
return the final results to the client. It is expected that this will require some amount
of:
* splitting a query plan
* implementing a flight service
* communicating with a remote flight service
* communicating with flight services using substrait

## Milestones

For an informal tracking of progress, we list some milestones here (which will be updated
as appropriate).

1. Execute a single query without splitting
   1. Submit query as a substrait plan
   2. Submit query to a flight service
   3. Implement flight service as a very simple data server (probably using a file system
      for now)
2. Execute a single query with a simple split
   1. Submit query as a substrait plan
   2. Split plan into 2 pieces
   3. Execute the 2nd piece on the intermediate results of the 1st piece.


# Getting Started

This section is a bootstrap guide for trying out the code in this repository. Here, I will
try to highlight the types of interactions I am trying to support and where in the code
they are implemented (so that it's possible to further explore the code if you're
interested).

## Installation

Note that I may have forgotten to include some steps necessary for installation. If this
is the case, let me know or file an issue in the [mohair issue tracker][issues-mohair].

### Dependencies

The C++ code in this repository depends on [Arrow][web-arrow], [Substrait][web-substrait],
and [DuckDB][web-duckdb]. I am trying to simplify installation of dependencies, but for
now this is only done for macosx using [Homebrew][web-homebrew].

I created a [homebrew tap][docs-tap], which is located at
[drin/homebrew-hatchery][repo-hatchery]:
```bash
# Opening my tap is optional
brew tap drin/hatchery
brew install apache-arrow-substrait

# In case my tap isn't tapped
# brew install drin/hatchery/apache-arrow-substrait

# and then the other formulas
brew install duckdb-substrait

# this is not yet working
# brew install skytether-mohair
```

### Build systems

To build `C++` code, I use [meson][web-meson]. To manage `python` code, I use
[poetry][web-poetry].

##### Building C++

To build the `C++` code:
```bash
brew install meson ninja git-lfs

git clone https://github.com/drin/mohair.git
pushd mohair

# Optional: these submodules are only needed for regenerating protobuf code
# git submodule init   -- submodules/substrait-proto
# git submodule update -- submodules/substrait-proto
# git submodule init   -- submodules/mohair-proto
# git submodule update -- submodules/mohair-proto
# NOTE: to regenerate, refer to the `Compiling Protobuf Wrappers` section

# Optional: git-lfs is only really needed for getting examples and such
# git lfs install --local
# git lfs pull

# "build-dir" is the name I use for my build directory
meson setup      build-dir
meson compile -C build-dir
```

Although the formula itself doesn't work (I'm not yet sure why), the formula logic should
be helpful as a reference for what commands to use:
[drin/homebrew-hatchery/skytether-mohair][formula-mohair].

##### Building Python

To build the `python` code:
```bash
brew install poetry

# poetry commands assume you're in the repository root
poetry install
```

##### Compiling Protobuf Wrappers

This will be done at a future date (it shouldn't be important now).

The short story is:
```bash
buf generate --template buf.gen.yaml submodules/substrait-proto
buf generate --template buf.gen.yaml submodules/mohair-proto

# NOTE: the below fixes don't accommodate the extensions.proto includes because of the
#       extra nesting

# For fixing the includes in the C++ code
# sed -i '' 's/include ["]substrait[/]/include "..\/substrait\//' (grep -Rl "include \"substrait" src/cpp/query/substrait/)
# sed -i '' 's/include ["]mohair[/]/include "..\/mohair\//'       (grep -Rl "include \"mohair"    src/cpp/query/mohair/)

# For fixing the imports in the python code
# sed -i '' 's/from substrait/from mohair.substrait/' (grep -Rl "from substrait" src/python/mohair/substrait/)
# sed -i '' 's/from substrait/from mohair.substrait/' (grep -Rl "from substrait" src/python/mohair/mohair/)
```


<!-- resources -->
[web-substrait]:  https://substrait.io/
[web-arrow]:      https://arrow.apache.org/
[web-duckdb]:     https://duckdb.org/
[web-homebrew]:   https://brew.sh/
[web-meson]:      https://mesonbuild.com/
[web-poetry]:     https://python-poetry.org/

[docs-flight]:    https://arrow.apache.org/docs/format/Flight.html
[docs-acero]:     https://arrow.apache.org/docs/cpp/streaming_execution.html
[docs-tap]:       https://docs.brew.sh/How-to-Create-and-Maintain-a-Tap

[repo-skytether]: https://gitlab.com/skyhookdm/skytether-singlecell
[repo-skyhookdm]: https://github.com/uccross/skyhookdm-ceph-cls
[repo-hatchery]:  https://github.com/drin/homebrew-hatchery

[issues-mohair]:  https://github.com/drin/mohair/issues

[formula-mohair]: https://github.com/drin/homebrew-hatchery/blob/mainline/Formula/skytether-mohair.rb
