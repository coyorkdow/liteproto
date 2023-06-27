//
// Created by Youtao Guo on 2023/6/24.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "liteproto/traits.hpp"

using uint8_t = std::uint8_t;
using int8_t = std::int8_t;
using uint32_t = std::uint32_t;
using int32_t = std::int32_t;
using uint64_t = std::uint64_t;
using int64_t = std::int64_t;
using size_t = std::size_t;

#if !defined(__cpp_rtti)
#warning The macro __cpp_rtti has not been defiend!
#endif
#define LITE_PROTO_ENABLE_RTTI_ __cpp_rtti

namespace liteproto {

enum class Kind : int32_t {
  UINT8,
  INT8,
  UINT32,
  INT32,
  UINT64,
  INT64,
  FLOAT_32,
  FLOAT_64,
  BOOLEAN,

  SCALAR,
  ARRAY,

  STD_VECTOR,
  STD_MAP,
  STD_STRING,
  STD_ARRAY,

  OTHER
};

// Thees are four data structures abstraction that supported by liteproto: list, map, string, array.

// A list is a flexible-sized sequential container_type. The elements can be accessed by subscript operator (i.e.,
// operator[]), but no random accessible required. A list also supports push_back and pop_back, and inserts a
// new element to arbitrary position.
// A list also supports size(), empty(), and clear().

// A map is an key-value pairs collection. Each key must be unique in a map, and the value can be looked up via the
// corresponding key. A map uses get() to look up and update() to modify. A map also supports size(), empty(), and
// clear().

// A string is almost the same as a list, except it doesn't be regarded as a container_type. A string is represented
// by a single value, like "123" or "abc". Since the String is not a container_type, the underlying type cannot be
// further inspected. A string supports all the operators that list supports.

// An array is a fixed-sized sequential container_type. The elements can be accessed by subscript operator (i.e.,
// operator[]). It doesn't support any other way to look up or modify. An array also supports size().

enum class Type : int8_t { SCALAR, POINTER, LIST, MAP, STRING, ARRAY, MESSAGE, OTHER };

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
  is_void,
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
  remove_const,
  remove_volatile,
  remove_reference,
  remove_pointer,
  remove_extent,
  remove_all_extents,
};

class TypeDescriptor;

namespace internal {

struct DescriptorInterface {
  using id_t = uint64_t();
  using kinde_enum_t = Kind();
  using type_enum_t = Type();
  using traits_t = bool(traits);
  using transform_t = TypeDescriptor(transform);
  using size_of_t = size_t();
  using alignment_of_t = size_t();
  using rank_t = size_t();
  using extent_t = size_t();

  using value_type_t = TypeDescriptor();
  using kv_type_t = std::pair<TypeDescriptor, TypeDescriptor>();

  id_t* id_;
  kinde_enum_t* kind_;
  type_enum_t* type_enum_;
  traits_t* traits_;
  transform_t* transform_;
  size_of_t* size_of_;
  alignment_of_t* alignment_of_;
  rank_t* rank_;
  extent_t* extent_;

  value_type_t* value_type_;
  kv_type_t* kv_type_;
};

template <class Tp>
struct TypeMeta;

}  // namespace internal

class TypeDescriptor {
  template <class Tp>
  friend class internal::TypeMeta;

 public:
  uint64_t Id() const noexcept { return inter_.id_(); }
  Kind KindEnum() const noexcept { return inter_.kind_(); }
  Type TypeEnum() const noexcept { return inter_.type_enum_(); }
  bool Traits(traits t) const noexcept { return inter_.traits_(t); }
  TypeDescriptor Transform(transform t) const noexcept { return inter_.transform_(t); }

  size_t SizeOf() const noexcept { return inter_.size_of_(); }
  size_t AlignmentOf() const noexcept { return inter_.alignment_of_(); }
  size_t Rank() const noexcept { return inter_.rank_(); }
  size_t Extent() const noexcept { return inter_.extent_(); }

  TypeDescriptor ValueType() const noexcept { return inter_.value_type_(); }
  std::pair<TypeDescriptor, TypeDescriptor> KVType() const noexcept { return inter_.kv_type_(); }

  bool operator==(const TypeDescriptor& rhs) const { return Id() == rhs.Id(); }
  bool operator!=(const TypeDescriptor& rhs) const { return Id() != rhs.Id(); }

 private:
  explicit constexpr TypeDescriptor(internal::DescriptorInterface inter) : inter_(inter) {}
  internal::DescriptorInterface inter_;
};

namespace internal {

template <class Tp>
struct TypeMeta {
  static TypeDescriptor MakeInterface() noexcept {
    DescriptorInterface inter{};
    inter.id_ = &Id;
    inter.size_of_ = &SizeOf;
    inter.alignment_of_ = &AlignmentOf;
    inter.rank_ = &Rank;
    inter.extent_ = &Extent;
    inter.transform_ = &Transform;
    inter.type_enum_ = &TypeEnum;
    inter.traits_ = &Traits;
    inter.transform_ = &Transform;

//    inter.value_type_ = &ValueType;
//    inter.kv_type_ = &KVType;

    return TypeDescriptor{inter};
  }

  static uint64_t Id() noexcept {
    static uint64_t alloc_addr = 0;
    uint64_t* addr = &alloc_addr;
    return *reinterpret_cast<uint64_t*>(&addr);
  }

  static constexpr size_t SizeOf() noexcept { return sizeof(Tp); }
  static constexpr size_t AlignmentOf() noexcept { return std::alignment_of_v<Tp>; }
  //  If T is an array type, provides the member constant value equal to the number of dimensions of the array. For any
  //  other type, value is 0.
  static constexpr size_t Rank() noexcept { return std::rank_v<Tp>; }
  // f T is an array type, provides the member constant value equal to the number of elements along the Nth dimension of
  // the array, if N is in [0, std::rank<T>::value). For any other type, or if T is an array of unknown bound
  // along its first dimension and N is 0, value is 0.
  static constexpr size_t Extent() noexcept { return std::extent_v<Tp>; }

  static TypeDescriptor Transform(transform t) noexcept;

  static TypeDescriptor remove_const() noexcept { using next_meta = TypeMeta<std::remove_const_t<Tp>>; }
  static TypeDescriptor remove_volatile() noexcept { using next_meta = TypeMeta<std::remove_volatile_t<Tp>>; }
  static TypeDescriptor remove_reference() noexcept { using next_meta = TypeMeta<std::remove_reference_t<Tp>>; }
  static TypeDescriptor remove_pointer() noexcept { using next_meta = TypeMeta<std::remove_pointer_t<Tp>>; }
  static TypeDescriptor remove_extent() noexcept { using next_meta = TypeMeta<std::remove_extent_t<Tp>>; }
  static TypeDescriptor remove_all_extents() noexcept { using next_meta = TypeMeta<std::remove_all_extents_t<Tp>>; }

  static constexpr TypeDescriptor ValueType() noexcept;

  static constexpr std::pair<TypeDescriptor, TypeDescriptor> KVType();

  static constexpr Type TypeEnum() {
    if constexpr (std::is_pointer_v<Tp>) {
      return Type::POINTER;
    } else if constexpr (std::is_scalar_v<Tp>) {
      return Type::SCALAR;
    } else if constexpr (std::is_array_v<Tp>) {
      return Type::ARRAY;
    } else if constexpr (IsListV<Tp>) {
      return Type::LIST;
    }

    return Type::OTHER;
  }

  static constexpr Kind KindEnum() noexcept {
    if constexpr (std::is_same_v<Tp, uint8_t>) {
      return Kind::UINT8;
    } else if constexpr (std::is_same_v<Tp, int8_t>) {
      return Kind::INT8;
    } else if constexpr (std::is_same_v<Tp, uint32_t>) {
      return Kind::UINT32;
    } else if constexpr (std::is_same_v<Tp, int32_t>) {
      return Kind::INT32;
    } else if constexpr (std::is_same_v<Tp, uint64_t>) {
      return Kind::UINT64;
    } else if constexpr (std::is_same_v<Tp, int64_t>) {
      return Kind::INT64;
    } else if constexpr (std::is_same_v<Tp, float>) {
      return Kind::FLOAT_32;
    } else if constexpr (std::is_same_v<Tp, double>) {
      return Kind::FLOAT_64;
    } else if constexpr (std::is_scalar_v<Tp>) {
      return Kind::SCALAR;
    } else if constexpr (std::is_array_v<Tp>) {
      return Kind::ARRAY;
    }
  }

  static constexpr bool Traits(traits t) noexcept {
#define LITE_PROTO_TRAITS_CASE_(name) \
  case traits::name:                  \
    return std::name##_v<Tp>

    switch (t) {
      LITE_PROTO_TRAITS_CASE_(is_void);
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