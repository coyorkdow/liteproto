//
// Created by Youtao Guo on 2023/7/5.
//

#pragma once

#include "liteproto/reflect/number.hpp"
#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/type.hpp"

namespace liteproto {

namespace internal {
struct Dummy;
using DummyPointer = Dummy*;
}  // namespace internal

template <class Tp, class = void>
struct ProxyType {
  using type = Tp;
};

template <class Tp>
struct ProxyType<Tp, std::enable_if_t<IsNumberTypeV<Tp>>> {
  using type = Number;
};

template <class Tp>
struct ProxyType<Tp, std::enable_if_t<IsIndirectTypeV<Tp>>> {
  using type = Object;
};

template <class Tp>
struct IsProxyType : std::disjunction<IsNumber<Tp>, IsObject<Tp>, IsNumberReference<Tp>> {};

template <class Tp>
inline constexpr bool IsProxyTypeV = IsProxyType<Tp>::value;

template <class Tp, ConstOption Opt>
struct InterfaceTraits {
  using value_type = std::conditional_t<Opt == ConstOption::CONST, const Tp, Tp>;
  using pointer = value_type*;
  using reference = value_type&;
};

template <ConstOption Opt>
struct InterfaceTraits<Object, Opt> {
  using value_type = Object;
  using pointer = internal::DummyPointer;
  using reference = Object;
};

template <ConstOption Opt>
struct InterfaceTraits<Number, Opt> {
  using value_type = Number;
  using pointer = internal::DummyPointer;
  using reference = NumberReference<Opt>;
};

template <class Proxy, class Tp, class = std::enable_if_t<IsObjectV<Proxy> || IsNumberReferenceV<Proxy>>>
Proxy MakeProxy(Tp&& value) noexcept {
  if constexpr (IsObjectV<Proxy>) {
    return GetReflection(&value);
  } else if constexpr (IsNumberReferenceV<Proxy>) {
    return Proxy{value};
  }
}

template <class Proxy>
struct MakeProxyWrapper {
  template <class Tp>
  Proxy operator()(Tp&& value) noexcept {
    return MakeProxy<Proxy>(std::forward<Tp>(value));
  }
};

struct IdentityWrapper {
  template <class Tp>
  decltype(auto) operator()(Tp&& value) noexcept {
    return std::forward<Tp>(value);
  }
};

template <class ProxyPair, class = std::enable_if_t<IsPairV<ProxyPair>>>
struct PairWrapper {
  template <class Pair, class = std::enable_if_t<IsPairV<std::remove_reference_t<Pair>>>>
  ProxyPair operator()(Pair&& pair) noexcept {
    using first_type = typename PairTraits<ProxyPair>::first_type;
    using second_type = typename PairTraits<ProxyPair>::second_type;
    return ProxyPair{MakeProxy<first_type>(std::forward<Pair>(pair).first, MakeProxy<second_type>(std::forward<Pair>(pair).second))};
  }
};

template <class Underlying, class Tp, class = std::enable_if_t<IsProxyTypeV<std::remove_reference_t<Tp>>>>
std::optional<Underlying> RestoreFromProxy(Tp&& value) noexcept;

template <class Underlying, class Tp, class>
std::optional<Underlying> RestoreFromProxy(Tp&& value) noexcept {
  using return_type = std::optional<Underlying>;
  using rm_refed = std::remove_reference_t<Tp>;
  if constexpr (IsObjectV<rm_refed>) {
    auto v_ptr = ObjectCast<Underlying>(value);
    if (v_ptr != nullptr) {
      // If this is an indirect interface, all the modification operations are considered as pass by "move".
      return return_type{std::move(*v_ptr)};
    }
  } else if constexpr (IsNumberV<rm_refed> || IsNumberReferenceV<rm_refed>) {
    if (value.IsSignedInteger()) {
      return return_type{value.AsInt64()};
    } else if (value.IsUnsigned()) {
      return return_type{value.AsUInt64()};
    } else {
      return return_type{value.AsFloat64()};
    }
  }
  return return_type{};
}

}  // namespace liteproto