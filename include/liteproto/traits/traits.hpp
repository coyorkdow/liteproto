//
// Created by Youtao Guo on 2023/6/25.
//

#pragma once

#include <array>
#include <forward_list>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "liteproto/traits/basic_traits.hpp"

namespace liteproto {

class Object;
class Number;

namespace internal {

template <class C, class V>
inline constexpr bool is_list =
    has_push_back_v<C, V> && has_pop_back_v<C> && has_size_v<C> && has_resize_v<C, V> && has_empty_v<C> &&
    has_clear_v<C> && is_bidirectional_iterable_v<C> && has_insert_v<C, V> && has_erase_v<C>;

template <class Str, class Char>
inline constexpr bool is_string =
    is_list<Str, Char> && has_c_str_v<Str, Char> && has_append_v<Str, Char> && has_data_v<Str, char>;

template <class C, class V>
inline constexpr bool is_array =
    std::is_array_v<C> || (has_subscript_v<C, V> && (has_constexpr_size_v<C> || has_size_v<C>)&&has_data_v<C, V> &&
                           is_bidirectional_iterable_v<C>);

template <class C, class K, class V>
inline constexpr bool is_map = has_insert_v<C, std::pair<K, V>> && has_find_v<C, K, V> && has_size_v<C> &&
                               has_empty_v<C> && has_clear_v<C> && has_erase_v<C> && is_forward_iterable_v<C>;

}  // namespace internal

// template <class Tp>
// struct EnforcedKind;

// using NonTempVector = void;
//
// template <>
// struct EnforcedKind<NonTempVector> {
//   static constexpr Kind kind = Kind::LIST;
//   using value_type = void;
// };

enum SmartPtrCategory : uint8_t { SHARED_PTR, UNIQUE_PTR };

template <class Tp>
struct SmartPtrTraits {};

template <class Tp>
struct SmartPtrTraits<std::shared_ptr<Tp>> {
  using value_type = Tp;
  static constexpr SmartPtrCategory category = SHARED_PTR;
};

template <class Tp, class Dp>
struct SmartPtrTraits<std::unique_ptr<Tp, Dp>> {
  using value_type = Tp;
  static constexpr SmartPtrCategory category = UNIQUE_PTR;
};

template <class Tp, class Cond = void>
struct IsSmartPtr : std::false_type {};

template <class Tp>
struct IsSmartPtr<Tp, std::void_t<typename SmartPtrTraits<Tp>::value_type>> : std::true_type {};

template <class Tp>
inline constexpr bool IsSmartPtrV = IsSmartPtr<Tp>::value;

static_assert(IsSmartPtrV<std::unique_ptr<int>> && IsSmartPtrV<std::shared_ptr<void*>>);
static_assert(SmartPtrTraits<std::unique_ptr<int>>::category == UNIQUE_PTR);
static_assert(!IsSmartPtrV<const std::unique_ptr<int>>);

#define LITE_PROTO_MAKE_STL_CONTAINER_TRAITS_(name)                                                    \
  template <class Tp, class Cond = void>                                                               \
  struct STD##name##Traits {};                                                                         \
  template <class... Args>                                                                             \
  struct STD##name##Traits<std::name<Args...>> {                                                       \
    using value_type = typename std::name<Args...>::value_type;                                        \
  };                                                                                                   \
  template <class Tp, class Cond = void>                                                               \
  struct IsSTD##name : std::false_type {};                                                             \
  template <class Tp>                                                                                  \
  struct IsSTD##name<Tp, std::void_t<typename STD##name##Traits<Tp>::value_type>> : std::true_type {}; \
  template <class Tp>                                                                                  \
  inline constexpr bool IsSTD##name##V = IsSTD##name<Tp>::value;

LITE_PROTO_MAKE_STL_CONTAINER_TRAITS_(vector)
LITE_PROTO_MAKE_STL_CONTAINER_TRAITS_(deque)
LITE_PROTO_MAKE_STL_CONTAINER_TRAITS_(list)
LITE_PROTO_MAKE_STL_CONTAINER_TRAITS_(map)
LITE_PROTO_MAKE_STL_CONTAINER_TRAITS_(unordered_map)

static_assert(IsSTDvectorV<std::vector<int>>);
static_assert(IsSTDdequeV<std::deque<int>>);
static_assert(IsSTDlistV<std::list<int>>);
static_assert(IsSTDmapV<std::map<int, int>>);
static_assert(IsSTDunordered_mapV<std::unordered_map<int, double>>);
static_assert(std::is_same_v<typename STDmapTraits<std::map<int, double>>::value_type, std::pair<const int, double>>);

#undef LITE_PROTO_MAKE_STL_CONTAINER_TRAITS_

template <class Tp, class Cond = void>
struct STDarrayTraits {};
template <class Arg, size_t N>
struct STDarrayTraits<std::array<Arg, N>> {
  using value_type = typename std::array<Arg, N>::value_type;
};
template <class Tp, class Cond = void>
struct IsSTDarray : std::false_type {};
template <class Tp>
struct IsSTDarray<Tp, std::void_t<typename STDarrayTraits<Tp>::value_type>> : std::true_type {};
template <class Tp>
inline constexpr bool IsSTDarrayV = IsSTDarray<Tp>::value;

static_assert(IsSTDarrayV<std::array<int, 123>>);
static_assert(std::is_same_v<typename STDarrayTraits<std::array<int, 123>>::value_type, int>);

template <class Tp>
struct IsObject : std::false_type {};
template <>
struct IsObject<Object> : std::true_type {};
template <>
struct IsObject<const Object> : std::true_type {};
template <>
struct IsObject<volatile Object> : std::true_type {};
template <>
struct IsObject<const volatile Object> : std::true_type {};

template <class Tp>
inline constexpr bool IsObjectV = IsObject<Tp>::value;

template <class Tp>
struct IsNumber : std::false_type {};
template <>
struct IsNumber<Number> : std::true_type {};
template <>
struct IsNumber<const Number> : std::true_type {};
template <>
struct IsNumber<volatile Number> : std::true_type {};
template <>
struct IsNumber<const volatile Number> : std::true_type {};

template <class Tp>
inline constexpr bool IsNumberV = IsNumber<Tp>::value;

template <class Tp, class Cond = void>
struct ListTraits {};

template <template <class...> class C, class V, class... Tps>
struct ListTraits<C<V, Tps...>, std::enable_if_t<internal::is_list<C<V, Tps...>, V>>> {
  using container_type = C<V, Tps...>;
  using value_type = V;
};

template <template <class...> class C, class V, class... Tps>
struct ListTraits<const C<V, Tps...>, std::enable_if_t<internal::is_list<C<V, Tps...>, V>>> {
  using container_type = const C<V, Tps...>;
  using value_type = V;
};

template <template <class, auto...> class C, class V, auto... Args>
struct ListTraits<C<V, Args...>, std::enable_if_t<internal::is_list<C<V, Args...>, V>>> {
  using container_type = C<V, Args...>;
  using value_type = V;
};

template <template <class, auto...> class C, class V, auto... Args>
struct ListTraits<const C<V, Args...>, std::enable_if_t<internal::is_list<C<V, Args...>, V>>> {
  using container_type = const C<V, Args...>;
  using value_type = V;
};

template <class Tp, class Cond = void>
struct IsList : std::false_type {};

template <class Tp>
struct IsList<Tp, std::void_t<typename ListTraits<Tp>::container_type, typename ListTraits<Tp>::value_type>>
    : std::true_type {};

template <class Tp>
inline constexpr bool IsListV = IsList<Tp>::value;

template <class Str>
struct IsString : std::bool_constant<internal::is_string<Str, char>> {};

template <class Str>
struct IsString<const Str> : std::bool_constant<internal::is_string<Str, char>> {};

template <class Str>
inline constexpr bool IsStringV = IsString<Str>::value;

template <class C, class Cond = void>
struct MapTraits {};

template <template <class...> class C, class K, class V, class... Tps>
struct MapTraits<C<K, V, Tps...>, std::enable_if_t<internal::is_map<C<K, V, Tps...>, K, V>>> {
  using key_type = K;
  using mapped_type = V;
  using value_type = std::pair<const K, V>;
  using container_type = C<K, V, Tps...>;
};

template <template <class...> class C, class K, class V, class... Tps>
struct MapTraits<const C<K, V, Tps...>, std::enable_if_t<internal::is_map<C<K, V, Tps...>, K, V>>> {
  using key_type = K;
  using mapped_type = V;
  using value_type = std::pair<const K, V>;
  using container_type = const C<K, V, Tps...>;
};

template <class Tp, class Cond = void>
struct IsMap : std::false_type {};

template <class Tp>
struct IsMap<Tp, std::void_t<typename MapTraits<Tp>::key_type>> : std::true_type {};

template <class Str>
inline constexpr bool IsMapV = IsMap<Str>::value;

template <class Tp, class Cond = void>
struct ArrayTraits : std::false_type {};

template <template <class, size_t> class C, class V, size_t N>
struct ArrayTraits<C<V, N>, std::enable_if_t<internal::is_array<C<V, N>, V>>> : std::true_type {
  using container_type = C<V, N>;
  using value_type = V;
  static constexpr size_t size = N;
};

template <class Tp, size_t N>
struct ArrayTraits<Tp[N]> : std::true_type {
  using container_type = Tp[N];
  using value_type = Tp;
  static constexpr size_t size = N;
};

template <template <class, size_t> class C, class V, size_t N>
struct ArrayTraits<const C<V, N>, std::enable_if_t<internal::is_array<C<V, N>, V>>> : std::true_type {
  using container_type = const C<V, N>;
  using value_type = V;
  static constexpr size_t size = N;
};

template <class Tp, size_t N>
struct ArrayTraits<const Tp[N]> : std::true_type {
  using container_type = const Tp[N];
  using value_type = Tp;
  static constexpr size_t size = N;
};

template <class Tp, class Cond = void>
struct IsArray : std::false_type {};

template <class Tp>
struct IsArray<Tp, std::void_t<typename ArrayTraits<Tp>::container_type, typename ArrayTraits<Tp>::value_type>>
    : std::true_type {};

template <class Tp>
inline constexpr bool IsArrayV = IsArray<Tp>::value;

template <class Pair, class Cond = void>
struct PairTraits {};

template <class Pair>
struct PairTraits<Pair, std::void_t<decltype(std::declval<Pair>().first), decltype(std::declval<Pair>().second)>> {
  using first = decltype(std::declval<Pair>().first);
  using second = decltype(std::declval<Pair>().second);
  using value_type = std::pair<first, second>;
};

template <class Pair>
struct PairTraits<const Pair> : PairTraits<Pair> {};

template <class Pair, class Cond = void>
struct IsPair : std::false_type {};

template <class Pair>
struct IsPair<Pair, std::void_t<typename PairTraits<Pair>::value_type>> : std::true_type {};

template <class Pair>
inline constexpr bool IsPairV = IsPair<Pair>::value;

template <class Tp, class = void>
struct IsIndirectType : std::true_type {};

template <>  // Object is indirect. We have to make is indirect so that our definition can be well-formed.
struct IsIndirectType<Object> : std::true_type {};

template <>
struct IsIndirectType<void*> : std::false_type {};

template <class Tp>
struct IsIndirectType<const Tp> : IsIndirectType<Tp> {};

template <class Tp>
struct IsIndirectType<volatile Tp> : IsIndirectType<Tp> {};

template <class Tp>
struct IsIndirectType<const volatile Tp> : IsIndirectType<Tp> {};

template <class Tp>
struct IsIndirectType<Tp*> : IsIndirectType<Tp> {};

template <class Tp>
struct IsIndirectType<Tp&> : IsIndirectType<Tp> {};

template <class Tp>
struct IsIndirectType<Tp&&> : IsIndirectType<Tp> {};

template <class Tp>
struct IsIndirectType<Tp, std::enable_if_t<!std::is_const_v<Tp> && !std::is_volatile_v<Tp> && std::is_arithmetic_v<Tp>>>
    : std::false_type {};

template <class Tp>
inline constexpr bool IsIndirectTypeV = IsIndirectType<Tp>::value;

}  // namespace liteproto
