#include <cassert>
#include <iostream>
#include <map>
#include <tuple>
#include <type_traits>

#include "liteproto/liteproto.hpp"

void test() {
  using namespace liteproto;
  std::vector<std::string> strlist;
  auto list = AsList(&strlist);
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
  auto const_list = AsList(&cref);
  for (int i = 0; i < const_list.size(); i++) {
    assert(const_list[i].back() == 'x');
  }
  for (auto it = const_list.begin(); it != const_list.end(); it++) {
    std::cout << *it << ' ';
  }
  std::cout << std::endl;

  Iterator<std::string> it = list.begin();

  std::list<std::string> anol;
  list = AsList(&anol);
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
}

template <class T1, class T2, class T3>
TEMPLATE_MESSAGE(TestMessage, $(T1, T2, T3)) {
  T1 FIELD(test_field)  -> Seq<1>;
  T2 FIELD(test_field2) -> Seq<2>;
  T3 FIELD(test_field3) -> Seq<3>;

  constexpr TestMessage(T1 v1, T2 v2, T3 v3) : test_field_(std::move(v1)), test_field2_(std::move(v2)), test_field3_(std::move(v3)) {}
};

int main() {
  using namespace liteproto;

  TestMessage<int, float, std::string> msg{1, 2.5, "str"};
  std::cout << msg.test_field() << ' ' << msg.test_field2() << ' ' << msg.test_field3() << '\n';
  auto tuple_ref = msg.DumpForwardTuple();
  std::get<0>(tuple_ref)++;
  std::get<1>(tuple_ref)--;
  std::get<2>(tuple_ref).append("str");
  std::cout << msg.test_field() << ' ' << msg.test_field2() << ' ' << msg.test_field3() << '\n';

  test();
  return 0;
}
