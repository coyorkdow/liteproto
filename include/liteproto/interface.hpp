//
// Created by Youtao Guo on 2023/6/25.
//

#pragma once

#include <any>
#include <type_traits>

#include "liteproto/iterator.hpp"

namespace liteproto {

namespace internal {

template <class Tp, class = void>
struct ProxyType {
  using type = Tp;
};

template <class Tp>
struct ProxyType<Tp, std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<Tp>> &&
                                      !is_char_v<std::remove_reference_t<Tp>>>> {
  using type = Number;
};

template <class Tp>
struct ProxyType<Tp, std::enable_if_t<IsIndirectTypeV<Tp>>> {
  using type = Object;
};

template <class Tp>
struct ListInterface {
  using iterator = Iterator<Tp>;
  using reference = typename iterator::reference;
  static_assert(std::is_same_v<typename iterator::value_type, Tp>);

  using push_back_t = void(const std::any&, const Tp&);
  using emplace_back_t = void(const std::any&, Tp&&);
  using pop_back_t = void(const std::any&);
  using insert_t = iterator(const std::any&, iterator, const Tp&);
  using emplace_insert_t = iterator(const std::any&, iterator, Tp&&);
  using erase_t = iterator(const std::any&, iterator);
  using operator_subscript_t = reference(const std::any&, std::size_t);
  using size_t = std::size_t(const std::any&) noexcept;
  using resize_t = void(const std::any&, std::size_t);
  using resize_append_t = void(const std::any&, std::size_t, const Tp&);
  using empty_t = bool(const std::any&) noexcept;
  using clear_t = void(const std::any&);
  using begin_t = iterator(const std::any&) noexcept;
  using end_t = iterator(const std::any&) noexcept;

  using maybe_const = std::conditional_t<std::is_same_v<Tp, Object>, Tp, const Tp>;

  using const_interface_t = ListInterface<maybe_const>() noexcept;
  using to_const_t = std::any(const std::any&) noexcept;

  push_back_t* push_back;
  emplace_back_t* emplace_back;
  pop_back_t* pop_back;
  insert_t* insert;
  emplace_insert_t* emplace_insert;
  erase_t* erase;
  operator_subscript_t* operator_subscript;
  size_t* size;
  resize_t* resize;
  resize_append_t* resize_append;
  empty_t* empty;
  clear_t* clear;
  begin_t* begin;
  end_t* end;

  const_interface_t* const_interface;
  to_const_t* to_const;
};

template <class Tp>
struct StringInterface {
  using c_str_t = const Tp*(const std::any&) noexcept;
  using append_t = void(const std::any&, const Tp*, std::size_t);
  using data_t = Tp*(const std::any&) noexcept;

  using const_interface_t = StringInterface<const Tp>() noexcept;
  using to_const_t = std::any(const std::any&) noexcept;

  c_str_t* c_str;
  append_t* append;
  data_t* data;

  const_interface_t* const_interface;
  to_const_t* to_const;
};

template <class Car, class Cdr>
struct PairInterface {
  using car_t = Car&(const std::any&);
  using cdr_t = Cdr&(const std::any&);

  car_t* car;
  cdr_t* cdr;
};

template <class Pair, class Car, class Cdr>
struct PairInterfaceImpl {
  using base = PairInterface<Car, Cdr>;

  static Car& CAR(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Pair>(&obj);
    return (*ptr).first();
  }

  static Cdr& CDR(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Pair>(&obj);
    return (*ptr).second();
  }
};

template <class Adapter, class Tp>
struct ListInterfaceImpl {
  using base = ListInterface<Tp>;
  using iterator = typename base::iterator;
  using reference = typename base::reference;

  static void push_back(const std::any& obj, const Tp& v) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    (*ptr).push_back(v);
  }
  static_assert(std::is_same_v<decltype(push_back), typename base::push_back_t>);

  static void emplace_back(const std::any& obj, Tp&& v) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    (*ptr).push_back(std::move(v));
  }
  static_assert(std::is_same_v<decltype(emplace_back), typename base::emplace_back_t>);

  static void pop_back(const std::any& obj) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    (*ptr).pop_back();
  }
  static_assert(std::is_same_v<decltype(pop_back), typename base::pop_back_t>);

  static iterator insert(const std::any& obj, iterator pos, const Tp& v) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).insert(std::move(pos), v);
  }
  static_assert(std::is_same_v<decltype(insert), typename base::insert_t>);

  static iterator emplace_insert(const std::any& obj, iterator pos, Tp&& v) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).insert(std::move(pos), std::move(v));
  }
  static_assert(std::is_same_v<decltype(emplace_insert), typename base::emplace_insert_t>);

  static iterator erase(const std::any& obj, iterator pos) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).erase(std::move(pos));
  }
  static_assert(std::is_same_v<decltype(erase), typename base::erase_t>);

  static reference operator_subscript(const std::any& obj, size_t pos) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr)[pos];
  }
  static_assert(std::is_same_v<decltype(operator_subscript), typename base::operator_subscript_t>);

  static size_t size(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).size();
  }
  static_assert(std::is_same_v<decltype(size), typename base::size_t>);

  static void resize(const std::any& obj, size_t count) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    (*ptr).resize(count);
  }
  static_assert(std::is_same_v<decltype(resize), typename base::resize_t>);

  static void resize_append(const std::any& obj, size_t count, const Tp& v) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    (*ptr).resize(count, v);
  }
  static_assert(std::is_same_v<decltype(resize_append), typename base::resize_append_t>);

  static bool empty(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).empty();
  }
  static_assert(std::is_same_v<decltype(empty), typename base::empty_t>);

  static void clear(const std::any& obj) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    (*ptr).clear();
  }
  static_assert(std::is_same_v<decltype(clear), typename base::clear_t>);

  static iterator begin(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).begin();
  }
  static_assert(std::is_same_v<decltype(begin), typename base::begin_t>);

  static iterator end(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).end();
  }
  static_assert(std::is_same_v<decltype(end), typename base::end_t>);

  static ListInterface<typename base::maybe_const> ConstInterface() noexcept {
    using const_adapter = typename Adapter::maybe_const_adapter;
    using iterator_value_type = typename const_adapter::iterator_value_type;
    using impl = ListInterfaceImpl<const_adapter, iterator_value_type>;
    ListInterface<iterator_value_type> interface {};
    impl::MakeInterface(&interface);
    return interface;
  }
  static_assert(std::is_same_v<decltype(ConstInterface), typename base::const_interface_t>);

  static std::any ToConst(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).ToConst();
  }
  static_assert(std::is_same_v<decltype(ToConst), typename base::to_const_t>);

  static void MakeInterface(base* interface) noexcept {
    interface->push_back = &push_back;
    interface->emplace_back = &emplace_back;
    interface->pop_back = &pop_back;
    interface->insert = &insert;
    interface->emplace_insert = &emplace_insert;
    interface->erase = &erase;
    interface->operator_subscript = &operator_subscript;
    interface->size = &size;
    interface->resize = &resize;
    interface->resize_append = &resize_append;
    interface->empty = &empty;
    interface->clear = &clear;
    interface->begin = &begin;
    interface->end = &end;
    interface->const_interface = &ConstInterface;
    interface->to_const = &ToConst;
  }
};

template <class Adapter, class Tp>
struct StringInterfaceImpl {
  using base = StringInterface<Tp>;

  static const Tp* c_str(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).c_str();
  }
  static_assert(std::is_same_v<decltype(c_str), typename base::c_str_t>);

  static void append(const std::any& obj, const Tp* cstr, size_t n) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).append(cstr, n);
  }
  static_assert(std::is_same_v<decltype(append), typename base::append_t>);

  static Tp* data(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).data();
  }
  static_assert(std::is_same_v<decltype(data), typename base::data_t>);

  static StringInterface<const Tp> ConstInterface() noexcept {
    using const_adapter = typename Adapter::const_adapter;
    using iterator_value_type = typename const_adapter::iterator_value_type;
    static_assert(std::is_same_v<const Tp, iterator_value_type>);
    using impl = StringInterfaceImpl<const_adapter, iterator_value_type>;
    StringInterface<iterator_value_type> interface {};
    impl::MakeInterface(&interface);
    return interface;
  }
  static_assert(std::is_same_v<decltype(ConstInterface), typename base::const_interface_t>);

  static std::any ToConst(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).ToConst();
  }
  static_assert(std::is_same_v<decltype(ToConst), typename base::to_const_t>);

  static void MakeInterface(base* interface) noexcept {
    interface->c_str = &c_str;
    interface->data = &data;
    interface->append = &append;

    interface->const_interface = &ConstInterface;
    interface->to_const = &ToConst;
  }
};

template <class Adapter>
auto MakeListInterface() {
  using iterator_value_type = typename Adapter::iterator_value_type;
  using impl = ListInterfaceImpl<Adapter, iterator_value_type>;
  ListInterface<iterator_value_type> interface {};
  impl::MakeInterface(&interface);
  return interface;
}

template <class Adapter>
auto MakeStringInterface() {
  using iterator_value_type = typename Adapter::iterator_value_type;
  using impl = StringInterfaceImpl<Adapter, iterator_value_type>;
  using list_impl = ListInterfaceImpl<Adapter, iterator_value_type>;
  StringInterface<iterator_value_type> interface {};
  impl::MakeInterface(&interface);

  ListInterface<iterator_value_type> list_interface{};
  list_impl::MakeInterface(&list_interface);
  return std::make_pair(list_interface, interface);
}

}  // namespace internal

}  // namespace liteproto