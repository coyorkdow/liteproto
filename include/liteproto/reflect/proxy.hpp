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
 private:
  using key_type = typename PairTraits<Tp>::first_type;
  using mapped_type = typename PairTraits<Tp>::second_type;

 public:
  using pointer = typename details::InterfaceTraitsHelper<key_type, mapped_type, Opt>::pointer;
  using reference = typename details::InterfaceTraitsHelper<key_type, mapped_type, Opt>::reference;
};

template <class Proxy, class Tp, class = std::enable_if_t<IsObjectV<Proxy> || IsNumberReferenceV<Proxy>>>
Proxy MakeProxy(Tp&& value) noexcept {
  if constexpr (IsObjectV<Proxy>) {
    return GetReflection(&value);
  } else if constexpr (IsNumberReferenceV<Proxy>) {
    return Proxy{value};
  }
  //  else {
  //    static_assert(IsPairV<Proxy>);
  //    using first_type = typename PairTraits<Proxy>::first_type;
  //    using second_type = typename PairTraits<Proxy>::second_type;
  //    if constexpr (IsProxyTypeV<first_type> && IsProxyTypeV<second_type>) {
  //      return Proxy{MakeProxy<first_type>(value.first), MakeProxy<first_type>(value.second)};
  //    } else if constexpr (IsProxyTypeV<first_type>) {
  //      return Proxy{MakeProxy<first_type>(value.first), value.second};
  //    } else {
  //      return Proxy{value.first, MakeProxy<second_type>(value.second)};
  //    }
  //  }
}

template <class Underlying, class Tp, class = std::enable_if_t<IsProxyTypeV<std::remove_reference_t<Tp>>>>
std::optional<Underlying> RestoreFromProxy(Tp&& value) noexcept;

namespace details {
template <class Underlying, class Tp>
auto RestoreFromProxyHelper(Tp&& value) noexcept {
  if constexpr (IsProxyTypeV<std::remove_reference_t<Tp>>) {
    return RestoreFromProxy<Underlying>(std::forward<Tp>(value));
  } else {
    return std::optional<Underlying>(std::forward<Tp>(value));
  }
}
}  // namespace details

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
  //  else {
  //    static_assert(IsPairV<rm_refed> && IsProxyTypeV<Underlying>);
  //    using first_type = typename PairTraits<Underlying>::first_type;
  //    using second_type = typename PairTraits<Underlying>::second_type;
  //    if constexpr (std::is_rvalue_reference_v<decltype(value)>) {
  //      auto first = details::RestoreFromProxyHelper<first_type>(std::move(value.first));
  //      auto second = details::RestoreFromProxyHelper<second_type>(std::move(value.second));
  //      if (first.has_value() && second.has_value()) {
  //        return return_type{std::in_place, std::move(first.value()), std::move(second.value())};
  //      }
  //    } else {
  //      auto first = details::RestoreFromProxyHelper<first_type>(value.first);
  //      auto second = details::RestoreFromProxyHelper<second_type>(value.second);
  //      if (first.has_value() && second.has_value()) {
  //        return return_type{std::in_place, std::move(first.value()), std::move(second.value())};
  //      }
  //    }
  //  }

  return return_type{};
}

}  // namespace liteproto