//
// Created by Youtao Guo on 26/6/23.
//

#pragma once

#include <memory>

#include "liteproto/interface.hpp"
#include "liteproto/iterator.hpp"
#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/type.hpp"
#include "liteproto/traits.hpp"

namespace liteproto {

template <class Tp>
class List<Tp, ConstOption::NON_CONST> {
  template <class C, class>
  friend auto AsList(C* container) noexcept;

  friend class List<Tp, ConstOption::CONST>;

 public:
  using iterator = Iterator<Tp>;

  void push_back(const Tp& v) const { interface_.push_back(obj_, v); }
  void push_back(Tp&& v) const { interface_.emplace_back(obj_, std::move(v)); }
  void pop_back() const { interface_.pop_back(obj_); }

  iterator insert(iterator pos, const Tp& v) const { return interface_.insert(obj_, std::move(pos), v); }
  iterator insert(iterator pos, Tp&& v) const { return interface_.emplace_insert(obj_, std::move(pos), std::move(v)); }
  iterator erase(iterator pos) const { return interface_.erase(obj_, pos); }

  decltype(auto) operator[](size_t pos) const { return interface_.operator_subscript(obj_, pos); }

  void resize(size_t count) const { return interface_.resize(obj_, count); }
  void resize(size_t count, const Tp& v) { return interface_.resize_append(obj_, count, v); }

  size_t size() const noexcept { return interface_.size(obj_); }
  bool empty() const noexcept { return interface_.empty(obj_); }
  void clear() const { return interface_.clear(obj_); }

  decltype(auto) begin() const { return interface_.begin(obj_); }
  decltype(auto) end() const { return interface_.end(obj_); }

  List(const List& rhs) = default;
  List& operator=(const List&) = default;
  List(List&&) noexcept = default;
  List& operator=(List&&) noexcept = default;

 private:
  template <class Adapter>
  List(Adapter&& adapter, internal::ListInterface<Tp> interface) noexcept
      : obj_(std::forward<Adapter>(adapter)), interface_(interface) {
    static_assert(IsListV<List>, "Why is the Adapter<Tp, ConstOption::NON_CONST> itself is not a Adapter?");
    static_assert(std::is_nothrow_move_constructible_v<List>);
  }

  std::any obj_;
  internal::ListInterface<Tp> interface_;
};

template <class Tp>
class List<Tp, ConstOption::CONST> {
  template <class C, class>
  friend auto AsList(C* container) noexcept;

 public:
  decltype(auto) operator[](size_t pos) const noexcept { return interface_.operator_subscript(obj_, pos); }

  size_t size() const noexcept { return interface_.size(obj_); }
  bool empty() const noexcept { return interface_.empty(obj_); }

  using iterator = Iterator<const Tp>;

  decltype(auto) begin() const noexcept { return interface_.begin(obj_); }
  decltype(auto) end() const noexcept { return interface_.end(obj_); }

  List(const List& rhs) = default;
  List& operator=(const List&) = default;
  List(List&&) noexcept = default;
  List& operator=(List&&) noexcept = default;

  List(const List<Tp, ConstOption::NON_CONST>& rhs) noexcept
      : interface_(rhs.interface_.const_interface()), obj_(rhs.interface_.to_const(rhs.obj_)) {}
  List(List<Tp, ConstOption::NON_CONST>&& rhs) noexcept
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
  using interface_type = internal::ListInterface<std::conditional_t<std::is_same_v<Tp, Object>, Tp, const Tp>>;
  template <class Adapter>
  List(Adapter&& adapter, interface_type interface) noexcept
      : obj_(std::forward<Adapter>(adapter)), interface_(interface) {}

  std::any obj_;
  interface_type interface_;
};

template <ConstOption ConstOpt>
class List<const Object, ConstOpt> {
 public:
  List() = delete;
};

template <ConstOption ConstOpt>
class List<volatile Object, ConstOpt> {
 public:
  List() = delete;
};

template <ConstOption ConstOpt>
class List<const volatile Object, ConstOpt> {
 public:
  List() = delete;
};

namespace internal {

template <class Tp, class = void>
class ListAdapter;

template <class Tp>
class ListAdapter<Tp, std::enable_if_t<IsListV<Tp>>> {
  static_assert(!std::is_reference_v<Tp>);
  using list_traits = ListTraits<Tp>;

 public:
  // What is the ListAdapter for Object supposed to do?
  // It still directly accesses the indirect object inside the class. Only if when visiting through the methods,
  // the adapter creates an Object instance as the proxy of underlying indirect object.
  static constexpr bool is_indirect = IsIndirectTypeV<typename list_traits::value_type>;

  using container_type = typename list_traits::container_type;
  using value_type = std::conditional_t<is_indirect, Object, typename list_traits::value_type>;
  using unerlying_value_type = typename list_traits::value_type;
  using iterator = typename List<value_type, static_cast<ConstOption>(std::is_const_v<container_type>)>::iterator;
  using iterator_value_type = typename iterator::value_type;
  using iterator_reference = typename iterator::reference;
  using iterator_adapter = IteratorAdapter<container_type, iterator_value_type>;
  // There is no ListAdapter<const Object>;
  using maybe_const_adapter = std::conditional_t<is_indirect, ListAdapter, ListAdapter<const Tp, void>>;

  explicit ListAdapter(container_type* c) noexcept : container_(c) {
    static_assert(std::is_copy_constructible_v<ListAdapter>);
    static_assert(std::is_copy_assignable_v<ListAdapter>);
    static_assert(std::is_nothrow_move_constructible_v<ListAdapter>);
    static_assert(std::is_trivially_copyable_v<ListAdapter>);
  }

  template <class Value>
  void push_back(Value&& v) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      if constexpr (!std::is_same_v<value_type, Object>) {
        container_->push_back(std::forward<Value>(v));
      } else {
        auto v_ptr = ObjectCast<unerlying_value_type>(v);
        if (v_ptr != nullptr) {
          // If this is an indirect interface, all the push_back & insert operations are considered as pass by "move".
          container_->push_back(std::move(*v_ptr));
        }
      }
    }
  }

  void pop_back() const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      container_->pop_back();
    }
  }

  // operator[] could be either const or non-const. So the return type should be exact the iterator_value_type.
  iterator_reference operator[](size_t pos) const {
    using std::begin;
    auto it = begin(*container_);
    std::advance(it, pos);
    if constexpr (!std::is_same_v<value_type, Object>) {
      return *it;
    } else {
      return GetReflection(&(*it));
    }
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
      if constexpr (!std::is_same_v<value_type, Object>) {
        container_->resize(count, v);
      } else {
        auto v_ptr = ObjectCast<unerlying_value_type>(v);
        if (v_ptr != nullptr) {
          container_->resize(count, *v_ptr);
        }
      }
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
    return internal::MakeIterator(std::move(adapter), MakeIteratorInterface<decltype(adapter)>());
  }

  iterator end() const noexcept {
    iterator_adapter adapter{container_->end()};
    return internal::MakeIterator(std::move(adapter), MakeIteratorInterface<decltype(adapter)>());
  }

  iterator insert(iterator pos, const value_type& v) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      if constexpr (!std::is_same_v<value_type, Object>) {
        auto& any_iter = internal::GetIteratorAdapter(pos);
        auto rhs_it = std::any_cast<iterator_adapter>(&any_iter);
        if (rhs_it == nullptr) {
          return end();
        }
        if constexpr (!std::is_same_v<value_type, Object>) {
          rhs_it->InsertMyself(container_, v);
          return pos;
        } else {
          auto v_ptr = ObjectCast<unerlying_value_type>(v);
          if (v_ptr != nullptr) {
            // If this is an indirect interface, all the push_back & insert operations are considered as pass by "move".
            rhs_it.InsertMyself(container_, std::move(*v_ptr));
          }
        }
      }
    }
    return end();
  }

  iterator erase(iterator pos) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      auto& any_iter = internal::GetIteratorAdapter(pos);
      auto rhs_it = std::any_cast<iterator_adapter>(&any_iter);
      if (rhs_it != nullptr) {
        rhs_it->EraseMyself(container_);
        return pos;
      }
    }
    return end();
  }

  [[nodiscard]] std::any ToConst() const noexcept { return maybe_const_adapter{container_}; }

 private:
  container_type* container_;
};

}  // namespace internal

template <class C, class = internal::ListAdapter<C>>
auto AsList(C* container) noexcept {
  internal::ListAdapter<C> adapter{container};

  static_assert(std::is_trivially_copyable_v<decltype(adapter)>);

  using value_type = typename internal::ListAdapter<C>::value_type;
  constexpr auto const_opt = static_cast<ConstOption>(std::is_const_v<C>);
  List<value_type, const_opt> list{adapter, internal::MakeListInterface<decltype(adapter)>()};
  return list;
}

}  // namespace liteproto