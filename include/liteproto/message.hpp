//
// Created by Youtao Guo on 23/6/23.
//

#pragma once

#include <array>
#include <iostream>
#include <map>
#include <type_traits>
#include <utility>

#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/type.hpp"
#include "liteproto/utils.hpp"

namespace liteproto {

class Message {
 public:
 private:
};

template <class Msg, int32_t Line>
class MessageBase : public Message {
 public:
  template <size_t I = 0, size_t N>
  static constexpr auto GetFieldIndexByName(const char (&str)[N]) {
    constexpr auto indices = FieldsIndices::value;
    if constexpr (I >= indices.size()) {
      return internal::PII{-1, -1};
    } else if (StrLiteralEQ(str, Msg::FIELD_name(int32_constant<indices[I].second>{}))) {
      return indices[I];
    } else {
      return GetFieldIndexByName<I + 1>(str);
    }
  }

  [[nodiscard]] auto DumpTuple() const {
    return DumpTupleImpl(std::make_index_sequence<FieldsIndices::value.size()>{});
  }

  [[nodiscard]] auto DumpForwardTuple() {
    return DumpForwardTupleImpl(std::make_index_sequence<FieldsIndices::value.size()>{});
  }

  static constexpr int32_t FIELDS_start = Line;

  struct FieldsIndices {
#if defined(__clang__) && defined(LITE_PROTO_DISABLE_COMPATIBLE_MODE_)
    static constexpr auto value = internal::GetAllFields<Msg>();
// #elif defined(__GNUC__) || defined(__GNUG__)
#else
    static constexpr auto value = internal::GetAllFields2<Msg>();
#endif
  };

  static const std::map<int32_t, const char*>& FieldsName() noexcept {
    static const std::map<int32_t, const char*> fields_name_ =
        MakeDynamicalFieldsName(std::make_index_sequence<FieldsIndices::value.size()>{});
    return fields_name_;
  }

 private:
  template <size_t... I>
  [[nodiscard]] constexpr auto DumpTupleImpl(std::index_sequence<I...>) const {
    auto& msg = static_cast<const Msg&>(*this);
    constexpr auto indices = FieldsIndices::value;
    return std::make_tuple(msg.FIELD_value(int32_constant<indices[I].second>{})...);
  }

  template <size_t... I>
  [[nodiscard]] auto DumpForwardTupleImpl(std::index_sequence<I...>) {
    auto& msg = static_cast<Msg&>(*this);
    constexpr auto indices = FieldsIndices::value;
    return std::forward_as_tuple(msg.FIELD_value(int32_constant<indices[I].second>{})...);
  }

  template <size_t... I>
  static auto MakeDynamicalFieldsName(std::index_sequence<I...>) noexcept {
    std::map<int32_t, const char*> fields_name;
    (GetNameForEachField<I>(&fields_name), ...);
    return fields_name;
  }

  template <size_t I>
  static void GetNameForEachField(std::map<int32_t, const char*>* field_name) noexcept {
    constexpr auto index = FieldsIndices::value[I];
    (*field_name)[index.first] = Msg::FIELD_name(int32_constant<index.second>{});
  }

  template <size_t I, class Msg_>
  static void ApplyReflectForEachField(Msg_&& msg, std::map<int32_t, Object>* field) {
    constexpr auto index = FieldsIndices::value[I];
    field->insert(std::make_pair(index.first, GetReflection(&msg.FIELD_value(int32_constant<index.second>{}))));
  }
};

}  // namespace liteproto
