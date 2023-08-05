//
// Created by Youtao Guo on 2023/8/5.
//

#include <cassert>
#include <iostream>

#include "liteproto/liteproto.hpp"

template <class Number>
void PrintNumber(Number&& number) {
  if (number.IsSignedInteger()) {
    std::cout << number.AsInt64();
  } else if (number.IsUnsigned()) {
    std::cout << number.AsUInt64();
  } else {
    std::cout << number.AsFloat64();
  }
}

void PrintDynamicalList(liteproto::Object obj) {
  auto descriptor = obj.Descriptor();
  assert(descriptor.KindEnum() == liteproto::Kind::LIST);
  std::cout << '[';
  descriptor = descriptor.ValueType();
  if (descriptor.KindEnum() == liteproto::Kind::NUMBER) {
    auto number_list = liteproto::ListCast<liteproto::Number>(obj);
    for (size_t i = 0; i < number_list->size(); i++) {
      if (i) {
        std::cout << ", ";
      }
      PrintNumber((*number_list)[i]);
    }
  } else {
    auto obj_list = liteproto::ListCast<liteproto::Object>(obj);
    for (size_t i = 0; i < obj_list->size(); i++) {
      if (i) {
        std::cout << ", ";
      }
      if (descriptor.KindEnum() == liteproto::Kind::STRING) {
        std::cout << '\"' << liteproto::StringCast((*obj_list)[i])->str() << '\"';
      } else if (descriptor.KindEnum() == liteproto::Kind::LIST) {
        PrintDynamicalList((*obj_list)[i]);
      }
    }
  }
  std::cout << ']';
}

void DynamicalReflect(liteproto::Message& msg) {
  for (size_t i = 0; i < msg.FieldsSize(); i++) {
    std::cout << msg.FieldName(i) << ": ";
    liteproto::Object object = msg.Field(i);
    if (object.Descriptor().KindEnum() == liteproto::Kind::NUMBER) {
      auto number = liteproto::NumberCast(object);  // number is a std::optional
      PrintNumber(number.value());
    } else if (object.Descriptor().KindEnum() == liteproto::Kind::STRING) {
      auto str = liteproto::StringCast(object);
      std::cout << '\"' << str->str() << '\"';
    } else if (object.Descriptor().KindEnum() == liteproto::Kind::LIST) {
      PrintDynamicalList(object);
    }
    std::cout << '\n';
  }
}

MESSAGE(FirstMessage) {
  int FIELD(foo) -> Seq<1>;
  double FIELD(bar) -> Seq<2>;
  std::string FIELD(baz) -> Seq<3>;
  std::vector<std::deque<int>> FIELD(d2list) -> Seq<4>;
  std::list<std::string> FIELD(strlist) -> Seq<5>;

  FirstMessage() : foo_(0), bar_(0) {}
};

void StaticReflect(FirstMessage& msg) {
  auto tuple = msg.DumpTuple();
  std::get<0>(tuple)++;
  std::get<1>(tuple) -= 5;
  std::get<2>(tuple).append("str");
  std::get<3>(tuple).emplace_back(std::deque{5, 6, 7});
  std::get<4>(tuple).emplace_back("abcdefg");
}

int main() {
  FirstMessage first_msg;
  first_msg.set_foo(1);
  first_msg.set_bar(1.5);
  first_msg.set_baz("str");
  first_msg.mutable_d2list().emplace_back(std::deque{1, 2, 3, 4});
  first_msg.mutable_strlist().emplace_back("abc");
  StaticReflect(first_msg);
  DynamicalReflect(first_msg);

  return 0;
}