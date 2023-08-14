# ------------------------------
# Dependencies

# >> Cython dependencies
from libcpp         cimport nullptr, bool
from libc.stdint    cimport int64_t, uint64_t
from libcpp.memory  cimport unique_ptr, shared_ptr, make_shared
from libcpp.string  cimport string
from libcpp.vector  cimport vector
from libcpp.utility cimport move

from pyarrow.lib cimport CStatus, CResult
from pyarrow.lib cimport CBuffer, CSchema, CTable, CRecordBatch


# ------------------------------
# C++ Definitions

# cdef extern from '<mohair/arrow.hpp>':
# 
#     # Define static C++ functions
#     cdef void PrintSchema(shared_ptr[CSchema] schema, int offset, int length)
#     cdef void PrintTable (shared_ptr[CTable]  table , int offset, int length)
# 
# 
# cdef extern from '<mohair/faodel.hpp>':
# 
#     # Define a wrapper for a C++ class
#     cdef cppclass KBuffer:
# 
#         # >> attributes
#         char   *base
#         size_t  len
# 
#         # >> constructors
#         KBuffer() except +
#         KBuffer(char *buf_base, size_t buf_len) except +
# 
#         # >> functions
#         string ToString()
# 
#         void Release()


