//
// Created by Youtao Guo on 2023/6/30.
//

#include <cassert>
#include <iostream>
#include <map>
#include <tuple>
#include <type_traits>

#include "gtest/gtest.h"
#include "liteproto/liteproto.hpp"
#include "liteproto/static_test/static_test.hpp"

TEST(TestList, basic) {
  using namespace liteproto;
  std::vector<std::string> strlist;
  auto list = AsList(&strlist);
  for (int i = 0; i < 10; i++) {
    list.push_back(std::to_string(i));
    EXPECT_EQ(strlist.back(), std::to_string(i));
  }
  EXPECT_EQ(strlist.size(), 10);
  EXPECT_EQ(list.size(), 10);
  for (int i = 0; i < 5; i++) {
    list.pop_back();
    EXPECT_EQ(list.size(), 10 - i - 1);
  }
  int cnt = 0;
  for (auto& v : list) {
    v.append("x");
    EXPECT_EQ(list[cnt].back(), 'x');
    cnt++;
  }
  EXPECT_EQ(list.size(), 5);
  const auto& cref = strlist;
  auto const_list = AsList(&cref);
  for (size_t i = 0; i < const_list.size(); i++) {
    EXPECT_EQ(const_list[i].back(), 'x');
  }
  cnt = 0;
  for (auto it = const_list.begin(); it != const_list.end(); it++, cnt++) {
    EXPECT_EQ(*it, strlist[cnt]);
  }

  Iterator<std::string> it = list.begin();
  std::list<std::string> anol;
  list = AsList(&anol);
  EXPECT_TRUE(it != list.begin());
  EXPECT_TRUE(list.begin() == list.end());
  EXPECT_EQ(list.size(), 0);
  EXPECT_TRUE(list.empty());
  for (int i = 0; i < 5; i++) {
    list.insert(list.end(), "no" + std::to_string(i));
  }
  cnt = 0;
  for (auto& v : anol) {
    EXPECT_EQ(v, "no" + std::to_string(cnt));
    cnt++;
  }
  list.resize(10, "abc");
  EXPECT_EQ(anol.size(), 10);
  auto iter = std::next(anol.begin(), 5);
  for (int i = 5; i < 10; i++) {
    EXPECT_EQ(*iter, "abc");
    iter++;
  }
  while (!list.empty()) {
    it = list.erase(list.begin());
  }
  EXPECT_EQ(anol.size(), 0);
  list.insert(list.begin(), "new str");
  const_list = list;
  EXPECT_STREQ(const_list.begin()->c_str(), "new str");
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

TEST(TestReflection, Basic) {
  using namespace liteproto;
  std::vector<int> one_dimension;
  std::vector<std::list<int>> two_dimension;
  liteproto::Object obj = GetReflection(&one_dimension);
  IterateObject(obj);
  static_assert(IsListV<decltype(two_dimension)>);
  obj = GetReflection(&two_dimension);
  IterateObject(obj);

  EXPECT_EQ(one_dimension.size(), 10);
  for (size_t i = 0; i < one_dimension.size(); i++) {
    EXPECT_EQ(one_dimension[i], i);
  }

  EXPECT_EQ(two_dimension.size(), 5);
  for (auto& each : two_dimension) {
    EXPECT_EQ(each.size(), 10);
    auto iter = each.begin();
    for (size_t i = 0; i < each.size(); i++, iter++) {
      EXPECT_EQ(*iter, i);
    }
  }
}

template <class T1, class T2, class T3>
TEMPLATE_MESSAGE(TestMessage, $(T1, T2, T3)) {
  T1 FIELD(test_field)->Seq<1>;
  T2 FIELD(test_field2)->Seq<2>;
  T3 FIELD(test_field3)->Seq<3>;

  TestMessage(T1 v1, T2 v2, T3 v3) noexcept
      : test_field_(std::move(v1)), test_field2_(std::move(v2)), test_field3_(std::move(v3)) {}
};

TEST(TestMsgFundamenal, basic) {
  TestMessage<int, float, std::string> msg{1, 2.5, "str"};
  EXPECT_EQ(msg.test_field(), 1);
  EXPECT_FLOAT_EQ(msg.test_field2(), 2.5);
  EXPECT_EQ(msg.test_field3(), "str");
  auto tuple_ref = msg.DumpForwardTuple();
  std::get<0>(tuple_ref)++;
  std::get<1>(tuple_ref)--;
  std::get<2>(tuple_ref).append("str");
  EXPECT_EQ(msg.test_field(), 2);
  EXPECT_FLOAT_EQ(msg.test_field2(), 1.5);
  EXPECT_EQ(msg.test_field3(), "strstr");
}