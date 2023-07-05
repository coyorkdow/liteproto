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

namespace details {

template <class Tp, class = std::enable_if_t<std::is_arithmetic_v<Tp> || IsNumberV<Tp>>>
auto IsProxiedByNumberImpl(int) -> std::true_type;

template <class Tp>
auto IsProxiedByNumberImpl(float) -> std::false_type;
}  // namespace details

template <class Tp>
struct IsProxiedByNumber : decltype(details::IsProxiedByNumberImpl<std::remove_reference_t<Tp>>(0)) {};

template <class Tp>
static inline constexpr bool IsProxiedByNumberV = IsProxiedByNumber<Tp>::value;

template <class Tp, class = void>
struct ProxyType {
  using type = Tp;
};

template <class Tp>
struct ProxyType<Tp, std::enable_if_t<IsProxiedByNumberV<Tp>>> {
  using type = Number;
};

template <class Tp>
struct ProxyType<Tp, std::enable_if_t<IsIndirectTypeV<Tp>>> {
  using type = Object;
};

template <class Tp>
struct ProxyType<Tp, std::enable_if_t<IsPairV<Tp>>> {
 private:
  using underlying_first = typename PairTraits<Tp>::first_type;
  using underlying_second = typename PairTraits<Tp>::second_type;

 public:
  using type = std::pair<typename ProxyType<underlying_first>::type, typename ProxyType<underlying_second>::type>;
};

template <class Tp>
struct IsProxyType : std::false_type {};

template <>
struct IsProxyType<Object> : std::true_type {};

template <>
struct IsProxyType<Number> : std::true_type {};

template <ConstOption Opt>
struct IsProxyType<NumberReference<Opt>> : std::true_type {};

template <class FT, class ST>
struct IsProxyType<std::pair<FT, ST>> : std::bool_constant<IsProxyType<FT>::value || IsProxyType<ST>::value> {};

template <class Tp>
inline constexpr bool IsProxyTypeV = IsProxyType<Tp>::value;

template <class Tp, ConstOption Opt, class Cond = void>
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

namespace details {

template <class K, class V, ConstOption Opt, class Cond = void>
struct InterfaceTraitsHelper;

template <class K, class V>
struct InterfaceTraitsHelper<K, V, ConstOption::NON_CONST, std::enable_if_t<!IsProxyTypeV<K> && !IsProxyTypeV<V>>> {
  using pointer = std::pair<K, V>*;
  using reference = std::pair<K, V>&;
};

template <class K, class V>
struct InterfaceTraitsHelper<K, V, ConstOption::CONST, std::enable_if_t<!IsProxyTypeV<K> && !IsProxyTypeV<V>>> {
  using pointer = const std::pair<K, V>*;
  using reference = const std::pair<K, V>&;
};

template <class K, class V, ConstOption Opt>
struct InterfaceTraitsHelper<K, V, Opt, std::enable_if_t<IsProxyTypeV<K> || IsProxyTypeV<V>>> {
  using pointer = internal::DummyPointer;
  using reference = std::pair<typename InterfaceTraits<K, Opt>::reference, typename InterfaceTraits<V, Opt>::reference>;
};
}  // namespace details

template <class Tp, ConstOption Opt>
struct InterfaceTraits<Tp, Opt, std::enable_if_t<IsPairV<Tp>>> {
  using key_type = typename PairTraits<Tp>::first_type;
  using mapped_type = typename PairTraits<Tp>::second_type;
  using value_type = std::pair<std::conditional_t<IsProxyTypeV<key_type>, key_type, const key_type>, mapped_type>;
  using pointer = typename details::InterfaceTraitsHelper<key_type, mapped_type, Opt>::pointer;
  using reference = typename details::InterfaceTraitsHelper<key_type, mapped_type, Opt>::reference;
};

template <class Proxy, class Tp, class = std::enable_if_t<IsProxyTypeV<Proxy>>>
Proxy MakeProxy(Tp&& value) {
  if constexpr (IsObjectV<Proxy>) {
    return GetReflection(&value);
  } else if constexpr (IsNumberReferenceV<Proxy>) {
    return Proxy{value};
  } else {
    static_assert(IsPairV<Proxy>);
    using first_type = typename PairTraits<Proxy>::first_type;
    using second_type = typename PairTraits<Proxy>::second_type;
    if constexpr (IsProxyTypeV<first_type> && IsProxyTypeV<second_type>) {
      return Proxy{MakeProxy<first_type>(value.first), MakeProxy<first_type>(value.second)};
    } else if constexpr (IsProxyTypeV<first_type>) {
      return Proxy{MakeProxy<first_type>(value.first), value.second};
    } else {
      return Proxy{value.first, MakeProxy<second_type>(value.second)};
    }
  }
}

}  // namespace liteproto