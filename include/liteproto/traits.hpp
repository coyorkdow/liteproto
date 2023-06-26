//
// Created by Youtao Guo on 2023/6/25.
//

#pragma once

#include <array>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace liteproto {

namespace details {
template <class C,
          class = std::enable_if_t<std::is_convertible_v<std::invoke_result_t<decltype(&C::size), const C>, size_t>>>
auto HasSize(int) -> std::true_type;

template <class>
auto HasSize(float) -> std::false_type;

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

template <template <class...> class C, class Tp, class = decltype(std::declval<C<Tp>&>().push_back(std::declval<Tp>()))>
auto HasPushBack(int) -> std::true_type;

template <template <class...> class, class>
auto HasPushBack(float) -> std::false_type;

template <class C, class = std::invoke_result_t<decltype(&C::pop_back), C>>
auto HasPopBack(int) -> std::true_type;

template <class>
auto HasPopBack(float) -> std::false_type;

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

template <template <class...> class C, class Tp, class It = decltype(begin(std::declval<C<Tp>&>())),
          class = decltype(std::declval<C<Tp>&>().insert(std::declval<It>(), std::declval<Tp>()))>
auto HasInsert(int) -> std::true_type;

template <template <class...> class, class>
auto HasInsert(float) -> std::false_type;

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

template <template <class...> class C, class Tp>
struct has_push_back : decltype(details::HasPushBack<C, Tp>(0)) {};

template <template <class...> class C, class Tp>
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

template <template <class...> class C, class Tp>
struct has_insert : decltype(details::HasInsert<C, Tp>(0)) {};

template <template <class...> class C, class Tp>
inline constexpr bool has_insert_v = has_insert<C, Tp>::value;

// For the traits of our own types, we use camel case instead of underscore case.

template <class Tp, class Cond = void>
struct IsList : std::false_type {};

template <template <class...> class C, class V>
struct IsList<
    C<V>, std::enable_if_t<has_push_back_v<C, V> && has_pop_back_v<C<V>> && has_size_v<C<V>> && has_empty_v<C<V>> &&
                           has_clear_v<C<V>> && is_forward_iterable_v<C<V>> && has_insert_v<C, V> && has_erase_v<C<V>>>>
    : std::true_type {
  using container_type = C<V>;
  using value_type = V;
};

template <template <class...> class C, class V>
struct IsList<const C<V>, std::enable_if_t<has_push_back_v<C, V> && has_pop_back_v<C<V>> && has_size_v<C<V>> &&
                                           has_empty_v<C<V>> && has_clear_v<C<V>> && is_forward_iterable_v<C<V>> &&
                                           has_insert_v<C, V> && has_erase_v<C<V>>>> : std::true_type {
  using container_type = const C<V>;
  using value_type = V;
};

template <class Tp>
inline constexpr bool IsListV = IsList<Tp>::value;

template <class Tp, class = std::bool_constant<IsListV<std::remove_reference_t<Tp>>>>
struct ListTraits;

template <class Tp>
struct ListTraits<Tp, std::false_type> {};

template <class Tp>
struct ListTraits<Tp, std::true_type> {
  using container_type = typename IsList<std::remove_reference_t<Tp>>::container_type;
  using value_type = typename IsList<std::remove_reference_t<Tp>>::value_type;
};

namespace internal_test {

static_assert(std::is_same_v<std::vector<int>, typename ListTraits<std::vector<int>&&>::container_type>);
static_assert(std::is_same_v<int, typename ListTraits<std::vector<int>&&>::value_type>);

static_assert(std::is_same_v<const std::deque<int>, typename ListTraits<const std::deque<int>&>::container_type>);
static_assert(std::is_same_v<int, typename ListTraits<const std::vector<int>&&>::value_type>);

static_assert(is_forward_iterator_v<std::list<int>::iterator>);
static_assert(!is_forward_iterator_v<std::list<int>>);
static_assert(is_forward_iterable_v<std::list<int>>);

static_assert(has_size_v<std::vector<int>>);
static_assert(has_empty_v<std::vector<int>>);
static_assert(has_clear_v<std::vector<int>>);
static_assert(has_capacity_v<const std::vector<int>>);

static_assert(has_push_back_v<std::vector, int>);
static_assert(!has_push_back_v<std::queue, int>);

static_assert(has_pop_back_v<std::vector<int>>);
static_assert(!has_pop_back_v<std::queue<int>>);

static_assert(has_erase_v<std::vector<int>>);
static_assert(has_erase_v<std::map<int, std::string>>);
static_assert(!has_erase_v<std::array<int, 1>>);

static_assert(has_insert_v<std::vector, int>);

}  // namespace internal_test

}  // namespace liteproto
