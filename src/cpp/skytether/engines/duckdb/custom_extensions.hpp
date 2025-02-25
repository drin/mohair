// ------------------------------
// License(s)
//
// >> For original duckdb code
// Copyright 2018-2024 Stichting DuckDB Foundation
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be included in all copies
// or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// >> For my modifications
//    (figure out what amount of modifications allows me to change the license at all)
// Copyright 2024 Aldrin Montana
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


// ------------------------------
// Dependencies
#pragma once

#include "skytether.hpp"
#include "skytether/engines/duckdb/adapter_duckdb.hpp"


// ------------------------------
// Aliases

namespace skytether::engines {

  // >> Templated types
  using ParamTypeMatrix = vector<vector<string>>;

  // >> Simple types
  using duckdb::Hash;
  using duckdb::CombineHash;

} // namespace: skytether::engines


// ------------------------------
// Functions and Classes

namespace skytether::engines {

  // >> Base classes to represent substrait functions and function extensions

  //! Class to describe an individual substrait function
  struct SubstraitCustomFunction {
    string         name;
    vector<string> arg_types;

    SubstraitCustomFunction() = default;
    SubstraitCustomFunction(string fn_name, vector<string> param_types)
        : name(fn_name), arg_types(std::move(param_types)) {}

    // Functions
    bool operator==(const SubstraitCustomFunction &other) const {
      return name == other.name && arg_types == other.arg_types;
    }

    string GetName();
  };

  //! Class to describe a substrait function extension (a URI for substrait function info)
  struct SubstraitFunctionExtensions {

    // Constructors
    SubstraitFunctionExtensions() = default;
    SubstraitFunctionExtensions( SubstraitCustomFunction function_p
                                ,string                  extension_path_p)
      : function(std::move(function_p)), extension_path(std::move(extension_path_p)) {}

    // Functions
    string GetExtensionURI() const;
    bool   IsNative()        const;

    // Attributes
    SubstraitCustomFunction function;
    string                  extension_path;
  };


  // >> Hash functors

  //! Hash functor for "*" (any arguments) substrait functions
  struct HashSubstraitFunctionsName {
    size_t operator()(SubstraitCustomFunction const& custom_function) const noexcept {
      return Hash(custom_function.name.c_str());
    }
  };

  //! Hash functor for "?" (repeatable argument) and regular substrait functions
  struct HashSubstraitFunctions {
    size_t operator()(SubstraitCustomFunction const& custom_function) const noexcept {
      // Hash the function arguments
      auto& fn_argtypes = custom_function.arg_types;

      auto hashed_argtypes = Hash(fn_argtypes[0].c_str());
      for (idx_t arg_ndx = 1; arg_ndx < fn_argtypes.size(); arg_ndx++) {
        hashed_argtypes = CombineHash(
          hashed_argtypes, Hash(fn_argtypes[arg_ndx].c_str())
        );
      }

      // Combine hashes for the function name and its argument types
      auto hashed_name = Hash(custom_function.name.c_str());
      return CombineHash(hashed_name, hashed_argtypes);
    }
  };


  // >> Other Classes

  //! Class that acts as a registry for substrait functions and their extensions
  struct SubstraitCustomFunctions {

    // convenience aliases (defined here to minimize type forward complexity)
    using SubstraitTypeVec  = vector<SubstraitType>;
    using SubstraitFnMap    = std::unordered_map< SubstraitCustomFunction
                                                 ,SubstraitFunctionExtensions
                                                 ,HashSubstraitFunctions     >;

    using SubstraitAnyFnMap = std::unordered_map< SubstraitCustomFunction
                                                 ,SubstraitFunctionExtensions
                                                 ,HashSubstraitFunctionsName>;

    // >> Attributes
    SubstraitFnMap    custom_functions;   // For regular functions
    SubstraitAnyFnMap any_arg_functions;  // For "*"     functions
    SubstraitFnMap    many_arg_functions; // For "?"     functions

    // >> Constructors
    SubstraitCustomFunctions();

    // Methods
    void Initialize();
    SubstraitFunctionExtensions Get(const string& name, const SubstraitTypeVec& types) const;

    // Static methods
	  static vector<string> GetTypes(const vector<SubstraitType>& types);

    void InsertCustomFunction( string         fn_name
                              ,string         ext_fpath
                              ,vector<string> param_types);

    void InsertFunctionExtension( string         name_p
                                 ,vector<string> types_p
                                 ,string         file_path);

    // A recursive function that calls `InsertCustomFunction` when it bottoms out
    void InsertAllFunctions( const ParamTypeMatrix& fn_param_types
                            ,vector<idx_t>&         ptype_indices
                            ,int                    param_ndx
                            ,string&                name
                            ,string&                file_path);
  };

} // namespace skytether::engines
