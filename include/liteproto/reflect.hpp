//
// Created by Youtao Guo on 2023/6/27.
//

#pragma once

#include "liteproto/list.hpp"
#include "liteproto/pair.hpp"
#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/type.hpp"
#include "liteproto/traits.hpp"

namespace liteproto {

template <class Tp>
[[nodiscard]] Object GetReflection(Tp* v) noexcept {
  if constexpr (std::is_scalar_v<Tp>) {
    return Object(v);
  } else if constexpr (IsStringV<Tp>) {
    // TODO
  } else if constexpr (IsListV<Tp>) {
    auto list = AsList(v);
    return Object(v, list);
  } else if constexpr (IsArrayV<Tp>) {
    // TODO
  } else if constexpr (IsPairV<Tp>) {
    // TODO
  }
}

template <class Tp>
inline std::pair<Object, std::any> TypeMeta<Tp>::DefaultValue() noexcept {
  if constexpr (std::is_default_constructible_v<Tp>) {
    auto instance = std::make_shared<Tp>();
    auto ptr = instance.get();
    return std::make_pair(GetReflection(ptr), std::any{std::move(instance)});
  }
  return std::make_pair(Object{}, std::any{});
}

template <class Tp>
inline TypeDescriptor TypeMeta<Tp>::MakeDescriptor() noexcept {
  internal::DescriptorInterface inter{};
  inter.id_ = &Id;
  inter.size_of_ = &SizeOf;
  inter.alignment_of_ = &AlignmentOf;
  inter.rank_ = &Rank;
  inter.extent_ = &Extent;
  inter.transform_ = &Transform;
  inter.type_enum_ = &TypeEnum;
  inter.kind_enum_ = &KindEnum;
  inter.traits_ = &Traits;
  inter.transform_ = &Transform;

  inter.value_type_ = &ValueType;
  inter.is_indirect_type_ = &IsIndirectType;
  inter.default_value_ = &DefaultValue;
  return TypeDescriptor{inter};
}

template <class Tp, ConstOption Opt = ConstOption::NON_CONST>
auto ListCast(const Object& object) noexcept {
  using return_type = std::optional<List<Tp, Opt>>;
  auto indirect_ptr = std::any_cast<List<Tp, Opt>>(&object.interface_);
  if (indirect_ptr == nullptr) {
    return return_type{};
  }
  return return_type{*indirect_ptr};
}

inline std::pair<Object, std::any> TypeDescriptor::DefaultValue() const noexcept { return inter_.default_value_(); }

}  // namespace liteproto