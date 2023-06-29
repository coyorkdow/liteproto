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

enum class Type : int32_t {
  VOID,
  UINT8,
  INT8,
  UINT32,
  INT32,
  UINT64,
  INT64,
  FLOAT_32,
  FLOAT_64,
  BOOLEAN,

  OBJECT,
  MESSAGE,

  STD_VECTOR,
  STD_DEQUE,
  STD_LIST,
  STD_MAP,
  STD_UNORDERED_MAP,
  STD_STRING,
  STD_ARRAY,

  STD_ANY,

  OTHER
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
  using id_t = uint64_t() noexcept;
  using kinde_enum_t = Kind() noexcept;
  using type_enum_t = Type() noexcept;
  using traits_t = bool(traits) noexcept;
  using transform_t = TypeDescriptor(transform) noexcept;
  using size_of_t = size_t() noexcept;
  using alignment_of_t = size_t() noexcept;
  using rank_t = size_t() noexcept;
  using extent_t = size_t() noexcept;

  using value_type_t = TypeDescriptor() noexcept;

  using is_indirect_type_t = bool() noexcept;
  using default_value = std::pair<Object, std::any>() noexcept;

  id_t* id_;
  kinde_enum_t* kind_enum_;
  type_enum_t* type_enum_;
  traits_t* traits_;
  transform_t* transform_;
  size_of_t* size_of_;
  alignment_of_t* alignment_of_;
  rank_t* rank_;
  extent_t* extent_;

  value_type_t* value_type_;

  is_indirect_type_t* is_indirect_type_;
  default_value* default_value_;
};

}  // namespace internal

class TypeDescriptor {
  template <class Tp>
  friend struct TypeMeta;

  friend class Object;

 public:
  [[nodiscard]] uint64_t Id() const noexcept { return inter_.id_(); }
  [[nodiscard]] Kind KindEnum() const noexcept { return inter_.kind_enum_(); }
  [[nodiscard]] Type TypeEnum() const noexcept { return inter_.type_enum_(); }
  [[nodiscard]] bool Traits(traits t) const noexcept { return inter_.traits_(t); }
  [[nodiscard]] TypeDescriptor Transform(transform t) const noexcept { return inter_.transform_(t); }

  [[nodiscard]] bool IsIndirectType() const noexcept { return inter_.is_indirect_type_(); }
  [[nodiscard]] inline std::pair<Object, std::any> DefaultValue() const noexcept;

  [[nodiscard]] size_t SizeOf() const noexcept { return inter_.size_of_(); }
  [[nodiscard]] size_t AlignmentOf() const noexcept { return inter_.alignment_of_(); }
  [[nodiscard]] size_t Rank() const noexcept { return inter_.rank_(); }
  [[nodiscard]] size_t Extent() const noexcept { return inter_.extent_(); }

  [[nodiscard]] TypeDescriptor ValueType() const noexcept { return inter_.value_type_(); }

  bool operator==(const TypeDescriptor& rhs) const { return Id() == rhs.Id(); }
  bool operator!=(const TypeDescriptor& rhs) const { return Id() != rhs.Id(); }

 private:
  TypeDescriptor() = default;

  explicit constexpr TypeDescriptor(const internal::DescriptorInterface& inter) : inter_(inter) {}
  internal::DescriptorInterface inter_;
};

template <class Tp>
struct TypeMeta {
  static inline TypeDescriptor MakeDescriptor() noexcept;
  static inline std::pair<Object, std::any> DefaultValue() noexcept;

  static constexpr bool IsIndirectType() noexcept { return IsIndirectTypeV<Tp>; }

  static uint64_t Id() noexcept {
    static uint64_t alloc_addr = 0;
    auto addr_val = &alloc_addr;
    uint64_t id = 0;
    std::memcpy(&id, &addr_val, sizeof addr_val);
    return id;
  }

  static constexpr size_t SizeOf() noexcept {
    if constexpr (!std::is_void_v<Tp>) {
      return sizeof(Tp);
    }
    return 0;
  }
  static constexpr size_t AlignmentOf() noexcept {
    if constexpr (!std::is_void_v<Tp>) {
      return std::alignment_of_v<Tp>;
    }
    return 0;
  }
  //  If T is an array type, provides the member constant value equal to the number of dimensions of the array. For any
  //  other type, value is 0.
  static constexpr size_t Rank() noexcept { return std::rank_v<Tp>; }
  // f T is an array type, provides the member constant value equal to the number of elements along the Nth dimension of
  // the array, if N is in [0, std::rank<T>::value). For any other type, or if T is an array of unknown bound
  // along its first dimension and N is 0, value is 0.
  static constexpr size_t Extent() noexcept { return std::extent_v<Tp>; }

  static constexpr TypeDescriptor Transform(transform t) noexcept {
    switch (t) {
      case transform::remove_const:
        return remove_const();
      case transform::remove_volatile:
        return remove_volatile();
      case transform::remove_reference:
        return remove_reference();
      case transform::remove_pointer:
        return remove_pointer();
      case transform::remove_extent:
        return remove_extent();
      case transform::remove_all_extents:
        return remove_all_extents();
    }
    return TypeMeta<void>::MakeDescriptor();
  }

  static constexpr TypeDescriptor remove_const() noexcept {
    using next_meta = TypeMeta<std::remove_const_t<Tp>>;
    return next_meta::MakeDescriptor();
  }
  static constexpr TypeDescriptor remove_volatile() noexcept {
    using next_meta = TypeMeta<std::remove_volatile_t<Tp>>;
    return next_meta::MakeDescriptor();
  }
  static constexpr TypeDescriptor remove_reference() noexcept {
    using next_meta = TypeMeta<std::remove_reference_t<Tp>>;
    return next_meta::MakeDescriptor();
  }
  static constexpr TypeDescriptor remove_pointer() noexcept {
    using next_meta = TypeMeta<std::remove_pointer_t<Tp>>;
    return next_meta::MakeDescriptor();
  }
  static constexpr TypeDescriptor remove_extent() noexcept {
    using next_meta = TypeMeta<std::remove_extent_t<Tp>>;
    return next_meta::MakeDescriptor();
  }
  static constexpr TypeDescriptor remove_all_extents() noexcept {
    using next_meta = TypeMeta<std::remove_all_extents_t<Tp>>;
    return next_meta::MakeDescriptor();
  }

  static constexpr TypeDescriptor ValueType() noexcept {
    if constexpr (std::is_pointer_v<Tp>) {
      return remove_pointer();
    } else if constexpr (std::is_reference_v<Tp>) {
      return remove_reference();
    } else if constexpr (std::is_array_v<Tp>) {
      return remove_extent();
    } else if constexpr (IsSmartPtrV<Tp>) {
      return TypeMeta<typename SmartPtrTraits<Tp>::value_type>::MakeDescriptor();
    } else if constexpr (IsListV<Tp>) {
      using traits = ListTraits<Tp>;
      return TypeMeta<typename traits::value_type>::MakeDescriptor();
    } else if constexpr (IsArrayV<Tp>) {
      using traits = ArrayTraits<Tp>;
      return TypeMeta<typename traits::value_type>::MakeDescriptor();
    } else if constexpr (IsPairV<Tp>) {
      using traits = PairTraits<Tp>;
      return TypeMeta<typename traits::value_type>::MakeDescriptor();
    }

    return TypeMeta<void>::MakeDescriptor();
  }

  static constexpr Kind KindEnum() noexcept {
    if constexpr (IsObjectV<Tp>) {
      return Kind::OBJECT;
    } else if constexpr (std::is_void_v<Tp>) {
      return Kind::VOID;
    } else if constexpr (std::is_scalar_v<Tp>) {
      static_assert(std::is_scalar_v<std::nullptr_t>);
      return Kind::SCALAR;
    } else if constexpr (std::is_reference_v<Tp>) {
      return Kind::REFERENCE;
    } else if constexpr (std::is_function_v<Tp>) {
      return Kind::FUNCTION;
    } else if constexpr (IsSmartPtrV<Tp>) {
      return Kind::SMART_PTR;
    } else if constexpr (IsStringV<Tp>) {
      return Kind::STRING;
    } else if constexpr (IsListV<Tp>) {
      return Kind::LIST;
    } else if constexpr (IsArrayV<Tp>) {
      return Kind::ARRAY;
    } else if constexpr (IsPairV<Tp>) {
      return Kind::PAIR;
    } else if constexpr (std::is_class_v<Tp>) {
      return Kind::CLASS;
    } else {
      static_assert(std::is_union_v<Tp>);
      return Kind::UNION;
    }
  }

  static constexpr Type TypeEnum() noexcept {
    if constexpr (std::is_same_v<Tp, Object>) {
      return Type::OBJECT;
    } else if constexpr (std::is_same_v<Tp, uint8_t>) {
      return Type::UINT8;
    } else if constexpr (std::is_same_v<Tp, int8_t>) {
      return Type::INT8;
    } else if constexpr (std::is_same_v<Tp, uint32_t>) {
      return Type::UINT32;
    } else if constexpr (std::is_same_v<Tp, int32_t>) {
      return Type::INT32;
    } else if constexpr (std::is_same_v<Tp, uint64_t>) {
      return Type::UINT64;
    } else if constexpr (std::is_same_v<Tp, int64_t>) {
      return Type::INT64;
    } else if constexpr (std::is_same_v<Tp, float>) {
      return Type::FLOAT_32;
    } else if constexpr (std::is_same_v<Tp, double>) {
      return Type::FLOAT_64;
    } else if constexpr (std::is_void_v<Tp>) {
      return Type::VOID;
    } else if constexpr (std::is_same_v<Tp, std::string>) {
      return Type::STD_STRING;
    } else if constexpr (IsSTDvectorV<Tp>) {
      return Type::STD_VECTOR;
    } else if constexpr (IsSTDdequeV<Tp>) {
      return Type::STD_DEQUE;
    } else if constexpr (IsSTDlistV<Tp>) {
      return Type::STD_LIST;
    } else if constexpr (IsSTDmapV<Tp>) {
      return Type::STD_MAP;
    } else if constexpr (IsSTDunordered_mapV<Tp>) {
      return Type::STD_UNORDERED_MAP;
    } else if constexpr (std::is_same_v<Tp, std::any>) {
      return Type::STD_ANY;
    }

    return Type::OTHER;
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
    return true;
  }
};

enum class ConstOption : bool { NON_CONST = false, CONST = true };

}  // namespace liteproto