//
// Created by Youtao Guo on 2023/6/30.
//

#include <cassert>
#include <iostream>
#include <map>
#include <numeric>
#include <tuple>
#include <type_traits>

#include "gtest/gtest.h"
#include "liteproto/liteproto.hpp"
#include "liteproto/static_test/static_test.hpp"
#include "nameof.hpp"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

TEST(TestNumber, Basic) {
  using namespace liteproto;
  Number n;
  n = static_cast<Number>(1);
  EXPECT_EQ(n.Descriptor().TypeEnum(), Type::INT32);
  EXPECT_TRUE(n.IsSignedInteger());
  EXPECT_EQ(n.AsInt64(), 1);
  EXPECT_EQ(static_cast<int64_t>(n), 1);
  n = 3.14159;
  EXPECT_TRUE((n.Descriptor().TypeEnum() == Type::FLOAT32) || (n.Descriptor().TypeEnum() == Type::FLOAT64));
  EXPECT_EQ(n.Descriptor().KindEnum(), Kind::NUMBER);
  EXPECT_TRUE(n.IsFloating());
  EXPECT_DOUBLE_EQ(n.AsFloat64(), 3.14159);
  EXPECT_DOUBLE_EQ(static_cast<double>(n), 3.14159);
  n = std::numeric_limits<int32_t>::min();
  EXPECT_EQ(n.Descriptor().TypeEnum(), Type::INT32);
  EXPECT_EQ(n.AsInt64(), std::numeric_limits<int32_t>::min());
  EXPECT_EQ(static_cast<int64_t>(n), std::numeric_limits<int32_t>::min());
  n = std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(n.Descriptor().TypeEnum(), Type::UINT32);
  EXPECT_TRUE(n.IsUnsigned());
  EXPECT_EQ(n.AsUInt64(), std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(static_cast<uint64_t>(n), std::numeric_limits<uint32_t>::max());

  n = static_cast<char>('v');
  EXPECT_EQ(n.Descriptor().KindEnum(), Kind::CHAR);
  EXPECT_EQ(n.AsInt64(), 'v');

  Number ano = n;
  EXPECT_EQ(ano.Descriptor().KindEnum(), Kind::CHAR);
  EXPECT_EQ(ano.AsInt64(), 'v');
}

TEST(TestNumber, Reference) {
  using namespace liteproto;
  Number n;
  n = static_cast<Number>(1);
  NumberReference ref = n;
  static_assert(std::is_same_v<decltype(ref), NumberReference<ConstOption::NON_CONST>>);
  ref.SetFloat64(3.14159);
  EXPECT_DOUBLE_EQ(static_cast<double>(n), 3.14159);
  int i = -1;
  ref = i;  // change binding
  EXPECT_EQ(ref.AsInt64(), -1);
  EXPECT_TRUE(ref.IsSignedInteger());
  EXPECT_EQ(ref.Descriptor().TypeEnum(), Type::INT32);
  const Number& cref_of_n = n;
  NumberReference cref = cref_of_n;
  EXPECT_DOUBLE_EQ(static_cast<double>(cref), 3.14159);
  float f = 0.1;
  cref = f;  // change binding
  EXPECT_FLOAT_EQ(cref.AsFloat64(), 0.1);
  EXPECT_DOUBLE_EQ(cref_of_n.AsFloat64(), 3.14159);
}

TEST(TestNumber, NumberIsNumber) {
  using namespace liteproto;
  std::vector<Number> nums;
  for (int i = 0; i < 3; i++) {
    nums.emplace_back(i);
  }
  auto list = AsList(&nums);
  static_assert(std::is_same_v<decltype(list), List<Number, ConstOption::NON_CONST>>);
  for (auto it = list.begin(); it != list.end(); it++) {
    (*it).SetInt64(123);
  }
  for (auto i : nums) {
    EXPECT_EQ(static_cast<int64_t>(i), 123);
  }
  std::vector<NumberReference<ConstOption::CONST>> refs;
  for (auto& i : nums) {
    refs.emplace_back(i);
  }
  for (auto i : list) {
    refs.emplace_back(i);
  }
  for (auto& i : refs) {
    EXPECT_EQ(i.AsInt64(), 123);
  }
  for (int i = 0; i < 3; i++) {
    nums[i] = i + 3;
    EXPECT_EQ(list[i].AsInt64(), i + 3);
    EXPECT_EQ(refs[i].AsInt64(), i + 3);
    EXPECT_EQ(refs[i + 3].AsInt64(), i + 3);
  }

  list.clear();
  list.push_back(1);
  list.push_back(3);
  auto it = list.begin();
  list.insert(++it, 2);
  for (int i = 0; i < 3; i++) {
    EXPECT_EQ(list[i].AsInt64(), i + 1);
  }
  list.resize(6, 10.57);
  for (int i = 3; i < 6; i++) {
    EXPECT_DOUBLE_EQ(list[i].AsFloat64(), 10.57);
  }
}

TEST(TestNumber, ReferenceIsObject) {
  using namespace liteproto;
  std::vector<int> nums(1, 5);
  auto list = AsList(&nums);
  std::vector<NumberReference<ConstOption::NON_CONST>> refs;
  for (auto i : list) {
    refs.emplace_back(i);
  }
  auto ref_list = AsList(&refs);
  static_assert(std::is_same_v<decltype(ref_list), List<Object, ConstOption::NON_CONST>>);
  for (auto obj : ref_list) {
    EXPECT_EQ(obj.Descriptor().KindEnum(), Kind::NUMBER_REFERENCE);
    EXPECT_EQ(obj.Descriptor().TypeEnum(), Type::NUMBER_REFERENCE_NON_CONST);
    auto ref = ObjectCast<NumberReference<ConstOption::NON_CONST>>(obj);
    ASSERT_TRUE(ref != nullptr);
    ref->SetInt64(65536);
  }
  for (auto i : nums) {
    EXPECT_EQ(i, 65536);
  }
}

TEST(TestList, Basic) {
  using namespace liteproto;
  std::deque<int> de;
  auto list = AsList(&de);
  list.resize(10);
  for (auto it = list.begin(); it != list.end(); it++) {
    (*it).SetInt64(123);
  }
  EXPECT_EQ(de.size(), 10);
  for (auto i : de) {
    EXPECT_EQ(i, 123);
  }
}

TEST(TestList, ListOfString) {
  using namespace liteproto;
  std::vector<std::string> strlist;
  auto list = AsList(&strlist);
  for (int i = 0; i < 10; i++) {
    std::string si = std::to_string(i);
    list.push_back(GetReflection(&si));
    EXPECT_EQ(strlist.back(), std::to_string(i));
  }
  EXPECT_EQ(strlist.size(), 10);
  EXPECT_EQ(list.size(), 10);
  for (int i = 0; i < 5; i++) {
    list.pop_back();
    EXPECT_EQ(list.size(), 10 - i - 1);
  }
  int cnt = 0;
  for (auto v : list) {
    auto str = StringCast(v);
    str->append("x");
    str = StringCast(list[cnt]);
    EXPECT_EQ((*str)[str->size() - 1], 'x');
    std::string* p = ObjectCast<std::string>(list[cnt]);
    EXPECT_EQ(p->back(), 'x');
    cnt++;
  }
  EXPECT_EQ(list.size(), 5);
  const auto& cref = strlist;
  auto const_list = AsList(&cref);
  for (size_t i = 0; i < const_list.size(); i++) {
    auto str_interface = StringCast(const_list[i]);
    EXPECT_FALSE(str_interface.has_value());
    auto cstr_interface = StringCast<ConstOption::CONST>(const_list[i]);
    auto endit = cstr_interface.value().end();
    EXPECT_EQ(*(--endit), 'x');
    auto str = ObjectCast<std::string>(const_list[i]);
    EXPECT_EQ(str, nullptr);
    auto cstr = ObjectCast<const std::string>(const_list[i]);
    EXPECT_EQ(cstr->back(), 'x');
  }
  cnt = 0;
  for (auto it = const_list.begin(); it != const_list.end(); it++, cnt++) {
    auto descriptor = (*it).Descriptor();
    EXPECT_EQ(descriptor.KindEnum(), Kind::STRING);
    EXPECT_EQ(descriptor.TypeEnum(), Type::STD_STRING);
    EXPECT_TRUE(descriptor.Traits(traits::is_const));
    EXPECT_EQ(descriptor.Transform(transform::remove_const).TypeEnum(), Type::STD_STRING);
    auto str = ObjectCast<const std::string>(*it);
    EXPECT_EQ(*str, strlist[cnt]);
  }

  auto it = list.begin();
  static_assert(std::is_same_v<decltype(it), ObjectIterator<>>);
  std::list<std::string> anol;
  list = AsList(&anol);
  EXPECT_TRUE(it != list.begin());
  EXPECT_TRUE(list.begin() == list.end());
  EXPECT_EQ(list.size(), 0);
  EXPECT_TRUE(list.empty());
  for (int i = 0; i < 5; i++) {
    auto str = "no" + std::to_string(i);
    list.insert(list.end(), GetReflection(&str));
    EXPECT_EQ(list.size(), i + 1);
  }
  EXPECT_EQ(anol.size(), 5);
  cnt = 0;
  for (auto& v : anol) {
    EXPECT_EQ(v, "no" + std::to_string(cnt));
    cnt++;
  }
  std::string _{"abc"};
  list.resize(10, GetReflection(&_));
  EXPECT_EQ(anol.size(), 10);
  auto iter = std::next(anol.begin(), 5);
  for (int i = 5; i < 10; i++) {
    EXPECT_EQ(*iter, "abc");
    iter++;
  }
  it = list.begin();
  std::advance(it, 5);
  for (int i = 5; i < 10; i++) {
    auto p = StringCast(*it);
    EXPECT_STRCASEEQ(p.value().c_str(), "abc");
    it++;
  }
  while (!list.empty()) {
    it = list.erase(list.begin());
  }
  EXPECT_EQ(anol.size(), 0);
  _ = "new str";
  list.insert(list.begin(), GetReflection(&_));
  EXPECT_TRUE(StringCast<ConstOption::NON_CONST>(*list.begin()).has_value());
  const_list = list;  // is const
  auto csit = const_list.begin();
  static_assert(std::is_same_v<decltype(csit), decltype(const_list.end())>);
  EXPECT_FALSE(StringCast<ConstOption::NON_CONST>(*csit).has_value());
  EXPECT_TRUE(StringCast<ConstOption::CONST>(*csit).has_value());
  EXPECT_STREQ(StringCast<ConstOption::CONST>(*const_list.begin())->c_str(), "new str");
}

TEST(TestString, basic) {
  using namespace liteproto;
  std::string stdstr;
  auto str = AsString(&stdstr);
  static_assert(std::is_same_v<decltype(str), String<liteproto::ConstOption::NON_CONST>>);
  ASSERT_TRUE(str.empty());
  ASSERT_EQ(str.size(), 0);
  ASSERT_TRUE(str.begin() == str.end());
  str.append("123").append("456").append("789");
  EXPECT_EQ(str.size(), 9);

  EXPECT_EQ(stdstr, "123456789");

  EXPECT_STRCASEEQ(str.c_str(), "123456789");
  EXPECT_STRCASEEQ(str.data(), "123456789");

  const auto& cref = stdstr;
  String<liteproto::ConstOption::CONST> cstr = AsString(&cref);
  EXPECT_STRCASEEQ(cstr.c_str(), "123456789");
  EXPECT_STRCASEEQ(cstr.data(), "123456789");
  size_t cnt = 0;
  for (char c : cstr) {
    EXPECT_EQ(c, cnt++ + '1');
  }
  cstr = str;
}

void IterateObject(const liteproto::Object& obj) {
  using namespace liteproto;
  auto descriptor = obj.Descriptor();
  if (descriptor.KindEnum() == Kind::LIST) {
    descriptor = descriptor.ValueType();
    if (auto number_list = ListCast<Number, ConstOption::NON_CONST>(obj); number_list.has_value()) {
      EXPECT_EQ(descriptor.KindEnum(), Kind::NUMBER);
      for (int i = 0; i < 10; i++) {
        number_list->push_back(i);
      }
    } else if (auto object_list = ListCast<Object, ConstOption::NON_CONST>(obj); object_list.has_value()) {
      for (int i = 0; i < 5; i++) {
        auto [new_obj, data] = descriptor.DefaultValue();
        ASSERT_FALSE(new_obj.empty());
        object_list->push_back(new_obj);
        IterateObject((*object_list)[object_list->size() - 1]);
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

  int16_t v = -45;
  auto o = GetReflection(&v);
  EXPECT_EQ(o.Descriptor().KindEnum(), Kind::NUMBER);
  auto vref = NumberCast(o);
  ASSERT_TRUE(vref.has_value());
  EXPECT_EQ(vref->AsInt64(), -45);
}

template <class T1, class T2, class T3>
TEMPLATE_MESSAGE(TestMessage, $(T1, T2, T3)) {
  T1 FIELD(foo)->Seq<1>;
  T2 FIELD(bar)->Seq<2>;
  T3 FIELD(baz)->Seq<3>;

  TestMessage(T1 v1, T2 v2, T3 v3) noexcept : foo_(std::move(v1)), bar_(std::move(v2)), baz_(std::move(v3)) {}
};

TEST(TestMsgFundamenal, basic) {
  TestMessage<int, float, std::string> msg{1, 2.5, "str"};
  EXPECT_EQ(msg.foo(), 1);
  EXPECT_FLOAT_EQ(msg.bar(), 2.5);
  EXPECT_EQ(msg.baz(), "str");
  auto tuple_ref = msg.DumpForwardTuple();
  std::get<0>(tuple_ref)++;
  std::get<1>(tuple_ref)--;
  std::get<2>(tuple_ref).append("str");
  EXPECT_EQ(msg.foo(), 2);
  EXPECT_FLOAT_EQ(msg.bar(), 1.5);
  EXPECT_EQ(msg.baz(), "strstr");

  auto& names = decltype(msg)::FieldsName();
  for (auto& [i, name] : names) {
    EXPECT_TRUE(i == 1 || i == 2 || i == 3);
    if (i == 1) {
      EXPECT_EQ(name, "foo");
    } else if (i == 2) {
      EXPECT_EQ(name, "bar");
    } else if (i == 3) {
      EXPECT_EQ(name, "baz");
    }
  }
}

TEST(Test3rd, RapidJson) {
  using namespace rapidjson;
  Document document;
  auto& value = document.SetObject();
  value.AddMember("foo", 1, document.GetAllocator());
  value.AddMember("bar", 2, document.GetAllocator());
  EXPECT_TRUE(document.HasMember("foo"));
  EXPECT_TRUE(document.HasMember("bar"));
  StringBuffer buffer;
  PrettyWriter<StringBuffer> writer(buffer);
  document.Accept(writer);
  std::cout << buffer.GetString();
}