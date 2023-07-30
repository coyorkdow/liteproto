//
// Created by Youtao Guo on 2023/6/27.
//

#pragma once

#include <cassert>

#include "liteproto/list.hpp"
#include "liteproto/map.hpp"
#include "liteproto/message.hpp"
#include "liteproto/pair.hpp"
#include "liteproto/reflect/number.hpp"
#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/proxy.hpp"
#include "liteproto/reflect/type.hpp"
#include "liteproto/string.hpp"
#include "liteproto/traits/traits.hpp"

namespace liteproto {

template <class Tp>
[[nodiscard]] Object GetReflection(Tp* v) noexcept {
  constexpr ConstOption opt [[maybe_unused]] = std::is_const_v<Tp> ? ConstOption::CONST : ConstOption::NON_CONST;
  if constexpr (IsNumberV<Tp> || (std::is_arithmetic_v<Tp>)) {
    return Object(v, NumberReference(*v));
  } else if constexpr (IsStringV<Tp>) {
    return Object(v, AsString(v));
  } else if constexpr (IsListV<Tp>) {
    return Object(v, AsList(v));
  } else if constexpr (IsArrayV<Tp>) {
    // TODO
  } else if constexpr (IsPairV<Tp>) {
    return Object(v, AsPair(v));
  } else {
    return Object(v);
  }
  std::abort();  // never enter
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
inline const TypeDescriptor& TypeMeta<Tp>::GetDescriptor() noexcept {
  static const TypeDescriptor descriptor = [&] {
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
    inter.first_type_ = &FirstType;
    inter.second_type_ = &SecondType;

    inter.is_indirect_type_ = &IsIndirectType;
    inter.default_value_ = &DefaultValue;
    return TypeDescriptor{inter};
  }();
  return descriptor;
}

// template <class Tp, ConstOption Opt>

template <class Tp, ConstOption Opt>
std::optional<List<Tp, Opt>> ListCast(const Object& object) noexcept {
  using return_type = std::optional<List<Tp, Opt>>;
  auto indirect_ptr = std::any_cast<List<Tp, Opt>>(&object.interface_);
  if (indirect_ptr == nullptr) {
    return return_type{};
  }
  return return_type{*indirect_ptr};
}

template <ConstOption Opt>
std::optional<String<Opt>> StringCast(const Object& object) noexcept {
  using return_type = std::optional<String<Opt>>;
  auto indirect_ptr = std::any_cast<String<Opt>>(&object.interface_);
  if (indirect_ptr == nullptr) {
    return return_type{};
  }
  return return_type{*indirect_ptr};
}

inline std::pair<Object, std::any> TypeDescriptor::DefaultValue() const noexcept { return inter_.default_value_(); }

}  // namespace liteproto