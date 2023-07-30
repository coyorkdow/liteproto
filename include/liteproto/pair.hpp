//
// Created by Youtao Guo on 2023/6/27.
//

#pragma once

#include "liteproto/reflect/number.hpp"
#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/proxy.hpp"
#include "liteproto/reflect/type.hpp"

namespace liteproto {

template <class FT, class ST>
using Pair = std::pair<FT, ST>;

template <class C, class = std::enable_if_t<IsPairV<C>>>
auto AsPair(C* pair) noexcept {
  using first_type = typename ProxyType<typename PairTraits<C>::first_type>::type;
  using second_type = typename ProxyType<typename PairTraits<C>::second_type>::type;
  constexpr bool is_const = std::is_const_v<C>;
  using first_reference = typename InterfaceTraits<first_type, static_cast<ConstOption>(is_const)>::reference;
  using second_reference = typename InterfaceTraits<second_type, static_cast<ConstOption>(is_const)>::reference;
  return std::make_pair(MakeProxy<first_reference>(pair->first), MakeProxy<second_reference>(pair->second));
}

}  // namespace liteproto