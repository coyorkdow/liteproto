#include <iostream>
#include <map>
#include <type_traits>

#include "basic_message.h"

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

#define DECLARE_FIELDS                     \
 public:                                   \
  template <int32_t N, class>              \
  static constexpr int32_t FIELD_seq = -1; \
                                           \
 private:

#define FIELD_X(name)                                                                    \
  name##_;                                                                               \
                                                                                         \
 public:                                                                                 \
  const decltype(name##_)& name() const { return name##_; }                              \
  decltype(name##_)& mutable_##name() { return name##_; }                                \
  void set_##name(decltype(name##_) v) { name##_ = std::move(v); }                       \
  static constexpr decltype(auto) FIELD_name(int32_constant<__LINE__>) { return #name; } \
  auto FIELD_type(int32_constant<__LINE__>)->decltype(test_field_);                      \
  decltype(name##_)& FIELD_value(int32_constant<__LINE__>) { return name##_; }           \
  static auto FIELD_seq(int32_constant<__LINE__>)

template <class Msg>
class MessageBasic {
 public:
  static constexpr auto GetAllFieldsIndex() { return ::GetAllFields<Msg>(); }

  template <size_t I = 0, size_t N>
  static constexpr auto GetFieldIndexByName(const char (&str)[N]) {
    constexpr auto indices = GetAllFieldsIndex();
    if constexpr (I >= indices.size()) {
      return PII{-1, -1};
    } else if (StrLiteralEQ(str, Msg::FIELD_name(int32_constant<indices[I].second>{}))) {
      return indices[I];
    } else {
      return GetFieldIndexByName<I + 1>(str);
    }
  }

  static constexpr int32_t FIELDS_start = __LINE__;

 private:
};

#define MESSAGE(name) class name : public MessageBasic<name>

//template <class Msg>
//class MessageBasic2 {
// public:
//  static constexpr auto GetAllFields() { return ::GetAllFields2<Msg>(); }
//
//  static constexpr int32_t FIELDS_start = __LINE__;
//};
//
//#define MESSAGE2(name) class name : public MessageBasic2<name>

MESSAGE(TestMessage) {
  DECLARE_FIELDS

  int FIELD(test_field) = 1;
  float FIELD(test_field3) = 3;
  double FIELD(test_field2) = 2;
};

//MESSAGE2(TestMessage2) {
//  int FIELD_X(test_field)->No<0>;
//  double FIELD_X(test_field2)->No<1>;
//};

int main() {
  constexpr auto arr = TestMessage::GetAllFieldsIndex();

  static_assert(arr.size() == 3);
  for (auto [a, b] : arr) {
    std::cout << a << ' ' << b << '\n';
  }
  constexpr char test_field_name[] = "test_field";
  constexpr char test_field2_name[] = "test_field2";

  static_assert(StrLiteralEQ(TestMessage::FIELD_name(int32_constant<arr[0].second>{}), test_field_name));
  static_assert(StrLiteralEQ(TestMessage::FIELD_name(int32_constant<arr[1].second>{}), test_field2_name));

  constexpr auto index0 = TestMessage::GetFieldIndexByName(test_field_name);
  static_assert(index0 == arr[0]);
  constexpr auto index1 = TestMessage::GetFieldIndexByName(test_field2_name);
  static_assert(index1 == arr[1]);
  auto v = TestMessage::GetFieldIndexByName("test_");
  std::cout << v.first << ' ' << v.second;
  return 0;
}
