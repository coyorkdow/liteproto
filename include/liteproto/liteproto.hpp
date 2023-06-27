//
// Created by Youtao Guo on 2023/6/27.
//

#pragma once

#include "message.hpp"
#include "pair.hpp"
#include "reflect.hpp"

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
