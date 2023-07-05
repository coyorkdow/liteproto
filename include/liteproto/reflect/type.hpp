//
// Created by Youtao Guo on 2023/6/24.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "liteproto/traits/traits.hpp"

#if !defined(__cpp_rtti)
#warning The macro __cpp_rtti has not been defiend!
#endif
#define LITE_PROTO_ENABLE_RTTI_ __cpp_rtti

namespace liteproto {

// Thees are six abstract interfaces that supported by liteproto: Number, Message, List, Map, String, Array.

// A List is a flexible-sized sequential container_type. The elements can be accessed by subscript operator (i.e.,
// operator[]), but no random accessible required. A List also supports push_back and pop_back, and inserts a
// new element to arbitrary position.
// A List also supports size(), empty(), and clear().

// A Map is an key-value pairs collection. Each key must be unique in a map, and the value can be looked up via the
// corresponding key. A Map also supports size(), empty(), and clear().

// A String is almost the same as a list, In fact it offers all the operations that offered by List. A String is
// represented by a single value, like "123" or "abc". Since the String is not a container_type, the underlying type
// cannot be further inspected.

// An Array is a fixed-sized sequential container_type. The elements can be accessed by subscript operator (i.e.,
// operator[]). It doesn't support any other way to look up or modify. An Array also supports size().

// In cpp, object type is a category of types. object types are (possibly cv-qualified) types that are not function
// types, reference types, or possibly cv-qualified void. scalar types are (possibly cv-qualified) object types that are
// not array types or class types. But in liteproto, there is a different Object (we can differentiate them by
// upper/lower case), which represents the base of all liteproto types.
enum class Kind : int8_t {
  VOID,
  NUMBER,
  NUMBER_REFERENCE,
  CHAR,
  OTHER_SCALAR,
  REFERENCE,
  FUNCTION,
  OBJECT,  // it means the Object in liteproto, not the std::is_object.
  MESSAGE,
  SMART_PTR,
  LIST,
  MAP,
  STRING,
  ARRAY,
  PAIR,
  CLASS,
  UNION
};

// What is the difference between Kind enum and Type enum? Kind means the category of the specified type. e.g., all
// references are REFERENCE kind. And all the types that can be manipulated through the List interface are LIST kind.
// And Type enum, on the other hand, indicates what exactly the type is. e.g., 32-bits signed integer is INT32 type, and
// double is FLOAT64 type. Some Type enums also indicate the template container in STL, like std::vector and std::map.
// It's because we regulate the Traits() method of TypeDescriptor offers all the unary predicates from <type_traits>,
// and there is no such trait of std::is_vector. So we offer such inspection in different ways.
// According to the c++ type system, we suppose that a type with cv-qualifier is still this type. e.g., const int still has the enum of
// Type::INT32.

enum class Type : int32_t {
  VOID,
  NULLPTR,
  UINT8,
  INT8,
  UINT32,
  INT32,
  UINT64,
  INT64,
  FLOAT32,
  FLOAT64,
  BOOLEAN,
  CHAR,

  SHARED_PTR,
  UNIQUE_PTR,

  STD_VECTOR,
  STD_DEQUE,
  STD_LIST,
  STD_MAP,
  STD_UNORDERED_MAP,
  STD_STRING,
  STD_ARRAY,
  STD_PAIR,
  STD_ANY,

  NUMBER,
  NUMBER_REFERENCE_NON_CONST,
  NUMBER_REFERENCE_CONST,

  // TODO adc all interface types
  LIST_NON_CONST,
  LIST_CONST,
  STRING_NON_CONST,
  STRING_CONST,

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
  underlying_type
};

class TypeDescriptor;

namespace internal {

struct DescriptorInterface {
  using id_t = uint64_t() noexcept;
  using kinde_enum_t = Kind() noexcept;
  using type_enum_t = Type() noexcept;
  using traits_t = bool(traits) noexcept;
  using transform_t = const TypeDescriptor&(transform) noexcept;
  using size_of_t = size_t() noexcept;
  using alignment_of_t = size_t() noexcept;
  using rank_t = size_t() noexcept;
  using extent_t = size_t() noexcept;

  using value_type_t = const TypeDescriptor&() noexcept;

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
  static inline const TypeDescriptor& GetDescriptor() noexcept;
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

  static constexpr const TypeDescriptor& Transform(transform t) noexcept {
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
      case transform::underlying_type:
        return underlying_type();
    }
    return TypeMeta<void>::GetDescriptor();
  }

  static constexpr decltype(auto) remove_const() noexcept {
    using next_meta = TypeMeta<std::remove_const_t<Tp>>;
    return next_meta::GetDescriptor();
  }
  static constexpr decltype(auto) remove_volatile() noexcept {
    using next_meta = TypeMeta<std::remove_volatile_t<Tp>>;
    return next_meta::GetDescriptor();
  }
  static constexpr decltype(auto) remove_reference() noexcept {
    using next_meta = TypeMeta<std::remove_reference_t<Tp>>;
    return next_meta::GetDescriptor();
  }
  static constexpr decltype(auto) remove_pointer() noexcept {
    using next_meta = TypeMeta<std::remove_pointer_t<Tp>>;
    return next_meta::GetDescriptor();
  }
  static constexpr decltype(auto) remove_extent() noexcept {
    using next_meta = TypeMeta<std::remove_extent_t<Tp>>;
    return next_meta::GetDescriptor();
  }
  static constexpr decltype(auto) remove_all_extents() noexcept {
    using next_meta = TypeMeta<std::remove_all_extents_t<Tp>>;
    return next_meta::GetDescriptor();
  }
  static constexpr decltype(auto) underlying_type() noexcept {
    // If T is a complete enumeration (enum) type, provides a member typedef type that names the underlying type of T.
    // Otherwise, the behavior is undefined. (before c++20)
    if constexpr (std::is_enum_v<Tp>) {
      using next_meta = TypeMeta<std::underlying_type_t<Tp>>;
      return next_meta::GetDescriptor();
    } else {
      return GetDescriptor();
    }
  }

  static constexpr const TypeDescriptor& ValueType() noexcept {
    if constexpr (std::is_pointer_v<Tp>) {
      return remove_pointer();
    } else if constexpr (std::is_reference_v<Tp>) {
      return remove_reference();
    } else if constexpr (std::is_array_v<Tp>) {
      return remove_extent();
    } else if constexpr (IsSmartPtrV<Tp>) {
      return TypeMeta<typename SmartPtrTraits<Tp>::value_type>::GetDescriptor();
    } else if constexpr (IsListV<Tp>) {
      using traits = ListTraits<Tp>;
      return TypeMeta<typename traits::value_type>::GetDescriptor();
    } else if constexpr (IsArrayV<Tp>) {
      using traits = ArrayTraits<Tp>;
      return TypeMeta<typename traits::value_type>::GetDescriptor();
    } else if constexpr (IsPairV<Tp>) {
      using traits = PairTraits<Tp>;
      return TypeMeta<typename traits::value_type>::GetDescriptor();
    }

    return TypeMeta<void>::GetDescriptor();
  }

  static constexpr Kind KindEnum() noexcept {
    if constexpr (IsObjectV<Tp>) {
      return Kind::OBJECT;
    } else if constexpr (std::is_void_v<Tp>) {
      return Kind::VOID;
    } else if constexpr (IsNumberV<Tp> || (std::is_arithmetic_v<Tp> && !is_char_v<Tp>)) {
      return Kind::NUMBER;
    } else if constexpr (IsNumberReferenceV<Tp>) {
      return Kind::NUMBER_REFERENCE;
    } else if constexpr (is_char_v<Tp>) {
      return Kind::CHAR;
    } else if constexpr (std::is_scalar_v<Tp>) {
      return Kind::OTHER_SCALAR;
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
    using removed_cv = std::remove_cv_t<Tp>;
    if constexpr (std::is_same_v<removed_cv, char>) {
      return Type::CHAR;
    } else if constexpr (std::is_same_v<removed_cv, bool>) {
      return Type::BOOLEAN;
    } else if constexpr (std::is_same_v<removed_cv, uint8_t>) {
      return Type::UINT8;
    } else if constexpr (std::is_same_v<removed_cv, int8_t>) {
      return Type::INT8;
    } else if constexpr (std::is_same_v<removed_cv, uint32_t>) {
      return Type::UINT32;
    } else if constexpr (std::is_same_v<removed_cv, int32_t>) {
      return Type::INT32;
    } else if constexpr (std::is_same_v<removed_cv, uint64_t>) {
      return Type::UINT64;
    } else if constexpr (std::is_same_v<removed_cv, int64_t>) {
      return Type::INT64;
    } else if constexpr (std::is_same_v<removed_cv, float>) {
      return Type::FLOAT32;
    } else if constexpr (std::is_same_v<removed_cv, double>) {
      return Type::FLOAT64;
    } else if constexpr (std::is_void_v<Tp>) {
      return Type::VOID;
    } else if constexpr (std::is_null_pointer_v<Tp>) {
      return Type::NULLPTR;
    } else if constexpr (IsSmartPtrV<Tp>) {
      if constexpr (SmartPtrTraits<Tp>::category == UNIQUE_PTR) {
        return Type::UNIQUE_PTR;
      } else {
        static_assert(SmartPtrTraits<Tp>::category == SHARED_PTR);
        return Type::SHARED_PTR;
      }
    } else if constexpr (std::is_same_v<removed_cv, std::string>) {
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
    } else if constexpr (IsSTDarrayV<Tp>) {
      return Type::STD_ARRAY;
    } else if constexpr (std::is_same_v<removed_cv, std::any>) {
      return Type::STD_ANY;
    } else if constexpr (IsSTDpairV<Tp>) {
      return Type::STD_PAIR;
    } else if constexpr (IsNumberV<Tp>) {
      return Type::NUMBER;
    } else if constexpr (std::is_same_v<removed_cv, NumberReference<ConstOption::NON_CONST>>) {
      return Type::NUMBER_REFERENCE_NON_CONST;
    } else if constexpr (std::is_same_v<removed_cv, NumberReference<ConstOption::CONST>>) {
      return Type::NUMBER_REFERENCE_CONST;
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

inline const TypeDescriptor void_descriptor = TypeMeta<void>::GetDescriptor();

}  // namespace liteproto