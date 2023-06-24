//
// Created by Youtao Guo on 23/6/23.
//

#pragma once

#include <array>
#include <iostream>
#include <type_traits>

template <int32_t N>
using int32_constant = std::integral_constant<int32_t, N>;

template <int32_t N>
using No = int32_constant<N>;

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

template<size_t Offset, class Tp, size_t N, size_t... S>
constexpr auto SubSTDArray(std::index_sequence<S...>, const std::array<Tp, N>& arr) {
  return std::array<Tp, sizeof...(S)> {arr[S + Offset]...};
}

template <class Tp, size_t... S1, size_t... S2>
constexpr auto MergeSTDArray(std::index_sequence<S1...>, std::index_sequence<S2...>,
                              const std::array<Tp, sizeof...(S1)>& a1, const std::array<Tp, sizeof...(S2)>& a2) {
  if constexpr (sizeof...(S1) == 0) {
    return a2;
  } else if constexpr (sizeof...(S2) == 0) {
    return a1;
  } else {
    using l = std::make_index_sequence<sizeof...(S1) - 1>;
    using r = std::make_index_sequence<sizeof...(S2) - 1>;
    auto rest = MergeSTDArray(l{}, r{}, SubSTDArray<1>(l{}, a1), SubSTDArray<1>(r{}, a2));
    if (a1[0] < a2[0]) {
      return ConcatSTDArray(std::index_sequence<0, 1>{},
                            std::make_index_sequence<rest.size()>{},
                            std::array<Tp, 2>{a1[0], a2[0]}, rest);
    } else {
      return ConcatSTDArray(std::index_sequence<0, 1>{},
                            std::make_index_sequence<rest.size()>{},
                            std::array<Tp, 2>{a2[0], a1[0]}, rest);
    }
  }
}


template <class Tp, size_t... S>
constexpr auto SortSTDArray(std::index_sequence<S...>, const std::array<Tp, sizeof...(S)>& arr) {
  if constexpr (sizeof...(S) <= 1) {
    return arr;
  } else {
    constexpr size_t l = sizeof...(S) / 2;
    constexpr size_t r = sizeof...(S) - l;
      auto sorted1 =
          SortSTDArray(std::make_index_sequence<l>{}, SubSTDArray<0>(std::make_index_sequence<l>{}, arr));
      auto sorted2 =
          SortSTDArray(std::make_index_sequence<r>{}, SubSTDArray<l>(std::make_index_sequence<r>{}, arr));
      return MergeSTDArray(std::make_index_sequence<sorted1.size()>{}, std::make_index_sequence<sorted2.size()>{},
                           sorted1, sorted2);
  }
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
  constexpr auto res1 = GetAllFieldsImpl<Tp, Tp::FIELDS_start, Tp::FIELDS_start + 128>();
  constexpr auto res2 = GetAllFieldsImpl<Tp, Tp::FIELDS_start + 128, Tp::FIELDS_start + 256>();
  constexpr auto res3 = GetAllFieldsImpl<Tp, Tp::FIELDS_start + 256, Tp::FIELDS_start + 384>();
  constexpr auto res4 = GetAllFieldsImpl<Tp, Tp::FIELDS_start + 384, Tp::FIELDS_start + 512>();
  constexpr auto concat1 = ConcatSTDArray(std::make_index_sequence<res1.size()>{},
                                          std::make_index_sequence<res2.size()>{},
                                          res1, res2);
  constexpr auto concat2 = ConcatSTDArray(std::make_index_sequence<res3.size()>{},
                                          std::make_index_sequence<res4.size()>{},
                                          res3, res4);
  constexpr auto concat_final = ConcatSTDArray(std::make_index_sequence<concat1.size()>{},
                                               std::make_index_sequence<concat2.size()>{},
                                               concat1, concat2);
  return SortSTDArray(std::make_index_sequence<concat_final.size()>{}, concat_final);
}

template <class Tp, int32_t N>
auto SeqNumber(int) -> decltype(Tp::FIELD_seq(int32_constant<N>{}));

template <class Tp, int32_t N>
auto SeqNumber(...) -> int32_constant<-1>;

//template <class Tp, int32_t Now, int32_t End>
//constexpr decltype(auto) GetAllFields2Impl() {
//  if constexpr (Now == End) {
//    return std::array<PII, 0>{};
//  } else if constexpr (decltype(SeqNumber<Tp, Now>(0))::value != -1) {
//    constexpr auto remains = GetAllFields2Impl<Tp, Now + 1, End>();
//    return ConcatSTDArray(std::make_index_sequence<1>{}, std::make_index_sequence<remains.size()>{},
//                          std::array<PII, 1>{PII{decltype(SeqNumber<Tp, Now>(0))::value, Now}}, remains);
//  } else {
//    return GetAllFields2Impl<Tp, Now + 1, End>();
//  }
//}
//
//template <class Tp>
//constexpr decltype(auto) GetAllFields2() {
//  constexpr auto res1 = GetAllFields2Impl<Tp, Tp::FIELDS_start, Tp::FIELDS_start + 128>();
//  constexpr auto res2 = GetAllFields2Impl<Tp, Tp::FIELDS_start + 128, Tp::FIELDS_start + 256>();
//  constexpr auto res3 = GetAllFields2Impl<Tp, Tp::FIELDS_start + 256, Tp::FIELDS_start + 384>();
//  constexpr auto res4 = GetAllFields2Impl<Tp, Tp::FIELDS_start + 384, Tp::FIELDS_start + 512>();
//  constexpr auto concat1 = ConcatSTDArray(std::make_index_sequence<res1.size()>{},
//                                          std::make_index_sequence<res2.size()>{},
//                                          res1,
//                                          res2);
//  constexpr auto concat2 = ConcatSTDArray(std::make_index_sequence<res3.size()>{},
//                                          std::make_index_sequence<res4.size()>{},
//                                          res3,
//                                          res4);
//  return ConcatSTDArray(std::make_index_sequence<concat1.size()>{},
//                        std::make_index_sequence<concat2.size()>{},
//                        concat1,
//                        concat2);
//}