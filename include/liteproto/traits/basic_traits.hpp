//
// Created by Youtao Guo on 2023/6/30.
//

#pragma once

#include <type_traits>

using uint8_t = std::uint8_t;
using int8_t = std::int8_t;
using uint32_t = std::uint32_t;
using int32_t = std::int32_t;
using uint64_t = std::uint64_t;
using int64_t = std::int64_t;
using size_t = std::size_t;

static_assert(!std::is_same_v<int8_t, char>, "liteproto requires that int8_t is a type different than char");

namespace liteproto {

namespace details {

using std::begin;
using std::end;

template <class C, class = std::enable_if_t<std::is_convertible_v<std::invoke_result_t<decltype(&C::size), const C>, size_t>>>
auto HasSize(int) -> std::true_type;

template <class>
auto HasSize(float) -> std::false_type;

template <class C, size_t = std::tuple_size<C>::value>
auto HasConstexprSize(int) -> std::true_type;

template <class C, class = std::enable_if_t<std::is_array_v<C>>>
auto HasConstexprSize(int) -> std::true_type;

template <class>
auto HasConstexprSize(float) -> std::false_type;

template <class C, class V, class = decltype(std::declval<C>().resize(std::declval<size_t>())),
          class = decltype(std::declval<C>().resize(std::declval<size_t>(), std::declval<V>()))>
auto HasReSize(int) -> std::true_type;

template <class, class>
auto HasReSize(float) -> std::false_type;

template <class C, class = std::enable_if_t<std::is_convertible_v<std::invoke_result_t<decltype(&C::capacity), const C>, size_t>>>
auto HasCapacity(int) -> std::true_type;

template <class>
auto HasCapacity(float) -> std::false_type;

template <class C, class = std::enable_if_t<std::is_convertible_v<std::invoke_result_t<decltype(&C::empty), const C>, bool>>>
auto HasEmpty(int) -> std::true_type;

template <class>
auto HasEmpty(float) -> std::false_type;

template <class C, class = std::invoke_result_t<decltype(&C::clear), C>>
auto HasClear(int) -> std::true_type;

template <class>
auto HasClear(float) -> std::false_type;

template <class C, class V, class = decltype(std::declval<C>().push_back(std::declval<V>()))>
auto HasPushBack(int) -> std::true_type;

template <class, class>
auto HasPushBack(float) -> std::false_type;

template <class C, class = std::invoke_result_t<decltype(&C::pop_back), C>>
auto HasPopBack(int) -> std::true_type;

template <class>
auto HasPopBack(float) -> std::false_type;

template <class C, class Char, class = std::enable_if_t<std::is_same_v<const Char*, std::invoke_result_t<decltype(&C::c_str), const C>>>>
auto HasCStr(int) -> std::true_type;

template <class, class>
auto HasCStr(float) -> std::false_type;

template <class C, class Char, class = decltype(std::declval<C>().append(std::declval<const Char*>(), std::declval<size_t>()))>
auto HasAppend(int) -> std::true_type;

template <class, class>
auto HasAppend(float) -> std::false_type;

template <class C, class V,
          class = std::enable_if_t<std::is_same_v<V*, decltype(std::declval<C>().data())> &&
                                   (std::is_same_v<V*, decltype(std::declval<const C>().data())> ||
                                    std::is_same_v<const V*, decltype(std::declval<const C>().data())>)>>
auto HasData(int) -> std::true_type;

template <class C, class V>
auto HasData(float) -> std::false_type;

template <class C, class K, class V, class Iter = decltype(std::declval<C>().find(std::declval<K>())),
          class = std::enable_if_t<std::is_same_v<const K, decltype(std::declval<Iter>()->first)> &&
                                   std::is_same_v<V, decltype(std::declval<Iter>()->second)>>>
auto HasFind(int) -> std::true_type;

template <class C, class K, class V>
auto HasFind(float) -> std::false_type;

template <class It,
          class = std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<It>::iterator_category>>>
auto IsForwardIterator(int) -> std::true_type;

template <class>
auto IsForwardIterator(float) -> std::false_type;

template <class It, class = std::enable_if_t<
                        std::is_base_of_v<std::bidirectional_iterator_tag, typename std::iterator_traits<It>::iterator_category>>>
auto IsBidirectionalIterator(int) -> std::true_type;

template <class>
auto IsBidirectionalIterator(float) -> std::false_type;

template <class C, class It = decltype(begin(std::declval<C>())), class = decltype(std::declval<C>().erase(std::declval<It>()))>
auto HasErase(int) -> std::true_type;

template <class>
auto HasErase(float) -> std::false_type;

template <class C, class V, class It = decltype(begin(std::declval<C>())),
          class = decltype(std::declval<C>().insert(std::declval<It>(), std::declval<V>()))>
auto HasInsert(int) -> std::true_type;

template <class, class>
auto HasInsert(float) -> std::false_type;

// For builtin array compatible. All traits about the iterable and subscript operator use reference in std::declval;

template <class Tp, class = std::enable_if_t<std::is_same_v<decltype(begin(std::declval<Tp&>())), decltype(end(std::declval<Tp&>()))>>>
auto IsForwardIterable(int) -> decltype(IsForwardIterator<decltype(begin(std::declval<Tp&>()))>(0));

template <class>
auto IsForwardIterable(float) -> std::false_type;

template <class Tp, class = std::enable_if_t<std::is_same_v<decltype(begin(std::declval<Tp&>())), decltype(end(std::declval<Tp&>()))>>>
auto IsBidirectionalIterable(int) -> decltype(IsBidirectionalIterator<decltype(begin(std::declval<Tp&>()))>(0));

template <class>
auto IsBidirectionalIterable(float) -> std::false_type;

template <class C, class V,
          class = std::enable_if_t<std::is_convertible_v<decltype(std::declval<C&>()[std::declval<size_t>()]), V&> &&
                                   std::is_convertible_v<decltype(std::declval<const C&>()[std::declval<size_t>()]), const V&>>>
auto HasSubscript(int) -> std::true_type;

template <class, class>
auto HasSubscript(float) -> std::false_type;

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

template <class Tp>
struct is_bidirectional_iterator : decltype(details::IsBidirectionalIterator<Tp>(0)) {};

template <class Tp>
inline constexpr bool is_bidirectional_iterator_v = is_bidirectional_iterator<Tp>::value;

template <class C>
struct is_forward_iterable : decltype(details::IsForwardIterable<C>(0)) {};

template <class C>
inline constexpr bool is_forward_iterable_v = is_forward_iterable<C>::value;

template <class C>
struct is_bidirectional_iterable : decltype(details::IsBidirectionalIterable<C>(0)) {};

template <class C>
inline constexpr bool is_bidirectional_iterable_v = is_bidirectional_iterable<C>::value;

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

template <class C, class K, class V>
struct has_find : decltype(details::HasFind<C, K, V>(0)) {};

template <class C, class K, class V>
inline constexpr bool has_find_v = has_find<C, K, V>::value;

template <class C>
struct is_char : std::false_type {};

template <>
struct is_char<char> : std::true_type {};

template <>
struct is_char<wchar_t> : std::true_type {};

template <>
struct is_char<char16_t> : std::true_type {};

template <>
struct is_char<char32_t> : std::true_type {};

template <class C>
struct is_char<const C> : is_char<C> {};

template <class C>
struct is_char<volatile C> : is_char<C> {};

template <class C>
struct is_char<const volatile C> : is_char<C> {};

template <class C>
inline constexpr bool is_char_v = is_char<C>::value;

}  // namespace liteproto
