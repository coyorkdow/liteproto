//
// Created by Youtao Guo on 23/6/23.
//

#pragma once

#include <array>
#include <iostream>
#include <type_traits>

template <class Tp, size_t... S1, size_t... S2>
constexpr auto ConcatSTDArray(std::index_sequence<S1...>, std::index_sequence<S2...>,
                              const std::array<Tp, sizeof...(S1)>& a1, const std::array<Tp, sizeof...(S2)>& a2) {
  using arr = std::array<Tp, sizeof...(S1) + sizeof...(S2)>;
  return arr{a1[S1]..., a2[S2]...};
}

template <class Tp, int32_t Now, int32_t End>
constexpr decltype(auto) GetAllFieldsImpl() {
  if constexpr (Now == End) {
    return std::array<int32_t, 0>{};
  } else if constexpr (Tp::template FIELD_seq<Now, void> != -1) {
    constexpr auto remains = GetAllFieldsImpl<Tp, Now + 1, End>();
    return ConcatSTDArray(std::make_index_sequence<1>{}, std::make_index_sequence<remains.size()>{},
                          std::array<int32_t, 1>{Now}, remains);
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
                                          res1,
                                          res2);
  constexpr auto concat2 = ConcatSTDArray(std::make_index_sequence<res3.size()>{},
                                          std::make_index_sequence<res4.size()>{},
                                          res3,
                                          res4);
  return ConcatSTDArray(std::make_index_sequence<concat1.size()>{},
                        std::make_index_sequence<concat2.size()>{},
                        concat1,
                        concat2);
}