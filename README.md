# liteproto
A lite serialization and reflection library of C++

Define a serializable class (called `Message`), and use reflection to inspect and modify its fields. Meanwhile you can use this class just like any other class.
The principle of the liteproto is to use Macro as little as possible, and make the syntax as close as possible to the C++'s native class definition.
Therefore, I came up with a syntax which is very similar to the protobuf. And it reaches a very strong flexibility.
You can define any constructor and member function for a `Message`. Moreover, a `Message` can be template or inherit other class.

---
**This Project is still work in progress. The first release is not ready yet. I am still working on my reflect system.**

---

## Quick Start

With the following codes, we defined a reflectable class which called `FirstMessage`. It has 5 reflectable fields. Besides, we can use it as any other normal C++ class, define the constructors, destructor, or any member function.

The `StaticReflect` use the static reflection to dump all fields into a tuple. We can get or update the original object's value through this tuple. Here, we changed the message's value.

Finally, we use dynamical reflection to print the structure and values of the message. Since the codes of the dyncamical reflection is complicate, we don't paste the implementation here. The full codes can be see in [example/first_message/first_message.cpp](example/first_message/first_message.cpp).

```C++
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

int main() {
  FirstMessage first_msg;
  first_msg.set_foo(1);
  first_msg.set_bar(1.5);
  first_msg.set_baz("str");
  first_msg.mutable_d2list().emplace_back(std::deque{1, 2, 3, 4});
  first_msg.mutable_strlist().emplace_back("abc");
  StaticReflect(first_msg);
  DynamicalReflect(first_msg);
  /*
DynamicalReflect will print
foo: 2
bar: -3.5
baz: "strstr"
d2list: [[1, 2, 3, 4], [5, 6, 7]]
strlist: ["abc", "abcdefg"]
  */
  return 0;
}

```
