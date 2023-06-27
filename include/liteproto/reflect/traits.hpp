//
// Created by Youtao Guo on 2023/6/27.
//

#pragma once

#include "liteproto/reflect/type.hpp"
#include "liteproto/traits.hpp"

namespace liteproto {

template <class Tp>  // All Strings are immediate types.
struct IsImmediateType : std::bool_constant<IsStringV<Tp>> {};

template <>
struct IsImmediateType<uint8_t> : std::true_type {};

template <>
struct IsImmediateType<int8_t> : std::true_type {};

template <>
struct IsImmediateType<uint32_t> : std::true_type {};

template <>
struct IsImmediateType<int32_t> : std::true_type {};

template <>
struct IsImmediateType<uint64_t> : std::true_type {};

template <>
struct IsImmediateType<int64_t> : std::true_type {};

template <>
struct IsImmediateType<float> : std::true_type {};

template <>
struct IsImmediateType<double> : std::true_type {};

template <>
struct IsImmediateType<void*> : std::true_type {};

template <class Tp>
struct IsImmediateType<const Tp> : IsImmediateType<Tp> {};

template <class Tp>
struct IsImmediateType<volatile Tp> : IsImmediateType<Tp> {};

template <class Tp>
struct IsImmediateType<const volatile Tp> : IsImmediateType<Tp> {};

template <class Tp>
struct IsImmediateType<Tp*> : IsImmediateType<Tp> {};

template <class Tp>
struct IsImmediateType<Tp&> : IsImmediateType<Tp> {};

template <class Tp>
struct IsImmediateType<Tp&&> : IsImmediateType<Tp> {};

}  // namespace liteproto