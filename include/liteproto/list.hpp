//
// Created by Youtao Guo on 26/6/23.
//

#pragma once

#include "liteproto/interface.hpp"
#include "liteproto/iterator.hpp"
#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/type.hpp"
#include "liteproto/traits/traits.hpp"

namespace liteproto {

namespace internal {
template <class Tp, class = void>
class ListAdapter;
}

template <class C, class = internal::ListAdapter<C>>
auto AsList(C* container) noexcept;

template <class Tp>
class List<Tp, ConstOption::NON_CONST> {
  template <class C, class>
  friend auto AsList(C* container) noexcept;

  friend class List<Tp, ConstOption::CONST>;

  using traits = InterfaceTraits<Tp, ConstOption::NON_CONST>;
  using const_traits = InterfaceTraits<Tp, ConstOption::CONST>;

 public:
  using value_type = typename traits::value_type;
  using pointer = typename traits::pointer;
  using reference = typename traits::reference;
  using const_value_type = typename const_traits::value_type;
  using const_pointer = typename const_traits::pointer;
  using const_reference = typename const_traits::reference;
  using iterator = Iterator<value_type, pointer, reference>;
  using interface = internal::ListInterface<value_type, pointer, reference, const_value_type, const_pointer, const_reference>;

  void push_back(const Tp& v) const { interface_->push_back(obj_, v); }
  void push_back(Tp&& v) const { interface_->emplace_back(obj_, std::move(v)); }
  void pop_back() const { interface_->pop_back(obj_); }

  iterator insert(iterator pos, const Tp& v) const { return interface_->insert(obj_, std::move(pos), v); }
  iterator insert(iterator pos, Tp&& v) const { return interface_->emplace_insert(obj_, std::move(pos), std::move(v)); }
  iterator erase(iterator pos) const { return interface_->erase(obj_, pos); }

  decltype(auto) operator[](size_t pos) const { return interface_->operator_subscript(obj_, pos); }

  void resize(size_t count) const { return interface_->resize(obj_, count); }
  void resize(size_t count, const Tp& v) { return interface_->resize_append(obj_, count, v); }

  size_t size() const noexcept { return interface_->size(obj_); }
  bool empty() const noexcept { return interface_->empty(obj_); }
  void clear() const { return interface_->clear(obj_); }

  decltype(auto) begin() const { return interface_->begin(obj_); }
  decltype(auto) end() const { return interface_->end(obj_); }

  List(const List& rhs) = default;
  List& operator=(const List&) = default;
  List(List&&) noexcept = default;
  List& operator=(List&&) noexcept = default;

 protected:
  template <class Adapter>
  List(Adapter&& adapter, const interface& interface) noexcept : obj_(std::forward<Adapter>(adapter)), interface_(&interface) {
    static_assert(IsListV<List>, "Why the List<Tp, ConstOption::NON_CONST> itself is not a List?");
    static_assert(std::is_nothrow_move_constructible_v<List>);
  }

  std::any obj_;
  const interface* interface_;
};

template <class Tp>
class List<Tp, ConstOption::CONST> {
  template <class C, class>
  friend auto AsList(C* container) noexcept;

  using traits = InterfaceTraits<Tp, ConstOption::CONST>;

 public:
  using value_type = typename traits::value_type;
  using pointer = typename traits::pointer;
  using reference = typename traits::reference;
  using iterator = Iterator<value_type, pointer, reference>;
  using interface = internal::ListInterface<value_type, pointer, reference>;

  decltype(auto) operator[](size_t pos) const noexcept { return interface_->operator_subscript(obj_, pos); }

  size_t size() const noexcept { return interface_->size(obj_); }
  bool empty() const noexcept { return interface_->empty(obj_); }

  decltype(auto) begin() const noexcept { return interface_->begin(obj_); }
  decltype(auto) end() const noexcept { return interface_->end(obj_); }

  List(const List& rhs) = default;
  List& operator=(const List&) = default;
  List(List&&) noexcept = default;
  List& operator=(List&&) noexcept = default;

  List(const List<Tp, ConstOption::NON_CONST>& rhs) noexcept
      : obj_(rhs.interface_->to_const(rhs.obj_)), interface_(rhs.interface_->const_interface()) {}
  List(List<Tp, ConstOption::NON_CONST>&& rhs) noexcept
      : obj_(rhs.interface_->to_const(rhs.obj_)), interface_(rhs.interface_->const_interface()) {}
  List& operator=(const List<Tp, ConstOption::NON_CONST>& rhs) noexcept {
    this->obj_ = rhs.interface_->to_const(rhs.obj_);
    this->interface_ = &rhs.interface_->const_interface();
    return *this;
  }
  List& operator=(List<Tp, ConstOption::NON_CONST>&& rhs) noexcept {
    static_assert(std::is_lvalue_reference_v<decltype(rhs)&>);
    *this = static_cast<decltype(rhs)&>(rhs);
    return *this;
  }

 protected:
  template <class Adapter>
  List(Adapter&& adapter, const interface& interface) noexcept : obj_(std::forward<Adapter>(adapter)), interface_(&interface) {}

  std::any obj_;
  const interface* interface_;
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

template <class Tp>
class ListAdapter<Tp, std::enable_if_t<IsListV<Tp>>> {
  static_assert(!std::is_reference_v<Tp>);
  using list_traits = ListTraits<Tp>;

 public:
  using container_type = typename list_traits::container_type;
  using underlying_value_type = typename list_traits::value_type;
  static inline constexpr bool is_const = std::is_const_v<container_type>;

 private:
  using traits = InterfaceTraits<typename ProxyType<underlying_value_type>::type, static_cast<ConstOption>(is_const)>;
  using const_traits = InterfaceTraits<typename ProxyType<underlying_value_type>::type, ConstOption::CONST>;

 public:
  using value_type = typename traits::value_type;
  using pointer = typename traits::pointer;
  using reference = typename traits::reference;
  using const_value_type = typename const_traits::value_type;
  using const_pointer = typename const_traits::pointer;
  using const_reference = typename const_traits::reference;
  using iterator = Iterator<value_type, pointer, reference>;

  // What is the ListAdapter for Object supposed to do?
  // It still directly accesses the indirect object inside the class. Only if when visiting through the methods,
  // the adapter creates an Object instance as the proxy of underlying indirect object.
  using iterator_adapter = IteratorAdapter<container_type, value_type, pointer, reference>;
  using const_adapter = ListAdapter<const Tp, void>;

  explicit ListAdapter(container_type* c) noexcept : container_(c) {
    static_assert(std::is_copy_constructible_v<ListAdapter>);
    static_assert(std::is_copy_assignable_v<ListAdapter>);
    static_assert(std::is_nothrow_move_constructible_v<ListAdapter>);
    static_assert(std::is_trivially_copyable_v<ListAdapter>);
  }

  template <class Value>
  void push_back(Value&& v) const {
    if constexpr (std::is_const_v<container_type>) {
      // If the container is const, do nothing. And it's assured that this method will never be called.
    } else {
      if constexpr (IsObjectV<value_type>) {
        static_assert(IsObjectV<std::remove_reference_t<Value>>);
        auto v_ptr = ObjectCast<underlying_value_type>(v);
        if (v_ptr != nullptr) {
          // If this is an indirect interface, all the push_back & insert operations are considered as pass by "move".
          container_->push_back(std::move(*v_ptr));
        }
      } else if constexpr (IsNumberV<value_type>) {
        static_assert(IsNumberV<std::remove_reference_t<Value>>);
        if (v.IsSignedInteger()) {
          container_->push_back(v.AsInt64());
        } else if (v.IsUnsigned()) {
          container_->push_back(v.AsUInt64());
        } else {
          container_->push_back(v.AsFloat64());
        }
      } else {
        container_->push_back(std::forward<Value>(v));
      }
    }
  }

  void pop_back() const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      container_->pop_back();
    }
  }

  // operator[] could be either const or non-const.
  reference operator[](size_t pos) const {
    using std::begin;
    auto it = begin(*container_);
    std::advance(it, pos);
    if constexpr (IsProxyTypeV<reference>) {
      return MakeProxy<reference>(*it);
    } else {
      return *it;
    }
  }

  void resize(size_t count) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      container_->resize(count);
    }
  }

  void resize(size_t count, const value_type& v) const {
    if constexpr (std::is_const_v<container_type>) {
      // If the container is const, do nothing. And it's assured that this method will never be called.
    } else {
      if constexpr (IsObjectV<value_type>) {
        auto v_ptr = ObjectCast<underlying_value_type>(v);
        if (v_ptr != nullptr) {
          container_->resize(count, *v_ptr);
        }
      } else if constexpr (IsNumberV<value_type>) {
        if (v.IsSignedInteger()) {
          container_->resize(count, v.AsInt64());
        } else if (v.IsUnsigned()) {
          container_->resize(count, v.AsUInt64());
        } else {
          container_->resize(count, v.AsFloat64());
        }
      } else {
        container_->resize(count, v);
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
    iterator_adapter it_adapter{container_->begin()};
    return internal::MakeIterator(std::move(it_adapter), GetIteratorInterface<decltype(it_adapter)>());
  }

  iterator end() const noexcept {
    iterator_adapter it_adapter{container_->end()};
    return internal::MakeIterator(std::move(it_adapter), GetIteratorInterface<decltype(it_adapter)>());
  }

  template <class Value>
  iterator insert(iterator pos, Value&& v) const {
    if constexpr (std::is_const_v<container_type>) {
      // If the container is const, do nothing. And it's assured that this method will never be called.
    } else {
      auto& any_iter = internal::GetIteratorAdapter(pos);
      auto rhs_it = std::any_cast<iterator_adapter>(&any_iter);
      if (rhs_it == nullptr) {
        return end();
      }
      if constexpr (IsObjectV<value_type>) {
        static_assert(IsObjectV<std::remove_reference_t<Value>>);
        auto v_ptr = ObjectCast<underlying_value_type>(v);
        if (v_ptr != nullptr) {
          // If this is an indirect interface, all the push_back & insert operations are considered as pass by "move".
          rhs_it->InsertMyself(container_, std::move(*v_ptr));
        }
      } else if constexpr (IsNumberV<value_type>) {
        static_assert(IsNumberV<std::remove_reference_t<Value>>);
        if (v.IsSignedInteger()) {
          rhs_it->InsertMyself(container_, v.AsInt64());
        } else if (v.IsUnsigned()) {
          rhs_it->InsertMyself(container_, v.AsUInt64());
        } else {
          rhs_it->InsertMyself(container_, v.AsFloat64());
        }
      } else {
        rhs_it->InsertMyself(container_, std::forward<Value>(v));
        return pos;
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

  [[nodiscard]] std::any ToConst() const noexcept { return const_adapter{container_}; }

 protected:
  container_type* container_;
};

}  // namespace internal

template <class C, class>
auto AsList(C* container) noexcept {
  internal::ListAdapter<C> adapter{container};

  static_assert(std::is_trivially_copyable_v<decltype(adapter)>);

  using value_type = typename internal::ListAdapter<C>::value_type;
  constexpr auto const_opt = static_cast<ConstOption>(std::is_const_v<C>);
  List<value_type, const_opt> list{adapter, internal::GetListInterface<decltype(adapter)>()};
  return list;
}

}  // namespace liteproto