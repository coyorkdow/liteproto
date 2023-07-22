# Proxy

C++ is a static language, which means the type of the object is determined when compiling, and we cannot create a new
type or designate the type to an object during the runtime. In a static language, we can still easily make a type which
can store the objects of any type. However, it's not easy to restore the original object from such storage.

Let's make an example with `std::any`.

```C++
    long v = 123;
    std::any any = v;
    if (auto ptr_int = std::any_cast<int>(&any); ptr_int) {
        std::cout << "any stores a int, the value is " << *ptr_int;
    } else if (auto ptr_float = std::any_cast<float>(&any); ptr_float) {
        std::cout << "any stores a int, the value is " << *ptr_float;
    } else if (auto ptr_long = std::any_cast<long>(&any); ptr_long) {
        std::cout << "any stores a long, the value is " << *ptr_long;
    } else if (auto ptr_double = std::any_cast<double>(&any); ptr_double) {
        std::cout << "any stores a double, the value is " << *ptr_double;
    } else if (auto ptr_ll = std::any_cast<long long>(&any); ptr_ll) {
        std::cout << "any stores a long long, the value is " << *ptr_ll;
    } else {
        std::cout << "we don't know what exactly the any stores";
    }
```

This snippet will print "any stores a long, the value is 123". The answer is correct, but if we change the type of `v` to
`unsigned`, it will print "we don't know what exactly the any stores". Since we cannot specify the object's type in runtime,
we have to enumerate all the possible type which the underlying object could be. In another word, we have to write verbose
codes to guess which type the underlying object actually is.

In the world of reflection, the type of the underlying object has been erased, just like the `std::any`. The meaning of the
reflection is to manipulate an object without knowing its type. We extract the common traits of the types, and encapsulate
these traits with the unified functions. Therefore, we get the **Interface**. An interface accepts any type of the object
which satisfies the requiring traits.

But we still have a problem. In C++, a type can be compounded. One kind of the compound type is generic type. For example,
we can use `std::vector<std::vector<int>>` to represent a two-dimension array. It satisfies the `List` interface, as well
as its value type. In such case, If we simply use the `List<std::vector<int>>`, then we have to guess the `std::vector<int>`,
which make our reflection system meaningless.

And how about `List<List<int>>`? Unfortunately, it doesn't solve the problem also. Consider a 3d structure, like
`std::vector<std::deque<std::list<int>>>`. It has three levels, and after we apply the interface on each level, we get
`List<List<List<int>>>`. In order to process this stuff, we have to guess `List<List<int>>` and write codes like
`ListCast<List<List<int>>, ConstOption::NON_CONST>`. It's still unacceptable.

What is the point? Such a complicated type is recurring created, and we should resolve it in a recurring way too.
We retrieve the interface from `Object`. If this interface also contains an interface, we should retrieve another `Object`
from the interface and resolve it, instead of guessing some more concrete type. Hence, we said the underlying type stored
in the interface is **proxied** by `Object`.

## `Object` and `Number`

With the type proxying, a `std::vector<std::deque<std::list<int>>>` is represented by `List<Object>`. This internal object
can continue to be resolved, in a recursive way. This process can be illustrated as follows.

```
Object => List<Object>
                 ||
                 ++==> List<Object>
                              ||
                              ++==> List<int>
```

It seems perfect, isn't it? But are we really able to guess the most underlying type is `int`? What if it's `short`,
`unsigned`, `long`, or `long long`? In fact, the fundamental types of C++ are still too many
(see https://en.cppreference.com/w/cpp/language/type). Hence, liteproto has another type to represent all arithmetic
types, the `Number`. Our resolving is actually like

```
Object => List<Object>
                 ||
                 ++==> List<Object>
                              ||
                              ++==> List<Number>
```

## Indirect Type and Number Type

From the view of the type proxy, we can categorize all the types into two parts: **indirect type** and **number type**.

All the arithmetic types (i.e., satisfy the `std::is_arithmetic`) are number types. A number type is proxied by `Number`.
And `Number` itself is also a number type.

All the types other than number types are indirect types. An indirect type is proxied by `Object`. And `Object` itself is
also an indirect type.

We should be aware of these two rules: A `Number` is a number type, and an `Object` is an indirect type. They make our
definition keep well-formed. We can create an STL container that stores the reflection type of liteproto, such as
`std::vector<Object>` or `std::vector<Number>`. Such types can also be reflected, and the underlying stored objects will
be proxied by the same types.

## Proxy Reference

The interfaces are all fat pointers, and they don't store the object. So an interface can be viewed as a reference to the
object which it currently manipulates. According to this rule, the `Number` is not an interface, because it doesn't ref
another object but stores the value by itself.

Interface supports iterating. In C++, an iterator has three attributes that in respect to the iterated value: value_type,
pointer, and reference. By default, the pointer is `value_type*` and the reference is `value_type&`. For iterator object
`it`, when we call `it->` it returns a pointer, and when we call `*it` it returns a reference. Some interface has subscript
operator, the `operator[]`, it also return a reference.

Assume we have a `List<Object>` object. All of its related member functions use `Object` as parameters, instead of the
actual underlying type. If we iterate this list, we iterate the proxy type `Object` too. Thus, the value_type shall be
`Object`. But we don't store an extra value in the iterator, instead we create a new proxy object every time when the
regarding member function of iterator is called. Since it is an object created on stack, we cannot return its reference
or pointer.

Since `Object` is a fat pointer, it can be already used as a reference. As for pointer, we don't offer the `operator->`
for the iterator of the proxy type, so than we can avoid any presence of it. The reference type of the proxy type is
always set to the `liteproto::internal::DummyPointer`.

### NumberReference

A `Number` doesn't refer any other object, it has its own value. According to our definition, `Number` is not an interface
as it's not a fat pointer. So unlike `Object`, `Number` can not be used as reference. So we propose another type,
`NumberReference`, to represent the reference of number types. Unlike `Number`, `NumberReference` is not number type, it
belongs to the indirect type. So it will be proxied by `Object`.
