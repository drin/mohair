#!/usr/bin/env python

# ------------------------------
# License

# Copyright 2023 Aldrin Montana
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# ------------------------------
# Module Docstring
"""
Module that handles anything convenient but low level for skytether.
"""

# ------------------------------
# Compiler directives

# distutils: language = c++


# ------------------------------
# Dependencies

# >> Python dependencies
from pyarrow     import py_buffer
from pyarrow.lib import frombytes, tobytes

# >> Cython dependencies
#    |> types
from libcpp         cimport nullptr, bool as c_bool
from libc.stdint    cimport int64_t, uint64_t
from libcpp.memory  cimport unique_ptr, shared_ptr, make_shared
from libcpp.string  cimport string
from libcpp.vector  cimport vector
from libcpp.utility cimport move

from pyarrow.lib cimport CStatus, CResult
from pyarrow.lib cimport CBuffer, CSchema, CTable, CRecordBatch
from pyarrow.lib cimport CRecordBatchReader, RecordBatchReader

from pyarrow.includes.libarrow_acero     cimport CDeclaration, CTableSourceNodeOptions, CExecNodeOptions
from pyarrow.includes.libarrow_substrait cimport CConversionOptions, CNamedTableProvider

#   |> functions
from libcpp.memory   cimport static_pointer_cast
from cython.operator cimport dereference as deref

from pyarrow.lib     cimport GetResultValue
from pyarrow.lib     cimport pyarrow_unwrap_buffer, pyarrow_unwrap_table
from pyarrow.lib     cimport pyarrow_wrap_schema

from pyarrow.includes.common             cimport BindFunction
from pyarrow.includes.libarrow           cimport GetFunctionRegistry
from pyarrow.includes.libarrow_substrait cimport ExecuteSerializedPlan
from pyarrow.includes.libarrow_substrait cimport default_extension_id_registry

# from pyarrow._substrait cimport _create_named_table_provider


# ------------------------------
# Python classes and functions

def run_query(plan, table_provider):
    """
    Function like `pyarrow.substrait.run_query` that executes a substrait plan using
    Acero.

    :plan: is a python `bytes` type
    """

    # >> validate params
    if not isinstance(plan, bytes):
        raise TypeError(f'Expected bytes, got "{type(plan)}"')

    if table_provider is None:
        raise TypeError('Table provider cannot be "None"')

    # >> prepare table provider
    cdef CConversionOptions c_cnv_opts
    named_table_args = { "provider": table_provider }
    c_cnv_opts.named_table_provider = BindFunction[CNamedTableProvider](
         &_create_named_table_provider
        ,named_table_args
    )

    # >> prepare serialized plan and other variables
    cdef CResult[shared_ptr[CRecordBatchReader]] c_res_reader

    cdef c_bool              c_use_threads = True
    cdef shared_ptr[CBuffer] c_buf_plan    = pyarrow_unwrap_buffer(py_buffer(plan))

    # >> execute the plan
    with nogil:
        c_res_reader = ExecuteSerializedPlan(
             deref(c_buf_plan)
            ,default_extension_id_registry()
            ,GetFunctionRegistry()
            ,c_cnv_opts
            ,c_use_threads
        )

    # >> return a python RecordBatchReader to read the result set
    cdef shared_ptr[CRecordBatchReader] c_reader = GetResultValue(c_res_reader)
    cdef RecordBatchReader reader = RecordBatchReader.__new__(RecordBatchReader)

    reader.reader = c_reader
    return reader


# ------------------------------
# Cython classes and functions

cdef CDeclaration _create_named_table_provider(       dict            named_args
                                               ,const vector[string]& names
                                               ,const CSchema&        schema):
    # collect table name parts (meant to represent canonical table name)
    py_names = []
    cdef string c_name
    for i in range(names.size()):
        c_name = names[i]
        py_names.append(frombytes(c_name))

    # access the table from the provider (using name and schema)
    py_schema = pyarrow_wrap_schema(make_shared[CSchema](schema))
    py_table  = named_args["provider"](py_names, py_schema)
    cdef shared_ptr[CTable] c_in_table = pyarrow_unwrap_table(py_table)

    # convert the Table -> TableSourceNodeOptions -> ExecNodeOptions
    cdef shared_ptr[CTableSourceNodeOptions] c_tablesourceopts
    cdef shared_ptr[CExecNodeOptions]        c_input_node_opts
    c_tablesourceopts = make_shared[CTableSourceNodeOptions](c_in_table)
    c_input_node_opts = static_pointer_cast[CExecNodeOptions, CTableSourceNodeOptions](c_tablesourceopts)

    # return a Declaration that treats the ExecNodeOptions as a table source
    cdef vector[CDeclaration.Input] empty_inputs
    return CDeclaration(tobytes("table_source"), empty_inputs, c_input_node_opts)
