//
// Created by Youtao Guo on 23/6/23.
//

#pragma once

#include <array>
#include <iostream>
#include <type_traits>

#include "liteproto/list.hpp"
#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/type.hpp"
#include "liteproto/utils.hpp"

namespace liteproto {

template <class Msg, int32_t Line>
class MessageBase {
 public:
  template <size_t I = 0, size_t N>
  static constexpr auto GetFieldIndexByName(const char (&str)[N]) {
    constexpr auto indices = FieldsIndices::value;
    if constexpr (I >= indices.size()) {
      return PII{-1, -1};
    } else if (StrLiteralEQ(str, Msg::FIELD_name(int32_constant<indices[I].second>{}))) {
      return indices[I];
    } else {
      return GetFieldIndexByName<I + 1>(str);
    }
  }

  [[nodiscard]] constexpr auto DumpTuple() const {
    return DumpTupleImpl(std::make_index_sequence<FieldsIndices::value.size()>{});
  }

  static constexpr int32_t FIELDS_start = Line;

  struct FieldsIndices {
#if defined(__clang__) && !defined(LITE_PROTO_COMPATIBLE_MODE_)
    static constexpr auto value = GetAllFields<Msg>();
// #elif defined(__GNUC__) || defined(__GNUG__)
#else
    static constexpr auto value = GetAllFields2<Msg>();
#endif
  };

 private:
  template <size_t... I>
  [[nodiscard]] constexpr auto DumpTupleImpl(std::index_sequence<I...>) const {
    auto msg = static_cast<const Msg&>(*this);
    constexpr auto indices = FieldsIndices::value;
    return std::make_tuple(msg.FIELD_value(int32_constant<indices[I].second>{})...);
  }
};

}  // namespace liteproto
