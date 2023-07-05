//
// Created by Youtao Guo on 2023/6/27.
//

#pragma once

#include <any>
#include <type_traits>

#include "liteproto/reflect/number.hpp"
#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/proxy.hpp"
#include "liteproto/reflect/type.hpp"

namespace liteproto {

template <class Tp, class Pointer, class Reference, class Category = std::bidirectional_iterator_tag>
class Iterator;

template <class Tp, class Category = std::bidirectional_iterator_tag>
using DefaultIterator = Iterator<Tp, Tp*, Tp&, Category>;

template <class Category = std::bidirectional_iterator_tag>
using ObjectIterator = Iterator<Object, internal::DummyPointer, Object, Category>;

template <class Category = std::bidirectional_iterator_tag>
using NumberIterator = Iterator<Number, internal::DummyPointer, NumberReference<ConstOption::NON_CONST>, Category>;

template <class Category = std::bidirectional_iterator_tag>
using ConstNumberIterator = Iterator<Number, internal::DummyPointer, NumberReference<ConstOption::CONST>, Category>;

namespace internal {

template <class Container, class Tp, class Pointer, class Reference>
class IteratorAdapter;

template <class Tp, class Pointer, class Reference>
struct IteratorInterface {
 public:
  using pointer = Pointer;
  using reference = Reference;
  //  using reference_or_value = typename ReferenceType<Tp>::type;

  using indirection_t = reference(const std::any&) noexcept;
  using member_of_object_t = pointer(const std::any&) noexcept;
  using increment_t = void(std::any&);
  using decrement_t = void(std::any&);
  using noteq_t = bool(const std::any&, const Iterator<Tp, pointer, reference>&) noexcept;

  using adapter_type_id_t = uint64_t(const std::any&) noexcept;
  indirection_t* indirection;
  member_of_object_t* member_of_object;
  increment_t* increment;
  decrement_t* decrement;
  noteq_t* noteq;
  adapter_type_id_t* adapter_type_id;
};

template <class IteratorAdapter, class Tp, class Pointer, class Reference, class Category = std::bidirectional_iterator_tag>
auto MakeIterator(IteratorAdapter&& it, const internal::IteratorInterface<Tp, Pointer, Reference>& inter) noexcept;

template <class Tp, class Pointer, class Reference, class Category>
std::any& GetIteratorAdapter(Iterator<Tp, Pointer, Reference, Category>&) noexcept;

template <class Tp, class Pointer, class Reference, class Category>
const std::any& GetIteratorAdapter(const Iterator<Tp, Pointer, Reference, Category>&) noexcept;

}  // namespace internal

template <class Tp, class Pointer, class Reference, class Category>
class IteratorBase {
  using iterator = Iterator<Tp, Pointer, Reference, Category>;

 public:
  using value_type = Tp;
  using difference_type = std::ptrdiff_t;
  using pointer = Pointer;
  using reference = Reference;
  using iterator_category = std::bidirectional_iterator_tag;

  using interface = internal::IteratorInterface<value_type, pointer, reference>;

  reference operator*() noexcept { return interface_->indirection(it_); }

  iterator& operator++() {
    interface_->increment(it_);
    return static_cast<iterator&>(*this);
  }
  iterator operator++(int) {
    Iterator old{*this};
    ++(*this);
    return old;
  }

  auto operator--() -> std::enable_if_t<std::is_base_of_v<std::bidirectional_iterator_tag, Category>, iterator&> {
    interface_->decrement(it_);
    return static_cast<iterator&>(*this);
  }
  auto operator--(int) -> std::enable_if_t<std::is_base_of_v<std::bidirectional_iterator_tag, Category>, iterator> {
    Iterator old{*this};
    --(*this);
    return old;
  }

  [[nodiscard]] uint64_t AdapterTypeId() const noexcept { return interface_->adapter_type_id(it_); }

  IteratorBase() = default;
  IteratorBase(const IteratorBase&) = default;
  IteratorBase(IteratorBase&&) noexcept = default;
  IteratorBase& operator=(const IteratorBase&) = default;
  IteratorBase& operator=(IteratorBase&&) noexcept = default;

 protected:
  template <class ItAdapter>
  IteratorBase(ItAdapter&& it, const interface& inter) noexcept : it_(std::forward<ItAdapter>(it)), interface_(&inter) {
    static_assert(std::is_copy_constructible<IteratorBase>::value);
    static_assert(std::is_copy_assignable<IteratorBase>::value);
    static_assert(std::is_swappable<IteratorBase>::value);
  }

  std::any it_;
  const interface* interface_;
};

template <class Tp, class Pointer, class Reference, class Category>
class Iterator : public IteratorBase<Tp, Pointer, Reference, Category> {
  template <class IteratorAdapter, class T, class P, class R, class C>
  friend auto internal::MakeIterator(IteratorAdapter&& it, const internal::IteratorInterface<T, P, R>& inter) noexcept;

  template <class T, class P, class R, class C>
  friend std::any& internal::GetIteratorAdapter(Iterator<T, P, R, C>&) noexcept;

  template <class T, class P, class R, class C>
  friend const std::any& internal::GetIteratorAdapter(const Iterator<T, P, R, C>&) noexcept;

  template <class Container, class T, class P, class R>
  friend class internal::IteratorAdapter;

  friend class IteratorBase<Tp, Pointer, Reference, Category>;
  using base = IteratorBase<Tp, Pointer, Reference, Category>;

 public:
  Iterator() = default;
  Iterator(const Iterator&) = default;
  Iterator(Iterator&&) noexcept = default;
  Iterator& operator=(const Iterator&) = default;
  Iterator& operator=(Iterator&&) noexcept = default;

  bool operator==(const Iterator& rhs) const noexcept { return !(*this != rhs); }
  bool operator!=(const Iterator& rhs) const noexcept { return base::interface_->noteq(base::it_, rhs); }
  typename base::pointer operator->() noexcept { return base::interface_->member_of_object(base::it_); }

 private:
  template <class ItAdapter>
  Iterator(ItAdapter&& it, const typename base::interface& inter) noexcept : base(std::forward<ItAdapter>(it), inter) {}

  explicit Iterator(const base& iterator) : base(iterator) {}
  explicit Iterator(base&& iterator) noexcept : base(std::move(iterator)) {}
};

template <class Tp, class Reference, class Category>
class Iterator<Tp, internal::DummyPointer, Reference, Category> : public IteratorBase<Tp, internal::DummyPointer, Reference, Category> {
  template <class IteratorAdapter, class T, class P, class R, class C>
  friend auto internal::MakeIterator(IteratorAdapter&& it, const internal::IteratorInterface<T, P, R>& inter) noexcept;

  template <class T, class P, class R, class C>
  friend std::any& internal::GetIteratorAdapter(Iterator<T, P, R, C>&) noexcept;

  template <class T, class P, class R, class C>
  friend const std::any& internal::GetIteratorAdapter(const Iterator<T, P, R, C>&) noexcept;

  template <class Container, class T, class P, class R>
  friend class internal::IteratorAdapter;

  friend class IteratorBase<Tp, internal::DummyPointer, Reference, Category>;
  using base = IteratorBase<Tp, internal::DummyPointer, Reference, Category>;

 public:
  Iterator() = default;
  Iterator(const Iterator&) = default;
  Iterator(Iterator&&) noexcept = default;
  Iterator& operator=(const Iterator&) = default;
  Iterator& operator=(Iterator&&) noexcept = default;

  bool operator==(const Iterator& rhs) const noexcept { return !(*this != rhs); }
  bool operator!=(const Iterator& rhs) const noexcept { return base::interface_->noteq(base::it_, rhs); }

 private:
  template <class ItAdapter>
  Iterator(ItAdapter&& it, const typename base::interface& inter) noexcept : base(std::forward<ItAdapter>(it), inter) {}

  explicit Iterator(const base& iterator) : base(iterator) {}
  explicit Iterator(base&& iterator) noexcept : base(std::move(iterator)) {}
};

namespace internal {

template <class IteratorAdapter, class Tp, class Pointer, class Reference, class Category>
auto MakeIterator(IteratorAdapter&& it, const IteratorInterface<Tp, Pointer, Reference>& inter) noexcept {
  return Iterator<Tp, Pointer, Reference, Category>(std::forward<IteratorAdapter>(it), inter);
}

template <class Tp, class Pointer, class Reference, class Category>
std::any& GetIteratorAdapter(Iterator<Tp, Pointer, Reference, Category>& iterator) noexcept {
  return iterator.it_;
}

template <class Tp, class Pointer, class Reference, class Category>
const std::any& GetIteratorAdapter(const Iterator<Tp, Pointer, Reference, Category>& iterator) noexcept {
  return iterator.it_;
}

template <class It, class Tp, class Pointer, class Reference>
struct IteratorInterfaceImpl {
  using base = IteratorInterface<Tp, Pointer, Reference>;
  using pointer = typename base::pointer;
  using reference = typename base::reference;

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

  static void Decrement(std::any& obj) {
    auto* ptr = std::any_cast<It>(&obj);
    (*ptr).Decrement();
  }
  static_assert(std::is_same_v<decltype(Decrement), typename base::decrement_t>);

  static bool NotEq(const std::any& obj, const Iterator<Tp, Pointer, Reference>& rhs) noexcept {
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
const IteratorInterface<typename It::value_type, typename It::pointer, typename It::reference>& GetIteratorInterface() noexcept {
  using value_type = typename It::value_type;
  using pointer = typename It::pointer;
  using reference = typename It::reference;
  using impl = IteratorInterfaceImpl<It, value_type, pointer, reference>;
  static const IteratorInterface<value_type, pointer, reference> inter = [&] {
    IteratorInterface<value_type, pointer, reference> interface {};
    interface.indirection = &impl::Indirection;
    interface.member_of_object = &impl::MemberOfObject;
    interface.increment = &impl::Increment;
    interface.decrement = &impl::Decrement;
    interface.noteq = &impl::NotEq;
    interface.adapter_type_id = &impl::AdapterTypeId;
    return interface;
  }();
  return inter;
}

template <class Container, class Tp, class Pointer, class Reference>
class IteratorAdapter {
 public:
  static_assert(!std::is_reference_v<Tp>);

 public:
  using value_type = Tp;
  using pointer = Pointer;
  using reference = Reference;

  // Iterator<Container, Object> is special, it is used to iterate an interface of the indirect type.
  // What should an adapter of the Object iterator do?
  // The adapter still iterates the underlying container, but returns Object instead of the real value_type.
  // The Object, as well as all other Interface types, are essentially some sort of fat pointers.
  // We don't store any additional data in the adapter. Instead, the Object returned by adapter is created when the
  // corresponding method is called. Therefore, the return type of indirection operator is value type, not
  // reference_or_value type.
  static_assert(IsProxyTypeV<value_type> || std::is_reference_v<reference>,
                "IteratorAdapter uses reference_or_value for all types except Object or Number");
  using container_type = Container;
  using wrapped_iterator =
      std::conditional_t<std::is_const_v<container_type>, typename container_type::const_iterator, typename container_type::iterator>;

  explicit IteratorAdapter(const wrapped_iterator& it) noexcept(noexcept(wrapped_iterator{it})) : it_(it) {
    static_assert(IsProxyTypeV<value_type> || std::is_same_v<value_type, typename container_type::value_type> ||
                  std::is_same_v<value_type, const typename container_type::value_type>);
    static_assert(std::is_copy_constructible<IteratorAdapter>::value);
    static_assert(std::is_copy_assignable<IteratorAdapter>::value);
    static_assert(std::is_swappable<IteratorAdapter>::value);
  }
  explicit IteratorAdapter(wrapped_iterator&& it) noexcept(noexcept(wrapped_iterator{std::move(it)})) : it_(std::move(it)) {}

  reference operator*() const noexcept(noexcept(*it_)) {
    if constexpr (IsProxyTypeV<reference>) {
      return MakeProxy<reference>(*it_);
    } else {
      return *it_;
    }
  }

  pointer operator->() const noexcept(noexcept(it_.operator->())) {
    if constexpr (!IsProxyTypeV<value_type>) {
      return it_.operator->();
    } else {
      // If the value_type is Object or Number, do nothing. And it's assured that this method will never be called.
      return nullptr;
    }
  }

  void Increment() noexcept(noexcept(++it_)) { ++it_; }
  void Decrement() noexcept(noexcept(--it_)) {
    if constexpr (is_bidirectional_iterator_v<wrapped_iterator>) {
      --it_;
    }
  }

  bool operator!=(const Iterator<value_type, pointer, reference>& rhs) const noexcept {
    if (AdapterTypeId() != rhs.AdapterTypeId()) {
      return true;
    }
    auto any_iter = &rhs.it_;
    auto rhs_it = std::any_cast<IteratorAdapter>(any_iter);
    return it_ != rhs_it->it_;
  }

  [[nodiscard]] uint64_t AdapterTypeId() const noexcept { return TypeMeta<IteratorAdapter>::Id(); }

  template <class V>
  IteratorAdapter& InsertMyself(container_type* container, V&& v) {
    if constexpr (!std::is_const_v<container_type>) {
      it_ = container->insert(it_, std::forward<V>(v));
    }
    return *this;
  }

  IteratorAdapter& EraseMyself(container_type* container) noexcept(noexcept(container->erase(it_))) {
    if constexpr (!std::is_const_v<container_type>) {
      it_ = container->erase(it_);
    }
    return *this;
  }

 private:
  wrapped_iterator it_;
};

//// Iterators of the Object with CV-qualified don't satisfies our semantics, disable them.
template <class Container, class P, class R>
class IteratorAdapter<Container, const Object, P, R> {
 public:
  IteratorAdapter() = delete;
};

template <class Container, class P, class R>
class IteratorAdapter<Container, volatile Object, P, R> {
 public:
  IteratorAdapter() = delete;
};

template <class Container, class P, class R>
class IteratorAdapter<Container, const volatile Object, P, R> {
 public:
  IteratorAdapter() = delete;
};

}  // namespace internal

}  // namespace liteproto
