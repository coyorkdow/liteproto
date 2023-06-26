//
// Created by Youtao Guo on 2023/6/24.
//

#pragma once

#include <cstddef>
#include <type_traits>

#include "liteproto/traits.hpp"

using uint8_t = uint8_t;
using int8_t = int8_t;
using uint32_t = uint32_t;
using int32_t = int32_t;
using uint64_t = uint64_t;
using int64_t = int64_t;
using float_32_t = float;
using float_64_t = double;
using size_t = std::size_t;

namespace liteproto {

enum class [[maybe_unused]] Kind : int32_t {
  UINT8,
  INT8,
  UINT32,
  INT32,
  UINT64,
  INT64,
  FLOAT_32,
  FLOAT_64,
  BOOLEAN,

  // Thees are four data structures abstraction that supported by liteproto: list, map, string, array.

  // A list is a flexible-sized sequential container_type. The elements can be accessed by subscript operator (i.e.,
  // operator[]), but no random accessible required. A list also supports push_back and pop_back, and inserts a
  // new element to arbitrary position.
  // A list also supports size(), empty(), and clear().
  LIST,
  // A map is an key-value pairs collection. Each key must be unique in a map, and the value can be looked up via the
  // corresponding key. A map uses get() to look up and update() to modify. A map also supports size(), empty(), and
  // clear().
  MAP,
  // A string is almost the same as a list, except it doesn't be regarded as a container_type. A string is represented
  // by a single value, like "123" or "abc". Since the String is not a container_type, the underlying type cannot be
  // further inspected. A string supports all the operators that list supports.
  STRING,
  // An array is a fixed-sized sequential container_type. The elements can be accessed by subscript operator (i.e.,
  // operator[]). It doesn't support any other way to look up or modify. An array also supports size().
  ARRAY
};

enum class Type : int8_t { SCALAR, POINTER, LIST, MAP, STRING, ARRAY, MESSAGE };

template <class Tp>
struct TypeMeta {
  static uint64_t Id() {
    static uint64_t alloc_addr = 0;
    uint64_t* addr = &alloc_addr;
    return *reinterpret_cast<uint64_t*>(&addr);
  }
};

enum class traits : uint8_t {
  // primary type categories
  is_null_pointer,
  is_integral,
  is_floating_point,
  is_array,
  is_pointer,
  is_lvalue_reference,
  is_rvalue_reference,
  is_member_object_pointer,
  is_member_function_pointer,
  is_enum,
  is_union,
  is_class,
  is_function,

  // composite type categories
  is_reference,
  is_arithmetic,
  is_fundamental,
  is_object,
  is_scalar,
  is_compound,
  is_member_pointer,

  // type properties
  is_const,
  is_volatile,
  is_trivial,
  is_trivially_copyable,
  is_standard_layout,
  is_empty,
  is_polymorphic,
  is_abstract,
  is_final,
  is_aggregate,

  is_signed,
  is_unsigned,

  is_default_constructible,
  is_copy_constructible,
  is_move_constructible,

  is_copy_assignable,
  is_move_assignable,

  is_swappable,

  is_destructible,

  is_trivially_default_constructible,
  is_trivially_copy_constructible,
  is_trivially_move_constructible,

  is_trivially_copy_assignable,
  is_trivially_move_assignable,
  is_trivially_destructible,

  is_nothrow_default_constructible,
  is_nothrow_copy_constructible,
  is_nothrow_move_constructible,

  is_nothrow_copy_assignable,
  is_nothrow_move_assignable,

  is_nothrow_swappable,

  is_nothrow_destructible,

  has_virtual_destructor,

  has_unique_object_representations

};

enum class transform : uint8_t {
  remove_cv,
  remove_const,
  remove_volatile,
  remove_reference,
  remove_pointer,
  remove_extent,
  remove_all_extents,
  remove_cvref
};

class TypeDescriptor {
 public:
  uint64_t Id() const noexcept;
  bool Traits(traits t) const noexcept;
  TypeDescriptor Transform(transform t) const noexcept;

  size_t alignment_of() const noexcept;
  size_t rank() const noexcept;
  size_t extent() const noexcept;

 private:
};

namespace internal {

template <class Tp>
struct TypeMeta {
  static uint64_t Id() {
    static uint64_t alloc_addr = 0;
    uint64_t* addr = &alloc_addr;
    return *reinterpret_cast<uint64_t*>(&addr);
  }

  static constexpr bool traits(traits t) {
#define LITE_PROTO_TRAITS_CASE_(name) \
  case traits::name:                  \
    return std::name##_v<Tp>

    switch (t) {
      LITE_PROTO_TRAITS_CASE_(is_null_pointer);
      LITE_PROTO_TRAITS_CASE_(is_integral);
      LITE_PROTO_TRAITS_CASE_(is_floating_point);
      LITE_PROTO_TRAITS_CASE_(is_array);
      LITE_PROTO_TRAITS_CASE_(is_pointer);
      LITE_PROTO_TRAITS_CASE_(is_lvalue_reference);
      LITE_PROTO_TRAITS_CASE_(is_rvalue_reference);
      LITE_PROTO_TRAITS_CASE_(is_member_object_pointer);
      LITE_PROTO_TRAITS_CASE_(is_member_function_pointer);
      LITE_PROTO_TRAITS_CASE_(is_enum);
      LITE_PROTO_TRAITS_CASE_(is_union);
      LITE_PROTO_TRAITS_CASE_(is_class);
      LITE_PROTO_TRAITS_CASE_(is_function);
      LITE_PROTO_TRAITS_CASE_(is_reference);
      LITE_PROTO_TRAITS_CASE_(is_arithmetic);
      LITE_PROTO_TRAITS_CASE_(is_fundamental);
      LITE_PROTO_TRAITS_CASE_(is_object);
      LITE_PROTO_TRAITS_CASE_(is_scalar);
      LITE_PROTO_TRAITS_CASE_(is_compound);
      LITE_PROTO_TRAITS_CASE_(is_member_pointer);
      LITE_PROTO_TRAITS_CASE_(is_const);
      LITE_PROTO_TRAITS_CASE_(is_volatile);
      LITE_PROTO_TRAITS_CASE_(is_trivial);
      LITE_PROTO_TRAITS_CASE_(is_trivially_copyable);
      LITE_PROTO_TRAITS_CASE_(is_standard_layout);
      LITE_PROTO_TRAITS_CASE_(is_empty);
      LITE_PROTO_TRAITS_CASE_(is_polymorphic);
      LITE_PROTO_TRAITS_CASE_(is_abstract);
      LITE_PROTO_TRAITS_CASE_(is_final);
      LITE_PROTO_TRAITS_CASE_(is_aggregate);
      LITE_PROTO_TRAITS_CASE_(is_signed);
      LITE_PROTO_TRAITS_CASE_(is_unsigned);
      LITE_PROTO_TRAITS_CASE_(is_default_constructible);
      LITE_PROTO_TRAITS_CASE_(is_copy_constructible);
      LITE_PROTO_TRAITS_CASE_(is_move_constructible);
      LITE_PROTO_TRAITS_CASE_(is_copy_assignable);
      LITE_PROTO_TRAITS_CASE_(is_move_assignable);
      LITE_PROTO_TRAITS_CASE_(is_swappable);
      LITE_PROTO_TRAITS_CASE_(is_destructible);
      LITE_PROTO_TRAITS_CASE_(is_trivially_default_constructible);
      LITE_PROTO_TRAITS_CASE_(is_trivially_copy_constructible);
      LITE_PROTO_TRAITS_CASE_(is_trivially_move_constructible);
      LITE_PROTO_TRAITS_CASE_(is_trivially_copy_assignable);
      LITE_PROTO_TRAITS_CASE_(is_trivially_move_assignable);
      LITE_PROTO_TRAITS_CASE_(is_trivially_destructible);
      LITE_PROTO_TRAITS_CASE_(is_nothrow_default_constructible);
      LITE_PROTO_TRAITS_CASE_(is_nothrow_copy_constructible);
      LITE_PROTO_TRAITS_CASE_(is_nothrow_move_constructible);
      LITE_PROTO_TRAITS_CASE_(is_nothrow_copy_assignable);
      LITE_PROTO_TRAITS_CASE_(is_nothrow_move_assignable);
      LITE_PROTO_TRAITS_CASE_(is_nothrow_swappable);
      LITE_PROTO_TRAITS_CASE_(is_nothrow_destructible);
      LITE_PROTO_TRAITS_CASE_(has_virtual_destructor);
      LITE_PROTO_TRAITS_CASE_(has_unique_object_representations);
#undef LITE_PROTO_TRAITS_CASE_
    }
  }

  static constexpr bool IsList() { return IsListV<Tp>; }
};

}  // namespace internal

enum class ConstOption : bool { NON_CONST = false, CONST = true };

}  // namespace liteproto