// ------------------------------
// License(s)
//
// Copyright 2024-2025 Aldrin Montana
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

#include "skytether/engines/duckdb/adapter_duckdb.hpp"


// ------------------------------
// Functions

namespace skytether::engines {

  int32_t GetTimestampPrecision(LogicalTypeId type) {
    switch (type) {
      case LogicalTypeId::TIMESTAMP_SEC: return 0;
      case LogicalTypeId::TIMESTAMP_MS:  return 3;
      case LogicalTypeId::TIMESTAMP:     return 6;
      case LogicalTypeId::TIMESTAMP_NS:  return 9;
      default:                           break;
    }

    throw duckdb::InternalException(
      "Only timestamp values can have a timestamp precision"
    );
  }



  unique_ptr<SubstraitType>
  FromDuckType(const LogicalType& type, bool not_null) {
    auto substrait_dtype = std::make_unique<SubstraitType>();

    SubstraitType::Nullability nullable {
      not_null ? SubstraitType::NULLABILITY_REQUIRED
               : SubstraitType::NULLABILITY_NULLABLE
    };

    // TODO: propose unsigned types to enable optimizations
    switch (type.id()) {
      case LogicalTypeId::BOOLEAN: {
        substrait_dtype->set_allocated_bool_(new SubstraitType::Boolean());
        substrait_dtype->mutable_bool_()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::TINYINT: {
        substrait_dtype->set_allocated_i8(new SubstraitType::I8());
        substrait_dtype->mutable_i8()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::UTINYINT:
      case LogicalTypeId::SMALLINT: {
        substrait_dtype->set_allocated_i16(new SubstraitType::I16());
        substrait_dtype->mutable_i16()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::USMALLINT:
      case LogicalTypeId::INTEGER: {
        substrait_dtype->set_allocated_i32(new SubstraitType::I32());
        substrait_dtype->mutable_i32()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::UINTEGER:
      case LogicalTypeId::BIGINT: {
        substrait_dtype->set_allocated_i64(new SubstraitType::I64());
        substrait_dtype->mutable_i64()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::UBIGINT:
      case LogicalTypeId::HUGEINT: {
        // FIXME: Support for hugeint types?
        substrait_dtype->set_allocated_decimal(new SubstraitType::Decimal());
        substrait_dtype->mutable_decimal()->set_nullability(nullable);
        substrait_dtype->mutable_decimal()->set_scale(0);
        substrait_dtype->mutable_decimal()->set_precision(38);
        return substrait_dtype;
      }

      case LogicalTypeId::DATE: {
        substrait_dtype->set_allocated_date(new SubstraitType::Date());
        substrait_dtype->mutable_date()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::TIME_TZ:
      case LogicalTypeId::TIME: {
        substrait_dtype->set_allocated_time(new SubstraitType::Time());
        substrait_dtype->mutable_time()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::TIMESTAMP:
      case LogicalTypeId::TIMESTAMP_MS:
      case LogicalTypeId::TIMESTAMP_NS:
      case LogicalTypeId::TIMESTAMP_SEC: {
        substrait_dtype->set_allocated_precision_timestamp(
          new SubstraitType::PrecisionTimestamp()
        );
        substrait_dtype->mutable_precision_timestamp()->set_precision(
          GetTimestampPrecision(type.id())
        );
        substrait_dtype->mutable_precision_timestamp()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::TIMESTAMP_TZ: {
        substrait_dtype->set_allocated_precision_timestamp_tz(
          new SubstraitType::PrecisionTimestampTZ()
        );

        // Timestamp tz is always 'us'
        substrait_dtype->mutable_precision_timestamp_tz()->set_precision(6);
        substrait_dtype->mutable_precision_timestamp_tz()->set_nullability(nullable);

        return substrait_dtype;
      }

      case LogicalTypeId::INTERVAL: {
        substrait_dtype->set_allocated_interval_day(new SubstraitType::IntervalDay());
        substrait_dtype->mutable_interval_day()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::FLOAT: {
        substrait_dtype->set_allocated_fp32(new SubstraitType::FP32());
        substrait_dtype->mutable_fp32()->set_nullability(nullable);
        return substrait_dtype;
      }
      case LogicalTypeId::DOUBLE: {
        substrait_dtype->set_allocated_fp64(new SubstraitType::FP64());
        substrait_dtype->mutable_fp64()->set_nullability(nullable);
        return substrait_dtype;
      }
      case LogicalTypeId::DECIMAL: {
        substrait_dtype->set_allocated_decimal(new SubstraitType::Decimal());

        substrait_dtype->mutable_decimal()->set_nullability(nullable);
        substrait_dtype->mutable_decimal()->set_precision(DuckDecimal::GetWidth(type));
        substrait_dtype->mutable_decimal()->set_scale(DuckDecimal::GetScale(type));

        return substrait_dtype;
      }

      case LogicalTypeId::VARCHAR: {
        substrait_dtype->set_allocated_string(new SubstraitType::String());
        substrait_dtype->mutable_string()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::BLOB: {
        substrait_dtype->set_allocated_binary(new SubstraitType::Binary());
        substrait_dtype->mutable_binary()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::UUID: {
        substrait_dtype->set_allocated_uuid(new SubstraitType::UUID());
        substrait_dtype->mutable_uuid()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::ENUM: {
        substrait_dtype->set_allocated_user_defined(new SubstraitType::UserDefined());
        substrait_dtype->mutable_user_defined()->set_nullability(nullable);
        return substrait_dtype;
      }

      case LogicalTypeId::STRUCT: {
        substrait_dtype->set_allocated_struct_(new SubstraitType::Struct());
        substrait_dtype->mutable_struct_()->set_nullability(nullable);

        // ok lets get the children of our struct
        for (auto& child : duckdb::StructType::GetChildTypes(type)) {
          auto new_type = substrait_dtype->mutable_struct_()->add_types();
          *new_type = *(FromDuckType(child.second, not_null));
        }

        return substrait_dtype;
      }

      default:
        throw duckdb::NotImplementedException(
          "Translation to substrait type not implemented for: " + type.ToString()
        );
    }

    return nullptr;
  }

} // namespace: skytether::engines
