#include <iostream>
#include <map>
#include <tuple>
#include <type_traits>

#include "basic_message.hpp"

#define FIELD(name)                                                                          \
  name##_;                                                                                   \
                                                                                             \
 public:                                                                                     \
  constexpr const decltype(name##_)& name() const { return name##_; }                        \
  decltype(name##_)& mutable_##name() { return name##_; }                                    \
  void set_##name(decltype(name##_) v) { name##_ = std::move(v); }                           \
  static constexpr decltype(auto) FIELD_name(int32_constant<__LINE__>) { return #name; }     \
  auto FIELD_type(int32_constant<__LINE__>)->decltype(test_field_);                          \
  constexpr decltype(auto) FIELD_value(int32_constant<__LINE__>) { return (name##_); }       \
  constexpr decltype(auto) FIELD_value(int32_constant<__LINE__>) const { return (name##_); } \
  template <class Tp>                                                                        \
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

// template <class Msg>
// class MessageBasic2 {
//  public:
//   static constexpr auto GetAllFields() { return ::GetAllFields2<Msg>(); }
//
//   static constexpr int32_t FIELDS_start = __LINE__;
// };
//
// #define MESSAGE2(name) class name : public MessageBasic2<name>

MESSAGE(TestMessage) {
  DECLARE_FIELDS

  int FIELD(test_field) = 1;
  double FIELD(test_field2) = 2;
  float FIELD(test_field3) = 3;

  constexpr TestMessage(int v1, double v2, float v3) : test_field_(v1), test_field2_(v2), test_field3_(v3) {}
};

// MESSAGE2(TestMessage2) {
//   int FIELD_X(test_field)->No<0>;
//   double FIELD_X(test_field2)->No<1>;
// };

int main() {
  constexpr auto arr = TestMessage::FieldsIndices::value;
  constexpr char test_field_name[] = "test_field";
  constexpr char test_field2_name[] = "test_field2";

  static_assert(StrLiteralEQ(TestMessage::FIELD_name(int32_constant<arr[0].second>{}), test_field_name));
  static_assert(StrLiteralEQ(TestMessage::FIELD_name(int32_constant<arr[1].second>{}), test_field2_name));

  constexpr auto index0 = TestMessage::GetFieldIndexByName(test_field_name);
  static_assert(index0 == arr[0]);
  constexpr auto index1 = TestMessage::GetFieldIndexByName(test_field2_name);
  static_assert(index1 == arr[1]);
  auto v = TestMessage::GetFieldIndexByName("test_field3");
  std::cout << v.first << ' ' << v.second;

  TestMessage msg{1, 2.5, 3.33};
  auto dumped = msg.DumpTuple();
  static_assert(std::tuple_size_v<decltype(dumped)> == 3);
  std::cout << std::get<0>(dumped) << ' ' << std::get<1>(dumped) << ' ' << std::get<2>(dumped);
  msg.test_field();
  return 0;
}
