//
// Created by Youtao Guo on 2023/8/10.
//

#include "liteproto/liteproto.hpp"

MESSAGE(PrintableMsg) {
  int FIELD(foo) -> Seq<1>;
  float FIELD(bar) -> Seq<2>;
  std::string FIELD(baz) -> Seq<3>;

  PrintableMsg() : foo_(0), bar_(0) {}
};

int main() {
  PrintableMsg msg;
  msg.set_foo(1);
  msg.set_bar(2.33);
  msg.set_baz("some text");
  for (size_t i = 0; i < msg.FieldsSize(); i++) {
    std::cout << msg.FieldName(i) << ": ";
    msg.Visit(i, [&](auto&& value) { std::cout << value << '\n'; });
  }

  return 0;
}