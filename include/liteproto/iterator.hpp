//
// Created by Youtao Guo on 2023/6/27.
//

#pragma once

#include <any>
#include <type_traits>

#include "liteproto/reflect/number.hpp"
#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/type.hpp"

namespace liteproto {

namespace internal {
struct Dummy;
using DummyPointer = Dummy*;
}  // namespace internal

template <class Tp, class Pointer, class Reference>
class Iterator;

template <class Tp>
using DefaultIterator = Iterator<Tp, Tp*, Tp&>;

using ObjectIterator = Iterator<Object, internal::DummyPointer, Object>;
using NumberIterator = Iterator<Number, internal::DummyPointer, NumberReference<ConstOption::NON_CONST>>;
using ConstNumberIterator = Iterator<Number, internal::DummyPointer, NumberReference<ConstOption::CONST>>;

template <class Tp, ConstOption Opt>
struct InterfaceTraits {
  using value_type = std::conditional_t<Opt == ConstOption::CONST, const Tp, Tp>;
  using pointer = value_type*;
  using reference = value_type&;
};

template <ConstOption Opt>
struct InterfaceTraits<Object, Opt> {
  using value_type = Object;
  using pointer = internal::DummyPointer;
  using reference = Object;
};

template <ConstOption Opt>
struct InterfaceTraits<Number, Opt> {
  using value_type = Number;
  using pointer = internal::DummyPointer;
  using reference = NumberReference<Opt>;
};

template <ConstOption Opt>
struct InterfaceTraits<const Number, Opt> {
  using value_type = Number;
  using pointer = internal::DummyPointer;
  using reference = NumberReference<Opt>;
};

namespace internal {

template <class Tp>
inline constexpr bool is_proxy_type_v =
    std::is_same_v<Tp, Object> || std::is_same_v<Tp, Number> || std::is_same_v<Tp, const Number>;

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

template <class IteratorAdapter, class Tp, class Pointer, class Reference>
Iterator<Tp, Pointer, Reference> MakeIterator(IteratorAdapter&& it,
                                              const internal::IteratorInterface<Tp, Pointer, Reference>& inter);

template <class Tp, class Pointer, class Reference>
std::any& GetIteratorAdapter(Iterator<Tp, Pointer, Reference>&) noexcept;

template <class Tp, class Pointer, class Reference>
const std::any& GetIteratorAdapter(const Iterator<Tp, Pointer, Reference>&) noexcept;

}  // namespace internal

template <class Tp, class Pointer, class Reference>
class IteratorBase {
  using iterator = Iterator<Tp, Pointer, Reference>;

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

  iterator& operator--() {
    interface_->decrement(it_);
    return static_cast<iterator&>(*this);
  }
  iterator operator--(int) {
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
  IteratorBase(ItAdapter&& it, const interface& inter) : it_(std::forward<ItAdapter>(it)), interface_(&inter) {
    static_assert(std::is_copy_constructible<IteratorBase>::value);
    static_assert(std::is_copy_assignable<IteratorBase>::value);
    static_assert(std::is_swappable<IteratorBase>::value);
  }

  std::any it_;
  const interface* interface_;
};

template <class Tp, class Pointer, class Reference>
class Iterator : public IteratorBase<Tp, Pointer, Reference> {
  template <class IteratorAdapter, class T, class P, class R>
  friend Iterator<T, P, R> internal::MakeIterator(IteratorAdapter&& it,
                                                  const internal::IteratorInterface<T, P, R>& inter);

  template <class T, class P, class R>
  friend std::any& internal::GetIteratorAdapter(Iterator<T, P, R>&) noexcept;

  template <class T, class P, class R>
  friend const std::any& internal::GetIteratorAdapter(const Iterator<T, P, R>&) noexcept;

  template <class Container, class T, class P, class R>
  friend class internal::IteratorAdapter;

  friend class IteratorBase<Tp, Pointer, Reference>;
  using base = IteratorBase<Tp, Pointer, Reference>;

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
  Iterator(ItAdapter&& it, const typename base::interface& inter) : base(std::forward<ItAdapter>(it), inter) {}

  explicit Iterator(const base& iterator) : base(iterator) {}
  explicit Iterator(base&& iterator) noexcept : base(std::move(iterator)) {}
};

template <class Tp, class Reference>
class Iterator<Tp, internal::DummyPointer, Reference> : public IteratorBase<Tp, internal::DummyPointer, Reference> {
  template <class IteratorAdapter, class T, class P, class R>
  friend Iterator<T, P, R> internal::MakeIterator(IteratorAdapter&& it,
                                                  const internal::IteratorInterface<T, P, R>& inter);

  template <class T, class P, class R>
  friend std::any& internal::GetIteratorAdapter(Iterator<T, P, R>&) noexcept;

  template <class T, class P, class R>
  friend const std::any& internal::GetIteratorAdapter(const Iterator<T, P, R>&) noexcept;

  template <class Container, class T, class P, class R>
  friend class internal::IteratorAdapter;

  friend class IteratorBase<Tp, internal::DummyPointer, Reference>;
  using base = IteratorBase<Tp, internal::DummyPointer, Reference>;

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
  Iterator(ItAdapter&& it, const typename base::interface& inter) : base(std::forward<ItAdapter>(it), inter) {}

  explicit Iterator(const base& iterator) : base(iterator) {}
  explicit Iterator(base&& iterator) noexcept : base(std::move(iterator)) {}
};

namespace internal {

template <class IteratorAdapter, class Tp, class Pointer, class Reference>
Iterator<Tp, Pointer, Reference> MakeIterator(IteratorAdapter&& it,
                                              const IteratorInterface<Tp, Pointer, Reference>& inter) {
  return Iterator<Tp, Pointer, Reference>(std::forward<IteratorAdapter>(it), inter);
}

template <class Tp, class Pointer, class Reference>
std::any& GetIteratorAdapter(Iterator<Tp, Pointer, Reference>& iterator) noexcept {
  return iterator.it_;
}

template <class Tp, class Pointer, class Reference>
const std::any& GetIteratorAdapter(const Iterator<Tp, Pointer, Reference>& iterator) noexcept {
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
const IteratorInterface<typename It::value_type, typename It::pointer, typename It::reference>&
GetIteratorInterface() noexcept {
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
  static_assert(is_proxy_type_v<value_type> || std::is_reference_v<reference>,
                "IteratorAdapter uses reference_or_value for all types except Object or Number");
  using container_type = Container;
  using wrapped_iterator = std::conditional_t<std::is_const_v<container_type>, typename container_type::const_iterator,
                                              typename container_type::iterator>;

  explicit IteratorAdapter(const wrapped_iterator& it) noexcept : it_(it) {
    static_assert(is_proxy_type_v<value_type> || std::is_same_v<value_type, typename container_type::value_type> ||
                  std::is_same_v<value_type, const typename container_type::value_type>);
    static_assert(std::is_copy_constructible<IteratorAdapter>::value);
    static_assert(std::is_copy_assignable<IteratorAdapter>::value);
    static_assert(std::is_swappable<IteratorAdapter>::value);
  }

  reference operator*() const {
    if constexpr (IsObjectV<value_type>) {
      return GetReflection(&(*it_));
    } else {
      // number can implicitly convert to NumberReference
      return *it_;
    }
  }

  pointer operator->() const {
    if constexpr (!is_proxy_type_v<value_type>) {
      return it_.operator->();
    } else {
      // If the value_type is Object or Number, do nothing. And it's assured that this method will never be called.
      return nullptr;
    }
  }

  void Increment() { ++it_; }
  void Decrement() { --it_; }

  bool operator!=(const Iterator<value_type, pointer, reference>& rhs) const {
    if (AdapterTypeId() != rhs.AdapterTypeId()) {
      return true;
    }
    auto any_iter = &rhs.it_;
    auto rhs_it = std::any_cast<IteratorAdapter>(any_iter);
    return it_ != rhs_it->it_;
  }

  [[nodiscard]] uint64_t AdapterTypeId() const { return TypeMeta<IteratorAdapter>::Id(); }

  template <class V>
  IteratorAdapter& InsertMyself(container_type* container, V&& v) {
    if constexpr (!std::is_const_v<container_type>) {
      it_ = container->insert(it_, std::forward<V>(v));
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
