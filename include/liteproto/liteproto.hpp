//
// Created by Youtao Guo on 2023/6/27.
//

#pragma once

#include "liteproto/message.hpp"
#include "liteproto/reflect.hpp"
#include "liteproto/serialize/resolver.hpp"

#define MESSAGE(msg_name) class msg_name : public liteproto::MessageBase<msg_name, __LINE__>

#define $(...) __VA_ARGS__

#define TEMPLATE_MESSAGE(msg_name, arg) class msg_name : public liteproto::MessageBase<msg_name<arg>, __LINE__>

#define LITE_PROTO_FIELD_DECLARE_BASE_(name)                                                        \
  name##_;                                                                                          \
                                                                                                    \
 public:                                                                                            \
  constexpr const decltype(name##_)& name() const { return name##_; }                               \
  decltype(name##_)& mutable_##name() { return name##_; }                                           \
  void set_##name(const decltype(name##_)& v) { name##_ = std::move(v); }                           \
  void set_##name(decltype(name##_)&& v) { name##_ = std::move(v); }                                \
  static constexpr decltype(auto) FIELD_name(liteproto::int32_constant<__LINE__>) { return #name; } \
  auto FIELD_type(liteproto::int32_constant<__LINE__>)->decltype(name##_);                          \
  constexpr decltype(name##_)& FIELD_value(liteproto::int32_constant<__LINE__>) { return name##_; } \
  constexpr const decltype(name##_)& FIELD_value(liteproto::int32_constant<__LINE__>) const { return name##_; }

#if defined(LITE_PROTO_DISABLE_COMPATIBLE_MODE_)
#define FIELD(name)                    \
  LITE_PROTO_FIELD_DECLARE_BASE_(name) \
  template <class Tp>                  \
  static constexpr int32_t FIELD_seq<__LINE__, Tp>

#define DECLARE_FIELDS()      \
 public:                      \
  template <int32_t N, class> \
  static constexpr int32_t FIELD_seq = -1;

// #elif defined(__GNUC__) || defined(__GNUG__)
#else
#define FIELD(name)                    \
  LITE_PROTO_FIELD_DECLARE_BASE_(name) \
  static auto FIELD_seq(liteproto::int32_constant<__LINE__>)

using liteproto::Seq;
#define DECLARE_FIELDS()
#endif
