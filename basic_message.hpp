//
// Created by Youtao Guo on 23/6/23.
//

#pragma once

#include <array>
#include <iostream>
#include <type_traits>

#include "utils.hpp"

template <class Msg>
class MessageBasic {
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

  struct FieldsIndices {
    static constexpr auto value = ::GetAllFields<Msg>();
  };

 private:
  template <size_t... I>
  [[nodiscard]] constexpr auto DumpTupleImpl(std::index_sequence<I...>) const {
    auto msg = static_cast<const Msg&>(*this);
    constexpr auto indices = FieldsIndices::value;
    return std::make_tuple(msg.FIELD_value(int32_constant<indices[I].second>{})...);
  }
};

#define MESSAGE_DEF_HELPER(msg_name)                  \
  struct MessageDefHelper_##msg_name {                \
    static constexpr int32_t FIELDS_start = __LINE__; \
  };

#define MESSAGE(msg_name)      \
  MESSAGE_DEF_HELPER(msg_name) \
  class msg_name : public MessageBasic<msg_name>, public MessageDefHelper_##msg_name
