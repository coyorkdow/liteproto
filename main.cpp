#include <cassert>
#include <iostream>
#include <map>
#include <tuple>
#include <type_traits>

#include "liteproto/liteproto.hpp"

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
  int FIELD(test_field)->Seq<1>;
  double FIELD(test_field2)->Seq<2>;
  float FIELD(test_field3)->Seq<3>;

  constexpr TestMessage(int v1, double v2, float v3) : test_field_(v1), test_field2_(v2), test_field3_(v3) {}
};

#include <functional>

int main() {
  using namespace liteproto;

  std::vector<std::string> strlist;
  auto list = AsList(strlist);
  for (int i = 0; i < 10; i++) {
    list.push_back(std::to_string(i));
  }
  for (int i = 0; i < 5; i++) {
    list.pop_back();
  }
  for (auto& v : list) {
    std::cout << v << ' ';
    v.append("x");
  }
  std::cout << std::endl;
  for (auto& v : list) {
    std::cout << v << ' ';
  }
  std::cout << std::endl;
  const auto& cref = strlist;
  auto const_list = AsList(cref);
  for (int i = 0; i < const_list.size(); i++) {
    assert(const_list[i].back() == 'x');
  }
  for (auto it = const_list.begin(); it != const_list.end(); it++) {
    std::cout << *it << ' ';
  }
  std::cout << std::endl;

  Iterator<std::string> it = list.begin();

  std::list<std::string> anol;
  list = AsList(anol);
  assert(it != list.begin());
  assert(list.size() == 0);
  for (int i = 0; i < 5; i++) {
    list.insert(list.end(), "no" + std::to_string(i));
  }
  for (auto& v : list) {
    std::cout << v << ' ';
  }
  std::cout << std::endl;
  list.resize(10, "abc");
  for (auto& v : anol) {
    std::cout << v << ' ';
  }
  std::cout << std::endl;
  while (!list.empty()) {
    std::cout << *it << ' ';
    it = list.erase(list.begin());
  }
  list.insert(list.begin(), "new str");
  const_list = list;
  std::cout << const_list.begin()->c_str();

  std::cout << std::endl;

  TestMessage msg{1, 2.5, 3.33};
  auto dumped = msg.DumpTuple();
  static_assert(std::tuple_size_v<decltype(dumped)> == 3);
  std::cout << std::get<0>(dumped) << ' ' << std::get<1>(dumped) << ' ' << std::get<2>(dumped) << std::endl;

  msg.set_test_field(5);
  msg.set_test_field2(msg.test_field2() + 1);
  msg.set_test_field3(msg.test_field3() + 2);
  dumped = msg.DumpTuple();
  std::cout << std::get<0>(dumped) << ' ' << std::get<1>(dumped) << ' ' << std::get<2>(dumped) << std::endl;

  return 0;
}
