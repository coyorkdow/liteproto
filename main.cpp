#include <iostream>
#include <type_traits>

template <int32_t N>
using int32_constant = std::integral_constant<int32_t, N>;

#define FIELD(name)                                                            \
  name##_;                                                                     \
                                                                               \
 public:                                                                       \
  const decltype(name##_)& name() const { return name##_; }                    \
  decltype(name##_)& mutable_##name() { return name##_; }                      \
  void set_##name(decltype(name##_) v) { name##_ = std::move(v); }             \
  static constexpr auto FIELD_name(int32_constant<__LINE__>) { return #name; } \
  auto FIELD_type(int32_constant<__LINE__>)->decltype(test_field_);            \
  decltype(name##_)& FIELD_value(int32_constant<__LINE__>) { return name##_; } \
  template <>                                                                  \
  static constexpr int32_t FIELD_seq<__LINE__>

#define DECLARE_FIELDS                     \
 public:                                   \
  template <int32_t N>                     \
  static constexpr int32_t FIELD_seq = -1; \
                                           \
 private:

class TestMessage {
  DECLARE_FIELDS

  int FIELD(test_field) = 0;
  double FIELD(test_field2) = 1;

  //  int test_field_;
  //  const decltype(test_field_)& test_field() const { return test_field_; }
  //  void set_test_field(decltype(test_field_) v) { test_field_ = std::move(v); }
  //  static constexpr decltype(auto) FIELD_name(int32_constant<11>) { return "test_field"; }
  //  auto FIELD_type(int32_constant<11>) -> decltype(test_field_);
  //  template <>
  //  static constexpr auto FIELD_seq<11> = 0;
};

template <class Tp, size_t... S1, size_t... S2>
constexpr auto ConcatSTDArray(std::index_sequence<S1...>, std::index_sequence<S2...>,
                              const std::array<Tp, sizeof...(S1)>& a1, const std::array<Tp, sizeof...(S2)>& a2) {
  using arr = std::array<Tp, sizeof...(S1) + sizeof...(S2)>;
  return arr{a1[S1]..., a2[S2]...};
}

template <int32_t Now, int32_t End>
constexpr decltype(auto) GetAllFields() {
  if constexpr (Now == End) {
    return std::array<int32_t, 0>{};
  } else if constexpr (TestMessage::FIELD_seq<Now> != -1) {
    constexpr auto remains = GetAllFields<Now + 1, End>();
    return ConcatSTDArray(std::make_index_sequence<1>{}, std::make_index_sequence<remains.size()>{},
                          std::array<int32_t, 1>{Now}, remains);
  } else {
    return GetAllFields<Now + 1, End>();
  }
}

template <class Tp, size_t N>
size_t Size(Tp (&arr)[N]) {
  return N;
}

int main() {
  constexpr const auto arr = GetAllFields<0, 100>();
  static_assert(arr.size() == 2);
  //  static_assert(arr[0] == 35);
  //  static_assert(arr[1] == 36);
  //  using arr = int32_t[];
  //  arr t {1, 2, 3};
  //  std::cout << Size(t);

  return 0;
}
