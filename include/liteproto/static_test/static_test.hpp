//
// Created by Youtao Guo on 2023/6/30.
//

#pragma once

#include "liteproto/reflect.hpp"
#include "liteproto/traits/traits.hpp"

namespace liteproto::internal_test {

static_assert(IsPairV<std::pair<int, IsPair<void>>>);
static_assert(!IsPairV<void>);
using p = std::pair<int, IsPair<void>>;
static_assert(std::is_same_v<typename PairTraits<p>::first, int>);
static_assert(std::is_same_v<typename PairTraits<p>::second, IsPair<void>>);

static_assert(has_c_str_v<std::string, char>);
static_assert(has_append_v<std::string, char>);
static_assert(IsStringV<std::string>);
static_assert(IsStringV<const std::string>);
static_assert(IsListV<std::vector<int>>);
static_assert(IsListV<std::string>);
static_assert(std::is_same_v<typename ListTraits<std::string>::container_type, std::string>);
static_assert(std::is_same_v<typename ListTraits<std::string>::value_type, char>);
static_assert(IsListV<const std::vector<int>>);
static_assert(has_subscript_v<std::vector<int>, int>);

static_assert(has_subscript_v<int[1][2][3], int[2][3]>);
static_assert(IsArrayV<int[1][2][3]>);
static_assert(std::is_same_v<typename ArrayTraits<int[1][2][3]>::value_type, int[2][3]>);

static_assert(internal::is_array<std::array<int, 6>, int>);
static_assert(IsArrayV<std::array<int, 6>>);
static_assert(std::is_same_v<typename ArrayTraits<std::array<int, 0>>::value_type, int>);

static_assert(!IsListV<std::vector<int>&&>);
static_assert(IsListV<std::vector<int>>);
static_assert(!IsListV<const std::deque<int>&>);
static_assert(IsListV<const std::deque<int>>);

static_assert(std::is_same_v<typename ListTraits<std::string>::value_type, char>);

static_assert(is_forward_iterator_v<std::list<int>::iterator>);
static_assert(!is_forward_iterator_v<std::list<int>>);
static_assert(is_forward_iterable_v<std::list<int>>);

static_assert(has_size_v<std::vector<int>>);
static_assert(has_empty_v<std::vector<int>>);
static_assert(has_clear_v<std::vector<int>>);
static_assert(has_capacity_v<const std::vector<int>>);

static_assert(has_push_back_v<std::vector<int>, int>);
static_assert(!has_push_back_v<std::queue<int>, int>);

static_assert(has_pop_back_v<std::vector<int>>);
static_assert(!has_pop_back_v<std::queue<int>>);

static_assert(has_erase_v<std::vector<int>>);
static_assert(has_erase_v<std::map<int, std::string>>);
static_assert(!has_erase_v<std::array<int, 1>>);

static_assert(has_insert_v<std::vector<int>, int>);

static_assert(has_constexpr_size_v<int[2][3][4]>);
static_assert(has_constexpr_size_v<std::array<int, 5>>);
static_assert(!has_constexpr_size_v<std::vector<int>>);

static_assert(has_find_v<std::map<std::string, int>, std::string, int>);
static_assert(has_insert_v<std::map<std::string, int>, std::pair<std::string, int>>);

#if defined(__cpp_concepts)

template <class I>
concept Referenceable = requires(I& i) { i; };

template <class I>
concept LegacyIterator = requires(I i) {
  { *i } -> Referenceable;
  { ++i } -> std::same_as<I&>;
  { *i++ } -> Referenceable;
} && std::copyable<I>;

template <class I>
concept LegacyInputIterator = LegacyIterator<I> && std::equality_comparable<I> && requires(I i) {
  typename std::incrementable_traits<I>::difference_type;
  typename std::indirectly_readable_traits<I>::value_type;
  typename std::common_reference_t<std::iter_reference_t<I>&&,
                                   typename std::indirectly_readable_traits<I>::value_type&>;
  *i++;
  typename std::common_reference_t<decltype(*i++)&&, typename std::indirectly_readable_traits<I>::value_type&>;
  requires std::signed_integral<typename std::incrementable_traits<I>::difference_type>;
};

template <class It>
concept LegacyForwardIterator =
    LegacyInputIterator<It> && std::constructible_from<It> && std::is_reference_v<std::iter_reference_t<It>> &&
    std::same_as<std::remove_cvref_t<std::iter_reference_t<It>>,
                 typename std::indirectly_readable_traits<It>::value_type> &&
    requires(It it) {
      { it++ } -> std::convertible_to<const It&>;
      { *it++ } -> std::same_as<std::iter_reference_t<It>>;
    };

static_assert(LegacyForwardIterator<Iterator<std::string>>);

#endif

}  // namespace liteproto::internal_test
