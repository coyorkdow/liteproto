//
// Created by Youtao Guo on 26/6/23.
//

#pragma once

#include <memory>

#include "liteproto/interface.hpp"
#include "liteproto/iterator.hpp"
#include "liteproto/reflect/type.hpp"

namespace liteproto {

template <class Tp, ConstOption ConstOpt>
class List;

template <class Tp>
class List<Tp, ConstOption::NON_CONST> {
  template <class C, class>
  friend auto AsList(C&& container);

  friend class List<Tp, ConstOption::CONST>;

 public:
  using iterator = Iterator<Tp>;

  void push_back(Tp v) const { interface_.push_back(obj_, std::move(v)); }
  void pop_back() const { interface_.pop_back(obj_); }

  iterator insert(iterator pos, Tp v) const { return interface_.insert(obj_, pos, std::move(v)); }
  iterator erase(iterator pos) const { return interface_.erase(obj_, pos); }

  decltype(auto) operator[](size_t pos) const { return interface_.operator_subscript(obj_, pos); }

  void resize(size_t count) const { return interface_.resize(obj_, count); }
  void resize(size_t count, const Tp& v) { return interface_.resize_append(obj_, count, v); }

  size_t size() const noexcept { return interface_.size(obj_); }
  bool empty() const noexcept { return interface_.empty(obj_); }
  void clear() const { return interface_.clear(obj_); }

  decltype(auto) begin() const { return interface_.begin(obj_); }
  decltype(auto) end() const { return interface_.end(obj_); }

  List(const List& rhs) noexcept = default;
  List& operator=(const List&) noexcept = default;
  List(List&&) noexcept = default;
  List& operator=(List&&) noexcept = default;

  operator List<Tp, ConstOption::CONST>() noexcept { return List<Tp, ConstOption::CONST>{*this}; }

 private:
  template <class Adapter>
  List(Adapter&& adapter, internal::ListInterface<Tp> interface) noexcept
      : obj_(std::forward<Adapter>(adapter)), interface_(interface) {
    static_assert(IsListV<List>, "Why is the Adapter<Tp, ConstOption::NON_CONST> itself is not a Adapter?");
    static_assert(std::is_nothrow_move_constructible_v<List>);
  }

  internal::ListInterface<Tp> interface_;
  mutable std::any obj_;
};

template <class Tp>
class List<Tp, ConstOption::CONST> {
  template <class C, class>
  friend auto AsList(C&& container);

 public:
  decltype(auto) operator[](size_t pos) const { return interface_.operator_subscript(obj_, pos); }

  size_t size() const noexcept { return interface_.size(obj_); }
  bool empty() const noexcept { return interface_.empty(obj_); }

  using iterator = Iterator<const Tp>;

  decltype(auto) begin() const noexcept { return interface_.begin(obj_); }
  decltype(auto) end() const noexcept { return interface_.end(obj_); }

  List(const List& rhs) noexcept = default;
  List& operator=(const List&) noexcept = default;
  List(List&&) noexcept = default;
  List& operator=(List&&) noexcept = default;

  explicit List(const List<Tp, ConstOption::NON_CONST>& rhs) noexcept
      : interface_(rhs.interface_.const_interface()), obj_(rhs.interface_.to_const(rhs.obj_)) {}
  explicit List(List<Tp, ConstOption::NON_CONST>&& rhs) noexcept
      : interface_(rhs.interface_.const_interface()), obj_(rhs.interface_.to_const(rhs.obj_)) {}
  List& operator=(const List<Tp, ConstOption::NON_CONST>& rhs) noexcept {
    this->interface_ = rhs.interface_.const_interface();
    this->obj_ = rhs.interface_.to_const(rhs.obj_);
    return *this;
  }
  List& operator=(List<Tp, ConstOption::NON_CONST>&& rhs) noexcept {
    static_assert(std::is_lvalue_reference_v<decltype(rhs)&>);
    *this = static_cast<decltype(rhs)&>(rhs);
    return *this;
  }

 private:
  template <class Adapter>
  List(Adapter&& adapter, internal::ListInterface<const Tp> interface) noexcept
      : obj_(std::forward<Adapter>(adapter)), interface_(interface) {}

  internal::ListInterface<const Tp> interface_;
  mutable std::any obj_;
};

namespace internal {

template <class Tp>
class ListAdapter<Tp, std::enable_if_t<IsListV<Tp>>> {
  static_assert(!std::is_reference_v<Tp>);
  using list_traits = ListTraits<Tp>;

 public:
  using container_type = typename list_traits::container_type;  // TODO recursive container_type
  using value_type = typename list_traits::value_type;
  using iterator = typename List<value_type, static_cast<ConstOption>(std::is_const_v<container_type>)>::iterator;
  using iterator_value_type = std::conditional_t<std::is_const_v<container_type>, const value_type, value_type>;
  using iterator_adapter = IteratorAdapter<container_type, iterator_value_type>;

  using const_adapter = ListAdapter<const Tp, void>;

  explicit ListAdapter(container_type* c) : container_(c) {
    static_assert(std::is_copy_constructible_v<ListAdapter>);
    static_assert(std::is_copy_assignable_v<ListAdapter>);
    static_assert(std::is_nothrow_move_constructible_v<ListAdapter>);
    static_assert(std::is_trivially_copyable_v<ListAdapter>);
  }

  void push_back(value_type v) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      container_->push_back(std::move(v));
    }
  }
  void pop_back() const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      container_->pop_back();
    }
  }

  // operator[] could be either const or non-const. So the return type should be exact the iterator_value_type.
  iterator_value_type& operator[](size_t pos) const {
    using std::begin;
    auto it = begin(*container_);
    std::advance(it, pos);
    return *it;
  }

  void resize(size_t count) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      container_->resize(count);
    }
  }
  void resize(size_t count, const value_type& v) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      container_->resize(count, v);
    }
  }

  size_t size() const noexcept { return container_->size(); }
  bool empty() const noexcept { return container_->empty(); }
  void clear() const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      container_->clear();
    }
  }

  iterator begin() const noexcept {
    iterator_adapter adapter{container_->begin()};
    return iterator{std::move(adapter), MakeIteratorInterface<decltype(adapter)>()};
  }

  iterator end() const noexcept {
    iterator_adapter adapter{container_->end()};
    return iterator{std::move(adapter), MakeIteratorInterface<decltype(adapter)>()};
  }

  iterator insert(iterator pos, value_type v) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      auto any_iter = &pos.it_;
      auto rhs_it = std::any_cast<iterator_adapter>(any_iter);
      rhs_it->InsertMyself(container_, std::move(v));
    }
    return pos;
  }

  iterator erase(iterator pos) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      auto any_iter = &pos.it_;
      auto rhs_it = std::any_cast<iterator_adapter>(any_iter);
      rhs_it->EraseMyself(container_);
    }
    return pos;
  }

  [[nodiscard]] std::any ToConst() const noexcept { return const_adapter{container_}; }

 private:
  container_type* container_;
};

}  // namespace internal

template <class C, class = internal::ListAdapter<std::remove_reference_t<C>>>
auto AsList(C&& container) {
  using ref_removed = std::remove_reference_t<C>;
  internal::ListAdapter<ref_removed> adapter{&container};

  static_assert(std::is_trivially_copyable_v<decltype(adapter)>);

  using value_type = typename internal::ListAdapter<ref_removed>::value_type;
  constexpr auto const_opt = static_cast<ConstOption>(std::is_const_v<ref_removed>);
  List<value_type, const_opt> list{adapter, internal::MakeListInterface<decltype(adapter)>()};
  return list;
}

}  // namespace liteproto