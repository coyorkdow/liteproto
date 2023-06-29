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

namespace liteproto {

class Object;

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

// object types are (possibly cv-qualified) types that are not function types, reference types, or possibly cv-qualified
// void. scalar types are (possibly cv-qualified) object types that are not array types or class types.
enum class Kind : int8_t {
  VOID,
  SCALAR,
  REFERENCE,
  FUNCTION,
  OBJECT,
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

namespace details {
template <class C,
          class = std::enable_if_t<std::is_convertible_v<std::invoke_result_t<decltype(&C::size), const C>, size_t>>>
auto HasSize(int) -> std::true_type;

template <class>
auto HasSize(float) -> std::false_type;

template <class C, size_t = std::tuple_size<C>::value>
auto HasConstexprSize(int) -> std::true_type;

template <class C, class = std::enable_if_t<std::is_array_v<C>>>
auto HasConstexprSize(int) -> std::true_type;

template <class>
auto HasConstexprSize(float) -> std::false_type;

template <class C, class V, class = decltype(std::declval<C&>().resize(std::declval<size_t>())),
          class = decltype(std::declval<C&>().resize(std::declval<size_t>(), std::declval<V>()))>
auto HasReSize(int) -> std::true_type;

template <class, class>
auto HasReSize(float) -> std::false_type;

template <class C, class = std::enable_if_t<
                       std::is_convertible_v<std::invoke_result_t<decltype(&C::capacity), const C>, size_t>>>
auto HasCapacity(int) -> std::true_type;

template <class>
auto HasCapacity(float) -> std::false_type;

template <class C,
          class = std::enable_if_t<std::is_convertible_v<std::invoke_result_t<decltype(&C::empty), const C>, bool>>>
auto HasEmpty(int) -> std::true_type;

template <class>
auto HasEmpty(float) -> std::false_type;

template <class C, class = std::invoke_result_t<decltype(&C::clear), C>>
auto HasClear(int) -> std::true_type;

template <class>
auto HasClear(float) -> std::false_type;

template <class C, class V, class = decltype(std::declval<C&>().push_back(std::declval<V>()))>
auto HasPushBack(int) -> std::true_type;

template <class, class>
auto HasPushBack(float) -> std::false_type;

template <class C, class = std::invoke_result_t<decltype(&C::pop_back), C>>
auto HasPopBack(int) -> std::true_type;

template <class>
auto HasPopBack(float) -> std::false_type;

template <class C, class Char,
          class = std::enable_if_t<std::is_same_v<const Char*, std::invoke_result_t<decltype(&C::c_str), const C>>>>
auto HasCStr(int) -> std::true_type;

template <class, class>
auto HasCStr(float) -> std::false_type;

template <class C, class Char,
          class = decltype(std::declval<C>().append(std::declval<const Char*>(), std::declval<size_t>()))>
auto HasAppend(int) -> std::true_type;

template <class, class>
auto HasAppend(float) -> std::false_type;

template <class C, class V,
          class = std::enable_if_t<std::is_same_v<V*, decltype(std::declval<C&>().data())> &&
                                   std::is_same_v<const V*, decltype(std::declval<const C&>().data())>>>
auto HasData(int) -> std::true_type;

template <class C, class V>
auto HasData(float) -> std::false_type;

template <class It, class = std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag,
                                                               typename std::iterator_traits<It>::iterator_category>>>
auto IsForwardIterator(int) -> std::true_type;

template <class>
auto IsForwardIterator(float) -> std::false_type;

using std::begin;
using std::end;

template <class Tp, class = std::enable_if_t<
                        std::is_same_v<decltype(begin(std::declval<Tp&>())), decltype(end(std::declval<Tp&>()))>>>
auto IsForwardIterable(int) -> decltype(IsForwardIterator<decltype(begin(std::declval<Tp&>()))>(0));

template <class>
auto IsForwardIterable(float) -> std::false_type;

template <class C, class It = decltype(begin(std::declval<C&>())),
          class = decltype(std::declval<C&>().erase(std::declval<It>()))>
auto HasErase(int) -> std::true_type;

template <class>
auto HasErase(float) -> std::false_type;

template <class C, class V, class It = decltype(begin(std::declval<C&>())),
          class = decltype(std::declval<C&>().insert(std::declval<It>(), std::declval<V>()))>
auto HasInsert(int) -> std::true_type;

template <class, class>
auto HasInsert(float) -> std::false_type;

template <class C, class V,
          class = std::enable_if_t<
              std::is_convertible_v<decltype(std::declval<C&>()[std::declval<size_t>()]), V&> &&
              std::is_convertible_v<decltype(std::declval<const C&>()[std::declval<size_t>()]), const V&>>>
auto HasSubscript(int) -> std::true_type;

template <class, class>
auto HasSubscript(float) -> std::false_type;

template <class C, class = std::void_t<decltype(std::declval<C&>().first), decltype(std::declval<C&>().second)>>
auto IsPair(int) -> std::true_type;

template <class C>
auto IsPair(float) -> std::false_type;

}  // namespace details

// We have two levels of member function requirement for Containers.
// The strict one is, by given a template class `C<Tp>, name `foo`, and a series of type `Tps...`. There must be one
// determined member function `C<Tp>::foo`, whose type can be invoked by the argument(s) `Tps...` (`this` parameter is
// omitted here).
// The relaxed one is, by given a template class `C<Tp>, name `foo`, and a series of type `Tps...`. The expression
// `c.foo(args...)` is accepted, where the `c` is an object of type `C<Tp>` and `args...` is a series of objects of
// which types is `Tps...`.
// The difference is, for the strict requirement, the `foo` cannot be a template, and any overload is not allowed
// either. Besides, the default arguments will be ignored (if any). While on the other hand, the relaxed requirement
// only requires a certain invocation can be done.
// In addition to these. For both requirement levels, the `const` specifier might be required, and there are might be
// some requirements on the result type as well.
//
// For example,
// The requirement of `C<Tp>.size()` is strict. It requires `&C<Tp>.size` is a pointer to member function.
// It must be const, and the result type can be implicitly converted to the `size_const_t`.
// The requirements of `C<Tp>.push_back(Tp)` is relaxed. Therefore, the following template is acceptable.
// template <class Tp>
// void push_back(Tp&& tp);

template <class C>
struct has_size : decltype(details::HasSize<C>(0)) {};

template <class C>
inline constexpr bool has_size_v = has_size<C>::value;

template <class C>
struct has_constexpr_size : decltype(details::HasConstexprSize<C>(0)) {};

template <class C>
inline constexpr bool has_constexpr_size_v = has_constexpr_size<C>::value;

template <class C, class V>
struct has_resize : decltype(details::HasReSize<C, V>(0)) {};

template <class C, class V>
inline constexpr bool has_resize_v = has_resize<C, V>::value;

template <class C>
struct has_capacity : decltype(details::HasCapacity<C>(0)) {};

template <class C>
inline constexpr bool has_capacity_v = has_capacity<C>::value;

template <class C>
struct has_empty : decltype(details::HasEmpty<C>(0)) {};

template <class C>
inline constexpr bool has_empty_v = has_empty<C>::value;

template <class C>
struct has_clear : decltype(details::HasClear<C>(0)) {};

template <class C>
inline constexpr bool has_clear_v = has_clear<C>::value;

template <class C, class Tp>
struct has_push_back : decltype(details::HasPushBack<C, Tp>(0)) {};

template <class C, class Tp>
inline constexpr bool has_push_back_v = has_push_back<C, Tp>::value;

template <class C>
struct has_pop_back : decltype(details::HasPopBack<C>(0)) {};

template <class C>
inline constexpr bool has_pop_back_v = has_pop_back<C>::value;

template <class Tp>
struct is_forward_iterator : decltype(details::IsForwardIterator<Tp>(0)) {};

template <class Tp>
inline constexpr bool is_forward_iterator_v = is_forward_iterator<Tp>::value;

template <class C>
struct is_forward_iterable : decltype(details::IsForwardIterable<C>(0)) {};

template <class C>
inline constexpr bool is_forward_iterable_v = is_forward_iterable<C>::value;

template <class C>
struct has_erase : decltype(details::HasErase<C>(0)) {};

template <class C>
inline constexpr bool has_erase_v = has_erase<C>::value;

template <class C, class Tp>
struct has_insert : decltype(details::HasInsert<C, Tp>(0)) {};

template <class C, class Tp>
inline constexpr bool has_insert_v = has_insert<C, Tp>::value;

template <class C, class Char>
struct has_c_str : decltype(details::HasCStr<C, Char>(0)) {};

template <class C, class Char>
inline constexpr bool has_c_str_v = has_c_str<C, Char>::value;

template <class C, class Char>
struct has_append : decltype(details::HasAppend<C, Char>(0)) {};

template <class C, class Char>
inline constexpr bool has_append_v = has_append<C, Char>::value;

template <class C, class Tp>
struct has_subscript : decltype(details::HasSubscript<C, Tp>(0)) {};

template <class C, class Tp>
inline constexpr bool has_subscript_v = has_subscript<C, Tp>::value;

template <class C, class Tp>
struct has_data : decltype(details::HasData<C, Tp>(0)) {};

template <class C, class Tp>
inline constexpr bool has_data_v = has_data<C, Tp>::value;

// For the traits of our own types, we use camel case instead of underscore case.

namespace internal {

template <class C, class V>
inline constexpr bool is_list =
    has_push_back_v<C, V> && has_pop_back_v<C> && has_size_v<C> && has_resize_v<C, V> && has_empty_v<C> &&
    has_clear_v<C> && is_forward_iterable_v<C> && has_insert_v<C, V> && has_erase_v<C>;

template <class Str, class Char>
inline constexpr bool is_string =
    is_list<Str, Char> && has_c_str_v<Str, Char> && has_append_v<Str, Char> && has_data_v<Str, char>;

template <class C, class V>
inline constexpr bool is_array =
    std::is_array_v<C> ||
    (has_subscript_v<C, V> && (has_constexpr_size_v<C> || has_size_v<C>)&&has_data_v<C, V> && is_forward_iterable_v<C>);

// template <class C>

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

template <class Tp, class Cond = void>
struct SmartPtrTraits {};

template <class Tp>
struct SmartPtrTraits<std::shared_ptr<Tp>> {
  using value_type = Tp;
};

template <class Tp, class Dp>
struct SmartPtrTraits<std::unique_ptr<Tp, Dp>> {
  using value_type = Tp;
};

template <class Tp, class Cond = void>
struct IsSmartPtr : std::false_type {};

template <class Tp>
struct IsSmartPtr<Tp, std::void_t<typename SmartPtrTraits<Tp>::value_type>> : std::true_type {};

template <class Tp>
inline constexpr bool IsSmartPtrV = IsSmartPtr<Tp>::value;

static_assert(IsSmartPtrV<std::unique_ptr<int>> && IsSmartPtrV<std::shared_ptr<void*>>);

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
static_assert(IsSTDunordered_mapV<std::unordered_map<int, int>>);

#undef LITE_PROTO_MAKE_STL_CONTAINER_TRAITS_

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

template <class Tp, class Cond = void>
struct ListTraits {};

template <template <class...> class C, class V, class... Tps>
struct ListTraits<C<V, Tps...>, std::enable_if_t<internal::is_list<C<V>, V>>> : std::true_type {
  using container_type = C<V, Tps...>;
  using value_type = V;
};

template <template <class...> class C, class V, class... Tps>
struct ListTraits<const C<V, Tps...>, std::enable_if_t<internal::is_list<C<V>, V>>> : std::true_type {
  using container_type = const C<V, Tps...>;
  using value_type = V;
};

template <template <class, auto...> class C, class V, auto... Args>
struct ListTraits<C<V, Args...>, std::enable_if_t<internal::is_list<C<V, Args...>, V>>> : std::true_type {
  using container_type = C<V, Args...>;
  using value_type = V;
};

template <template <class, auto...> class C, class V, auto... Args>
struct ListTraits<const C<V, Args...>, std::enable_if_t<internal::is_list<C<V, Args...>, V>>> : std::true_type {
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

template <class Pair>
struct IsPair : decltype(details::IsPair<Pair>(0)) {};

template <class Pair>
struct IsPair<const Pair> : decltype(details::IsPair<Pair>(0)) {};

template <class Pair>
inline constexpr bool IsPairV = IsPair<Pair>::value;

template <class Pair, class = std::bool_constant<IsPairV<Pair>>>
struct PairTraits;

template <class Pair>
struct PairTraits<Pair, std::false_type> {};

template <class Pair>
struct PairTraits<Pair, std::true_type> {
  using first = decltype(std::declval<Pair&>().first);
  using second = decltype(std::declval<Pair&>().second);
  using value_type = std::pair<first, second>;
};

template <class Tp>  // All Strings are direct types.
struct IsIndirectType : std::bool_constant<!IsStringV<Tp>> {};

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

template <>
struct IsIndirectType<uint8_t> : std::false_type {};

template <>
struct IsIndirectType<int8_t> : std::false_type {};

template <>
struct IsIndirectType<uint32_t> : std::false_type {};

template <>
struct IsIndirectType<int32_t> : std::false_type {};

template <>
struct IsIndirectType<uint64_t> : std::false_type {};

template <>
struct IsIndirectType<int64_t> : std::false_type {};

template <>
struct IsIndirectType<float> : std::false_type {};

template <>
struct IsIndirectType<double> : std::false_type {};

template <class Tp>
inline constexpr bool IsIndirectTypeV = IsIndirectType<Tp>::value;

}  // namespace liteproto
