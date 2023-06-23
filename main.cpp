#include <iostream>
#include <type_traits>

#include "basic_message.h"

template <int32_t N>
using int32_constant = std::integral_constant<int32_t, N>;

#define FIELD(name)                                                                      \
  name##_;                                                                               \
                                                                                         \
 public:                                                                                 \
  const decltype(name##_)& name() const { return name##_; }                              \
  decltype(name##_)& mutable_##name() { return name##_; }                                \
  void set_##name(decltype(name##_) v) { name##_ = std::move(v); }                       \
  static constexpr decltype(auto) FIELD_name(int32_constant<__LINE__>) { return #name; } \
  auto FIELD_type(int32_constant<__LINE__>)->decltype(test_field_);                      \
  decltype(name##_)& FIELD_value(int32_constant<__LINE__>) { return name##_; }           \
  template <class Tp>                                                                    \
  static constexpr int32_t FIELD_seq<__LINE__, Tp>

#define DECLARE_FIELDS                              \
 public:                                            \
  template <int32_t N, class>                       \
  static constexpr int32_t FIELD_seq = -1;          \
  static constexpr int32_t FIELDS_start = __LINE__; \
                                                    \
 private:

#define MESSAGE(name) class name

MESSAGE(TestMessage) {
  DECLARE_FIELDS

  int FIELD(test_field) = 0;
  double FIELD(test_field2) = 1;
};

template <size_t N>
constexpr bool str_literal_eq(const char (&s1)[N], const char (&s2)[N]) {
  for (size_t i = 0; i < N; i++) {
    if (s1[i] != s2[i]) return false;
  }
  return true;
}

int main() {
  constexpr const auto arr = GetAllFields<TestMessage>();
  static_assert(arr.size() == 2);
  constexpr char test_field_name[] = "test_field";
  constexpr char test_field2_name[] = "test_field2";

  static_assert(str_literal_eq(TestMessage::FIELD_name(int32_constant<arr[0]>{}), test_field_name));
  static_assert(str_literal_eq(TestMessage::FIELD_name(int32_constant<arr[1]>{}), test_field2_name));

  return 0;
}
