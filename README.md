# liteproto
A lite serialization and reflection library of C++

Define a serializable class (called `Message`), and use reflection to inspect and modify its fields. Meanwhile you can use this class just like any other class.
The principle of the liteproto is to use Macro as little as possible, and make the syntax as close as possible to the C++'s native class definition.
Therefore, I came up with a syntax which is very similar to the protobuf. And it reaches a very strong flexibility.
You can define any constructor and member function for a `Message`. Moreover, a `Message` can be template or inherit other class.

---
**This Project is still work in progress. The first release is not ready yet. I am still working on my reflect system.**

---

```C++
template <class T1, class T2, class T3>
TEMPLATE_MESSAGE(TestMessage, $(T1, T2, T3)) {
  T1 FIELD(test_field) -> Seq<1>;
  T2 FIELD(test_field2) -> Seq<2>;
  T3 FIELD(test_field3) -> Seq<3>;

  constexpr TestMessage(T1 v1, T2 v2, T3 v3)
      : test_field_(std::move(v1)), test_field2_(std::move(v2)), test_field3_(std::move(v3)) {}
};

void test() {
  TestMessage<int, float, std::string> msg{1, 2.5, "str"};
  auto tuple_ref = msg.DumpTuple();
  std::get<0>(tuple_ref)++;
  std::get<1>(tuple_ref)--;
  std::get<2>(tuple_ref).append("str");
  std::cout << msg.test_field(); // print 2
  std::cout << msg.test_field2(); // print 1.5
  std::cout << msg.test_field3(); // print strstr
}
```
