#include <cassert>
#include <iostream>
#include <map>
#include <tuple>
#include <type_traits>

#include "liteproto/liteproto.hpp"

#if defined(__cpp_concepts)

template <class I>
concept Referenceable = requires(I& i) { i; };

template <class I>
concept LegacyIterator = requires(I i) {
  { *i } -> Referenceable;
  { ++i } -> std::same_as<I&>;
  { *i++ } -> Referenceable;
} && std::copyable<I>;

template <class I>
concept LegacyInputIterator = LegacyIterator<I> && std::equality_comparable<I> && requires(I i) {
  typename std::incrementable_traits<I>::difference_type;
  typename std::indirectly_readable_traits<I>::value_type;
  typename std::common_reference_t<std::iter_reference_t<I>&&,
                                   typename std::indirectly_readable_traits<I>::value_type&>;
  *i++;
  typename std::common_reference_t<decltype(*i++)&&, typename std::indirectly_readable_traits<I>::value_type&>;
  requires std::signed_integral<typename std::incrementable_traits<I>::difference_type>;
};

template <class It>
concept LegacyForwardIterator =
    LegacyInputIterator<It> && std::constructible_from<It> && std::is_reference_v<std::iter_reference_t<It>> &&
    std::same_as<std::remove_cvref_t<std::iter_reference_t<It>>,
                 typename std::indirectly_readable_traits<It>::value_type> &&
    requires(It it) {
      { it++ } -> std::convertible_to<const It&>;
      { *it++ } -> std::same_as<std::iter_reference_t<It>>;
    };

#endif

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
#if defined(__cpp_concepts)
  static_assert(LegacyForwardIterator<decltype(it)>);
#endif
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

void IterateObject(const liteproto::Object& obj) {
  using namespace liteproto;
  auto descriptor = obj.Descriptor();
  if (descriptor.KindEnum() == Kind::LIST) {
    descriptor = descriptor.ValueType();
    if (descriptor.TypeEnum() == Type::INT32) {
      assert(descriptor.KindEnum() == Kind::SCALAR);
      auto list = ListCast<int32_t>(obj);
      if (!list.has_value()) {
        return;
      }
      for (int i = 0; i < 10; i++) {
        list->push_back(i);
      }
    } else if (descriptor.IsIndirectType()) {
      if (descriptor.KindEnum() == Kind::LIST) {
        auto list = ListCast<Object>(obj);
        if (!list.has_value()) {
          return;
        }
        for (int i = 0; i < 5; i++) {
          auto [new_obj, data] = descriptor.DefaultValue();
          if (new_obj.empty()) {
            return;
          }
          list->push_back(new_obj);
          IterateObject((*list)[list->size() - 1]);
        }
      }
    }
  }
}

void test2() {
  using namespace liteproto;
  std::vector<int> one_dimension;
  std::vector<std::list<int>> two_dimension;
  liteproto::Object obj = GetReflection(&one_dimension);
  IterateObject(obj);
  static_assert(IsListV<decltype(two_dimension)>);
  obj = GetReflection(&two_dimension);
  IterateObject(obj);

  for (auto i : one_dimension) {
    std::cout << i << ' ';
  }
  std::cout << '\n';
  for (auto& each : two_dimension) {
    std::cout << '[';
    for (auto i : each) {
      std::cout << i << ' ';
    }
    std::cout << "] ";
  }
}

template <class T1, class T2, class T3>
TEMPLATE_MESSAGE(TestMessage, $(T1, T2, T3)) {
  T1 FIELD(test_field)->Seq<1>;
  T2 FIELD(test_field2)->Seq<2>;
  T3 FIELD(test_field3)->Seq<3>;

  constexpr TestMessage(T1 v1, T2 v2, T3 v3)
      : test_field_(std::move(v1)), test_field2_(std::move(v2)), test_field3_(std::move(v3)) {}
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
  test2();
  return 0;
}
