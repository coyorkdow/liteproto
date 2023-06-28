//
// Created by Youtao Guo on 2023/6/25.
//

#pragma once

#include <any>
#include <type_traits>

namespace liteproto {

template <class Tp>
class Iterator;

namespace internal {

template <class Tp>
struct ListInterface {
  using iterator = Iterator<Tp>;

  using push_back_t = void(const std::any&, Tp);
  using pop_back_t = void(const std::any&);
  using insert_t = iterator(const std::any&, iterator, Tp);
  using erase_t = iterator(const std::any&, iterator);
  using operator_subscript_t = Tp&(const std::any&, std::size_t);
  using size_t = std::size_t(const std::any&) noexcept;
  using resize_t = void(const std::any&, std::size_t);
  using resize_append_t = void(const std::any&, std::size_t, const Tp&);
  using empty_t = bool(const std::any&) noexcept;
  using clear_t = void(const std::any&);
  using begin_t = iterator(const std::any&) noexcept;
  using end_t = iterator(const std::any&) noexcept;

  using const_interface_t = ListInterface<const Tp>() noexcept;
  using to_const_t = std::any(const std::any&) noexcept;

  push_back_t* push_back;
  pop_back_t* pop_back;
  insert_t* insert;
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

struct StringInterface : ListInterface<char> {
  using c_str_t = const char*(const std::any&);
  using append_t = void(const std::any&, const char*, std::size_t);

  c_str_t* c_str;
  append_t* append;
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
  using iterator = typename ListInterface<Tp>::iterator;

  static void push_back(const std::any& obj, Tp v) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    (*ptr).push_back(std::move(v));
  }
  static_assert(std::is_same_v<decltype(push_back), typename base::push_back_t>);

  static void pop_back(const std::any& obj) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    (*ptr).pop_back();
  }
  static_assert(std::is_same_v<decltype(pop_back), typename base::pop_back_t>);

  static iterator insert(const std::any& obj, iterator pos, Tp v) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).insert(std::move(pos), std::move(v));
  }
  static_assert(std::is_same_v<decltype(insert), typename base::insert_t>);

  static iterator erase(const std::any& obj, iterator pos) {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).erase(std::move(pos));
  }
  static_assert(std::is_same_v<decltype(erase), typename base::erase_t>);

  static Tp& operator_subscript(const std::any& obj, size_t pos) {
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

  static ListInterface<const Tp> ConstInterface() noexcept;
  static_assert(std::is_same_v<decltype(ConstInterface), typename base::const_interface_t>);

  static std::any ToConst(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Adapter>(&obj);
    return (*ptr).ToConst();
  }
  static_assert(std::is_same_v<decltype(ToConst), typename base::to_const_t>);
};

template <class Str>
struct StringInterfaceImpl : ListInterfaceImpl<Str, char> {
  using base = StringInterface;

  static const char* c_str(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<Str>(&obj);
    return (*ptr).c_str();
  }
  static_assert(std::is_same_v<decltype(c_str), typename base::c_str_t>);

  static void append(const std::any& obj, const char* cstr, size_t n) {
    auto* ptr = std::any_cast<Str>(&obj);
    return (*ptr).append(cstr, n);
  }
  static_assert(std::is_same_v<decltype(append), typename base::append_t>);
};

template <class Adapter>
auto MakeListInterface() {
  using iterator_value_type = typename Adapter::iterator_value_type;
  using impl = ListInterfaceImpl<Adapter, iterator_value_type>;
  ListInterface<iterator_value_type> interface {};
  interface.push_back = &impl::push_back;
  interface.pop_back = &impl::pop_back;
  interface.insert = &impl::insert;
  interface.erase = &impl::erase;
  interface.operator_subscript = &impl::operator_subscript;
  interface.size = &impl::size;
  interface.resize = &impl ::resize;
  interface.resize_append = &impl ::resize_append;
  interface.empty = &impl::empty;
  interface.clear = &impl::clear;
  interface.begin = &impl::begin;
  interface.end = &impl::end;

  interface.const_interface = &impl::ConstInterface;
  interface.to_const = &impl::ToConst;
  return interface;
}

template <class Adapter, class Tp>
ListInterface<const Tp> ListInterfaceImpl<Adapter, Tp>::ConstInterface() noexcept {
  using const_adapter = typename Adapter::const_adapter;
  return MakeListInterface<const_adapter>();
}

template <class Adapter>
auto MakeStringInterface() {
  using impl = StringInterfaceImpl<Adapter>;
  StringInterface interface {};
  interface.push_back = &impl::push_back;
  interface.pop_back = &impl::pop_back;
  interface.insert = &impl::insert;
  interface.erase = &impl::erase;
  interface.operator_subscript = &impl::operator_subscript;
  interface.size = &impl::size;
  interface.resize = &impl ::resize;
  interface.resize_append = &impl ::resize_append;
  interface.empty = &impl::empty;
  interface.clear = &impl::clear;
  interface.begin = &impl::begin;
  interface.end = &impl::end;

  interface.c_str = &impl::c_str;
  interface.append = &impl::append;
  return interface;
}

}  // namespace internal

}  // namespace liteproto