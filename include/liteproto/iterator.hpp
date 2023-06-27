//
// Created by Youtao Guo on 2023/6/27.
//

#pragma once

#include <any>
#include <type_traits>

#include "liteproto/reflect/type.hpp"

namespace liteproto {

template <class Tp>
class Iterator;

namespace internal {

template <class Tp, class Cond = void>
class ListAdapter;

template <class Tp>
struct IteratorInterface {
 public:
  using pointer = Tp*;
  using reference = Tp&;

  using indirection_t = reference(const std::any&) noexcept;
  using member_of_object_t = pointer(const std::any&) noexcept;
  using increment_t = void(std::any&);
  using noteq_t = bool(const std::any&, const Iterator<Tp>&) noexcept;

  using adapter_type_id_t = uint64_t(const std::any&) noexcept;

  indirection_t* indirection;
  member_of_object_t* member_of_object;
  increment_t* increment;
  noteq_t* noteq;
  adapter_type_id_t* adapter_type_id;
};

template <class It, class Tp>
struct IteratorInterfaceImpl {
  using base = IteratorInterface<Tp>;
  using pointer = Tp*;
  using reference = Tp&;

  static reference Indirection(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<It>(&obj);
    return *(*ptr);
  }
  static_assert(std::is_same_v<decltype(Indirection), typename base::indirection_t>);

  static pointer MemberOfObject(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<It>(&obj);
    return (*ptr).operator->();
  }
  static_assert(std::is_same_v<decltype(MemberOfObject), typename base::member_of_object_t>);

  static void Increment(std::any& obj) {
    auto* ptr = std::any_cast<It>(&obj);
    (*ptr).Increment();
  }
  static_assert(std::is_same_v<decltype(Increment), typename base::increment_t>);

  static bool NotEq(const std::any& obj, const Iterator<Tp>& rhs) noexcept {
    auto* ptr = std::any_cast<It>(&obj);
    return (*ptr) != rhs;
  }
  static_assert(std::is_same_v<decltype(NotEq), typename base::noteq_t>);

  static uint64_t AdapterTypeId(const std::any& obj) noexcept {
    auto* ptr = std::any_cast<It>(&obj);
    return (*ptr).AdapterTypeId();
  }
  static_assert(std::is_same_v<decltype(AdapterTypeId), typename base::adapter_type_id_t>);
};

template <class It>
auto MakeIteratorInterface() {
  using value_type = typename It::value_type;
  using impl = IteratorInterfaceImpl<It, value_type>;
  IteratorInterface<value_type> interface {};
  interface.indirection = &impl::Indirection;
  interface.member_of_object = &impl::MemberOfObject;
  interface.increment = &impl::Increment;
  interface.noteq = &impl::NotEq;
  interface.adapter_type_id = &impl::AdapterTypeId;
  return interface;
}

template <class Container, class Tp>
class IteratorAdapter {
 public:
  using pointer = typename IteratorInterface<Tp>::pointer;
  using reference = typename IteratorInterface<Tp>::reference;
  using container_type = Container;
  using value_type = Tp;
  using wrapped_iterator = std::conditional_t<std::is_const_v<container_type>, typename container_type::const_iterator,
                                              typename container_type::iterator>;

  explicit IteratorAdapter(const wrapped_iterator& it) : it_(it) {
    static_assert(std::is_same_v<value_type, typename container_type::value_type> ||
                  std::is_same_v<value_type, const typename container_type::value_type>);
    static_assert(std::is_copy_constructible<IteratorAdapter>::value);
    static_assert(std::is_copy_assignable<IteratorAdapter>::value);
    static_assert(std::is_swappable<IteratorAdapter>::value);
  }

  reference operator*() const { return *it_; }
  pointer operator->() const { return it_.operator->(); }
  void Increment() { ++it_; }
  bool operator!=(const Iterator<Tp>& rhs) const {
    if (AdapterTypeId() != rhs.AdapterTypeId()) {
      return true;
    }
    auto any_iter = &rhs.it_;
    auto rhs_it = std::any_cast<IteratorAdapter>(any_iter);
    return it_ != rhs_it->it_;
  }

  uint64_t AdapterTypeId() const { return TypeMeta<IteratorAdapter>::Id(); }

  IteratorAdapter& InsertMyself(container_type* container, value_type v) {
    if constexpr (!std::is_const_v<container_type>) {
      it_ = container->insert(it_, std::move(v));
    }
    return *this;
  }

  IteratorAdapter& EraseMyself(container_type* container) {
    if constexpr (!std::is_const_v<container_type>) {
      it_ = container->erase(it_);
    }
    return *this;
  }

 private:
  wrapped_iterator it_;
};

}  // namespace internal

template <class Tp>
class Iterator {
  template <class Tp_, class Cond>
  friend class internal::ListAdapter;

  template <class Container, class Tp_>
  friend class internal::IteratorAdapter;

 public:
  using value_type = Tp;
  using difference_type = std::ptrdiff_t;
  using pointer = Tp*;
  using reference = Tp&;
  using iterator_category = std::forward_iterator_tag;

  reference operator*() noexcept { return interface_.indirection(it_); }
  pointer operator->() noexcept { return interface_.member_of_object(it_); }
  Iterator& operator++() {
    interface_.increment(it_);
    return *this;
  }
  Iterator operator++(int) {
    Iterator old = *this;
    ++(*this);
    return old;
  }
  bool operator==(const Iterator& rhs) const noexcept { return !(*this != rhs); }
  bool operator!=(const Iterator& rhs) const noexcept { return interface_.noteq(it_, rhs); }

  uint64_t AdapterTypeId() const noexcept { return interface_.adapter_type_id(it_); }

 private:
  template <class ItAdapter>
  Iterator(ItAdapter&& it, internal::IteratorInterface<Tp> inter)
      : it_(std::forward<ItAdapter>(it)), interface_(inter) {
    static_assert(std::is_copy_constructible<Iterator>::value);
    static_assert(std::is_copy_assignable<Iterator>::value);
    static_assert(std::is_swappable<Iterator>::value);
  }

  internal::IteratorInterface<Tp> interface_;
  mutable std::any it_;
};

}  // namespace liteproto
