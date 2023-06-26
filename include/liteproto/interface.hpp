//
// Created by Youtao Guo on 2023/6/25.
//

#pragma once

#include <any>

#include "liteproto/traits.hpp"

namespace liteproto {

template <class Tp>
class ListIterator;

template <class Tp>
struct ListInterface {
  using iterator = ListIterator<Tp>;

  using push_back_t = void(std::any&, Tp);
  using pop_back_t = void(std::any&);
  using insert_t = void(std::any&, std::size_t, Tp);
  using erase_t = void(std::any&, std::size_t);
  using operator_subscript_t = Tp&(std::any&, std::size_t);
  using size_t = std::size_t(const std::any&);
  using empty_t = bool(const std::any&);
  using clear_t = void(std::any&);
  using begin_t = iterator(std::any&);
  using end_t = iterator(std::any&);

  push_back_t* push_back;
  pop_back_t* pop_back;
  insert_t* insert;
  erase_t* erase;
  operator_subscript_t* operator_subscript;
  size_t* size;
  empty_t* empty;
  clear_t* clear;
  begin_t* begin;
  end_t* end;
};

namespace internal {

template <class Tp, class List>
struct ListInterfaceImpl : ListInterface<Tp> {
  using base = ListInterface<Tp>;
  using iterator = typename ListInterface<Tp>::iterator;

  static void push_back(std::any& obj, Tp v) {
    auto* ptr = std::any_cast<List>(&obj);
    (*ptr).push_back(std::move(v));
  }
  static_assert(std::is_same_v<decltype(push_back), typename base::push_back_t>);

  static void pop_back(std::any& obj) {
    auto* ptr = std::any_cast<List>(&obj);
    (*ptr).pop_back();
  }
  static_assert(std::is_same_v<decltype(pop_back), typename base::pop_back_t>);

  static void insert(std::any& obj, size_t pos, Tp v) {
    auto* ptr = std::any_cast<List>(&obj);
    (*ptr).insert(pos, std::move(v));
  }
  static_assert(std::is_same_v<decltype(insert), typename base::insert_t>);

  static void erase(std::any& obj, size_t pos) {
    auto* ptr = std::any_cast<List>(&obj);
    (*ptr).erase(pos);
  }
  static_assert(std::is_same_v<decltype(erase), typename base::erase_t>);

  static Tp& operator_subscript(std::any& obj, size_t pos) {
    auto* ptr = std::any_cast<List>(&obj);
    return (*ptr)[pos];
  }
  static_assert(std::is_same_v<decltype(operator_subscript), typename base::operator_subscript_t>);

  static size_t size(const std::any& obj) {
    auto* ptr = std::any_cast<List>(&obj);
    return (*ptr).size();
  }
  static_assert(std::is_same_v<decltype(size), typename base::size_t>);

  static bool empty(const std::any& obj) {
    auto* ptr = std::any_cast<List>(&obj);
    return (*ptr).empty();
  }
  static_assert(std::is_same_v<decltype(empty), typename base::empty_t>);

  static void clear(std::any& obj) {
    auto* ptr = std::any_cast<List>(&obj);
    (*ptr).clear();
  }
  static_assert(std::is_same_v<decltype(clear), typename base::clear_t>);

  static iterator begin(std::any& obj) {
    auto* ptr = std::any_cast<List>(&obj);
    return (*ptr).begin();
  }
  static_assert(std::is_same_v<decltype(begin), typename base::begin_t>);

  static iterator end(std::any& obj) {
    auto* ptr = std::any_cast<List>(&obj);
    return (*ptr).end();
  }
  static_assert(std::is_same_v<decltype(end), typename base::end_t>);
};

}  // namespace internal

template <class Adapter>
auto MakeListInterface() {
  using iterator_value_type = typename Adapter::iterator_value_type;
  using impl = internal::ListInterfaceImpl<iterator_value_type, Adapter>;
  ListInterface<iterator_value_type> interface {};
  interface.push_back = &impl::push_back;
  interface.pop_back = &impl::pop_back;
  interface.insert = &impl::insert;
  interface.erase = &impl::erase;
  interface.operator_subscript = &impl::operator_subscript;
  interface.size = &impl::size;
  interface.empty = &impl::empty;
  interface.clear = &impl::clear;
  interface.begin = &impl::begin;
  interface.end = &impl::end;
  return interface;
}

}  // namespace liteproto