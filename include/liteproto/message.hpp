//
// Created by Youtao Guo on 23/6/23.
//

#pragma once

#include <array>
#include <iostream>
#include <type_traits>

#include "liteproto/list.hpp"
#include "liteproto/type.hpp"
#include "liteproto/utils.hpp"

namespace liteproto {

template <class Msg>
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

#define MESSAGE_DEF_HELPER(msg_name)                  \
  struct MessageDefHelper_##msg_name {                \
    static constexpr int32_t FIELDS_start = __LINE__; \
  };

#define MESSAGE(msg_name)      \
  MESSAGE_DEF_HELPER(msg_name) \
  class msg_name : public liteproto::MessageBase<msg_name>, public MessageDefHelper_##msg_name

#if defined(__clang__) && !defined(LITE_PROTO_COMPATIBLE_MODE_)
#define FIELD(name)                                                                                     \
  name##_;                                                                                              \
                                                                                                        \
 public:                                                                                                \
  constexpr const decltype(name##_)& name() const { return name##_; }                                   \
  decltype(name##_)& mutable_##name() { return name##_; }                                               \
  void set_##name(decltype(name##_) v) { name##_ = std::move(v); }                                      \
  static constexpr decltype(auto) FIELD_name(liteproto::int32_constant<__LINE__>) { return #name; }     \
  auto FIELD_type(liteproto::int32_constant<__LINE__>)->decltype(test_field_);                          \
  constexpr decltype(auto) FIELD_value(liteproto::int32_constant<__LINE__>) { return (name##_); }       \
  constexpr decltype(auto) FIELD_value(liteproto::int32_constant<__LINE__>) const { return (name##_); } \
  template <class Tp>                                                                                   \
  static constexpr int32_t FIELD_seq<__LINE__, Tp>

#define DECLARE_FIELDS                     \
 public:                                   \
  template <int32_t N, class>              \
  static constexpr int32_t FIELD_seq = -1; \
                                           \
 private:
// #elif defined(__GNUC__) || defined(__GNUG__)
#else
#define FIELD(name)                                                                                     \
  name##_;                                                                                              \
                                                                                                        \
 public:                                                                                                \
  constexpr const decltype(name##_)& name() const { return name##_; }                                   \
  decltype(name##_)& mutable_##name() { return name##_; }                                               \
  void set_##name(decltype(name##_) v) { name##_ = std::move(v); }                                      \
  static constexpr decltype(auto) FIELD_name(liteproto::int32_constant<__LINE__>) { return #name; }     \
  auto FIELD_type(liteproto::int32_constant<__LINE__>)->decltype(test_field_);                          \
  constexpr decltype(auto) FIELD_value(liteproto::int32_constant<__LINE__>) { return (name##_); }       \
  constexpr decltype(auto) FIELD_value(liteproto::int32_constant<__LINE__>) const { return (name##_); } \
  static auto FIELD_seq(liteproto::int32_constant<__LINE__>)

using liteproto::Seq;
#endif
