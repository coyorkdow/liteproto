//
// Created by Youtao Guo on 2023/6/24.
//

#pragma once

#include <array>
#include <type_traits>
#include <utility>

#ifndef LITE_PROTO_SCAN_FIELD_RANGE_
#define LITE_PROTO_SCAN_FIELD_RANGE_ 192
#endif

static_assert(LITE_PROTO_SCAN_FIELD_RANGE_ % 4 == 0, "LITE_PROTO_SCAN_FIELD_RANGE_ must be the multiple of 4");

namespace liteproto {

template <int32_t N>
using int32_constant = std::integral_constant<int32_t, N>;

template <int32_t N, class = std::enable_if_t<N >= 0>>
struct Seq : int32_constant<N> {};

namespace internal {
using PII = std::pair<int32_t, int32_t>;

template <size_t N, size_t M>
constexpr bool StrLiteralEQ(const char (&s1)[M], const char (&s2)[N]) {
  if (M != N) return false;
  for (size_t i = 0; i < N; i++) {
    if (s1[i] != s2[i]) return false;
  }
  return true;
}

template <class Tp, size_t... S1, size_t... S2>
constexpr auto ConcatSTDArray(std::index_sequence<S1...>, std::index_sequence<S2...>,
                              const std::array<Tp, sizeof...(S1)>& a1, const std::array<Tp, sizeof...(S2)>& a2) {
  using arr = std::array<Tp, sizeof...(S1) + sizeof...(S2)>;
  return arr{a1[S1]..., a2[S2]...};
}

template <size_t Offset, class Tp, size_t N, size_t... S>
constexpr auto SubSTDArray(std::index_sequence<S...>, const std::array<Tp, N>& arr) {
  return std::array<Tp, sizeof...(S)>{arr[S + Offset]...};
}

template <class Tp, size_t S1, size_t S2>
constexpr auto MergeSTDArray(const std::array<Tp, S1>& a1, const std::array<Tp, S2>& a2) {
  if constexpr (S1 == 0) {
    return a2;
  } else if constexpr (S2 == 0) {
    return a1;
  } else {
    using l = std::make_index_sequence<S1 - 1>;
    using r = std::make_index_sequence<S2 - 1>;
    auto rest = MergeSTDArray(SubSTDArray<1>(l{}, a1), SubSTDArray<1>(r{}, a2));
    if (a1[0] < a2[0]) {
      return ConcatSTDArray(std::index_sequence<0, 1>{}, std::make_index_sequence<rest.size()>{},
                            std::array<Tp, 2>{a1[0], a2[0]}, rest);
    } else {
      return ConcatSTDArray(std::index_sequence<0, 1>{}, std::make_index_sequence<rest.size()>{},
                            std::array<Tp, 2>{a2[0], a1[0]}, rest);
    }
  }
}

template <class Tp, size_t N>
constexpr auto SortSTDArray(const std::array<Tp, N>& arr) {
  if constexpr (N <= 1) {
    return arr;
  } else {
    constexpr size_t l = N / 2;
    constexpr size_t r = N - l;
    auto sorted1 = SortSTDArray(SubSTDArray<0>(std::make_index_sequence<l>{}, arr));
    auto sorted2 = SortSTDArray(SubSTDArray<l>(std::make_index_sequence<r>{}, arr));
    return MergeSTDArray(sorted1, sorted2);
  }
}

template <size_t N>
constexpr bool NoDuplicate(const std::array<PII, N>& arr) {
  for (size_t i = 1; i < arr.size(); i++) {
    if (arr[i].first == arr[i - 1].first || arr[i].second == arr[i - 1].second) {
      return false;
    }
  }
  return true;
}

template <class Tp, int32_t Now, int32_t End>
constexpr decltype(auto) GetAllFieldsImpl() {
  if constexpr (Now == End) {
    return std::array<PII, 0>{};
  } else if constexpr (Tp::template FIELD_seq<Now, void> != -1) {
    constexpr auto remains = GetAllFieldsImpl<Tp, Now + 1, End>();
    return ConcatSTDArray(std::make_index_sequence<1>{}, std::make_index_sequence<remains.size()>{},
                          std::array<PII, 1>{PII{Tp::template FIELD_seq<Now, void>, Now}}, remains);
  } else {
    return GetAllFieldsImpl<Tp, Now + 1, End>();
  }
}

template <class Tp>
constexpr decltype(auto) GetAllFields() {
  constexpr size_t round_num = LITE_PROTO_SCAN_FIELD_RANGE_ / 4;
  constexpr auto res1 = GetAllFieldsImpl<Tp, Tp::FIELDS_start, Tp::FIELDS_start + round_num>();
  constexpr auto res2 = GetAllFieldsImpl<Tp, Tp::FIELDS_start + round_num, Tp::FIELDS_start + round_num * 2>();
  constexpr auto res3 = GetAllFieldsImpl<Tp, Tp::FIELDS_start + round_num * 2, Tp::FIELDS_start + round_num * 3>();
  constexpr auto res4 = GetAllFieldsImpl<Tp, Tp::FIELDS_start + round_num * 3, Tp::FIELDS_start + round_num * 4>();
  constexpr auto concat1 =
      ConcatSTDArray(std::make_index_sequence<res1.size()>{}, std::make_index_sequence<res2.size()>{}, res1, res2);
  constexpr auto concat2 =
      ConcatSTDArray(std::make_index_sequence<res3.size()>{}, std::make_index_sequence<res4.size()>{}, res3, res4);
  constexpr auto concat_final = ConcatSTDArray(std::make_index_sequence<concat1.size()>{},
                                               std::make_index_sequence<concat2.size()>{}, concat1, concat2);
  constexpr auto final = SortSTDArray(concat_final);
  static_assert(final.size() == 0 || final[0].first >= 0, "seq number must be greater than or equal to 0");
  static_assert(NoDuplicate(final), "each field must has unique seq number in a same message");
  return final;
}

template <class Tp, int32_t N>
auto SeqNumber(int) -> decltype(Tp::FIELD_seq(int32_constant<N>{}));

template <class Tp, int32_t N>
auto SeqNumber(...) -> int32_constant<-1>;

template <class Tp, int32_t Now, int32_t End>
constexpr decltype(auto) GetAllFields2Impl() {
  if constexpr (Now == End) {
    return std::array<PII, 0>{};
  } else if constexpr (decltype(SeqNumber<Tp, Now>(0))::value != -1) {
    constexpr auto remains = GetAllFields2Impl<Tp, Now + 1, End>();
    return ConcatSTDArray(std::make_index_sequence<1>{}, std::make_index_sequence<remains.size()>{},
                          std::array<PII, 1>{PII{decltype(SeqNumber<Tp, Now>(0))::value, Now}}, remains);
  } else {
    return GetAllFields2Impl<Tp, Now + 1, End>();
  }
}

template <class Tp>
constexpr decltype(auto) GetAllFields2() {
  constexpr size_t round_num = LITE_PROTO_SCAN_FIELD_RANGE_ / 4;
  constexpr auto res1 = GetAllFields2Impl<Tp, Tp::FIELDS_start, Tp::FIELDS_start + round_num>();
  constexpr auto res2 = GetAllFields2Impl<Tp, Tp::FIELDS_start + round_num, Tp::FIELDS_start + round_num * 2>();
  constexpr auto res3 = GetAllFields2Impl<Tp, Tp::FIELDS_start + round_num * 2, Tp::FIELDS_start + round_num * 3>();
  constexpr auto res4 = GetAllFields2Impl<Tp, Tp::FIELDS_start + round_num * 3, Tp::FIELDS_start + round_num * 4>();
  constexpr auto concat1 =
      ConcatSTDArray(std::make_index_sequence<res1.size()>{}, std::make_index_sequence<res2.size()>{}, res1, res2);
  constexpr auto concat2 =
      ConcatSTDArray(std::make_index_sequence<res3.size()>{}, std::make_index_sequence<res4.size()>{}, res3, res4);
  constexpr auto concat_final = ConcatSTDArray(std::make_index_sequence<concat1.size()>{},
                                               std::make_index_sequence<concat2.size()>{}, concat1, concat2);
  constexpr auto final = SortSTDArray(concat_final);
  static_assert(final.size() == 0 || final[0].first >= 0, "seq number must be greater than or equal to 0");
  static_assert(NoDuplicate(final), "each field must has unique seq number in a same message");
  return final;
}

}  // namespace internal

}  // namespace liteproto