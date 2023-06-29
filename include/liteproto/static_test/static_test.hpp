//
// Created by Youtao Guo on 2023/6/30.
//

#pragma once

#include "liteproto/traits.hpp"

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

}  // namespace liteproto::internal_test
