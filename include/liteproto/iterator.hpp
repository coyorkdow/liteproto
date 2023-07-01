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

template <class Tp>
class Iterator;

namespace internal {

template <class Tp, class = void>
struct ReferenceType {
  using type = Tp&;
};

template <>
struct ReferenceType<Object> {
  using type = Object;
};

template <>
struct ReferenceType<Number> {
  using type = NumberReference<ConstOption::NON_CONST>;
};

template <>
struct ReferenceType<const Number> {
  using type = NumberReference<ConstOption::CONST>;
};

template <class Tp>
inline constexpr bool is_proxy_type_v =
    std::is_same_v<Tp, Object> || std::is_same_v<Tp, Number> || std::is_same_v<Tp, const Number>;

template <class Container, class Tp>
class IteratorAdapter;

template <class Tp>
struct IteratorInterface {
 public:
  using pointer = Tp*;
  using reference_or_value = typename ReferenceType<Tp>::type;

  using indirection_t = reference_or_value(const std::any&) noexcept;
  using member_of_object_t = pointer(const std::any&) noexcept;
  using increment_t = void(std::any&);
  using decrement_t = void(std::any&);
  using noteq_t = bool(const std::any&, const Iterator<Tp>&) noexcept;

  using adapter_type_id_t = uint64_t(const std::any&) noexcept;
  indirection_t* indirection;
  member_of_object_t* member_of_object;
  increment_t* increment;
  decrement_t* decrement;
  noteq_t* noteq;
  adapter_type_id_t* adapter_type_id;
};

template <class IteratorAdapter, class Tp>
Iterator<Tp> MakeIterator(IteratorAdapter&& it, const internal::IteratorInterface<Tp>& inter);

template <class Tp_>
std::any& GetIteratorAdapter(Iterator<Tp_>&) noexcept;

template <class Tp_>
const std::any& GetIteratorAdapter(const Iterator<Tp_>&) noexcept;

}  // namespace internal

template <class Tp>
class IteratorBase {
  using iterator = Iterator<Tp>;

 public:
  using value_type = Tp;
  using difference_type = std::ptrdiff_t;
  using pointer = typename internal::IteratorInterface<Tp>::pointer;
  using reference = typename internal::IteratorInterface<Tp>::reference_or_value;
  using iterator_category = std::bidirectional_iterator_tag;

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
  IteratorBase(ItAdapter&& it, const internal::IteratorInterface<Tp>& inter)
      : it_(std::forward<ItAdapter>(it)), interface_(&inter) {
    static_assert(std::is_copy_constructible<IteratorBase>::value);
    static_assert(std::is_copy_assignable<IteratorBase>::value);
    static_assert(std::is_swappable<IteratorBase>::value);
  }

  std::any it_;
  const internal::IteratorInterface<Tp>* interface_;
};

template <class Tp>
class Iterator : public IteratorBase<Tp> {
  template <class IteratorAdapter, class Tp_>
  friend Iterator<Tp_> internal::MakeIterator(IteratorAdapter&& it, const internal::IteratorInterface<Tp_>& inter);

  template <class Tp_>
  friend std::any& internal::GetIteratorAdapter(Iterator<Tp_>&) noexcept;

  template <class Tp_>
  friend const std::any& internal::GetIteratorAdapter(const Iterator<Tp_>&) noexcept;

  template <class Container, class Tp_>
  friend class internal::IteratorAdapter;

  friend class IteratorBase<Tp>;
  using base = IteratorBase<Tp>;

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
  Iterator(ItAdapter&& it, const internal::IteratorInterface<Tp>& inter)
      : IteratorBase<Tp>(std::forward<ItAdapter>(it), inter) {}

  explicit Iterator(const base& iterator) : IteratorBase<Tp>(iterator) {}
  explicit Iterator(base&& iterator) noexcept : IteratorBase<Tp>(std::move(iterator)) {}
};

template <>
class Iterator<Object> : public IteratorBase<Object> {
  template <class IteratorAdapter, class Tp_>
  friend Iterator<Tp_> internal::MakeIterator(IteratorAdapter&& it, const internal::IteratorInterface<Tp_>& inter);

  template <class Tp_>
  friend std::any& internal::GetIteratorAdapter(Iterator<Tp_>&) noexcept;

  template <class Tp_>
  friend const std::any& internal::GetIteratorAdapter(const Iterator<Tp_>&) noexcept;

  template <class Container, class Tp_>
  friend class internal::IteratorAdapter;

  friend class IteratorBase<Object>;
  using base = IteratorBase<Object>;

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
  Iterator(ItAdapter&& it, const internal::IteratorInterface<Object>& inter)
      : IteratorBase<Object>(std::forward<ItAdapter>(it), inter) {}

  explicit Iterator(const base& iterator) : IteratorBase<Object>(iterator) {}
  explicit Iterator(base&& iterator) noexcept : IteratorBase<Object>(std::move(iterator)) {}
};

template <>
class Iterator<Number> : public IteratorBase<Number> {
  template <class IteratorAdapter, class Tp_>
  friend Iterator<Tp_> internal::MakeIterator(IteratorAdapter&& it, const internal::IteratorInterface<Tp_>& inter);

  template <class Tp_>
  friend std::any& internal::GetIteratorAdapter(Iterator<Tp_>&) noexcept;

  template <class Tp_>
  friend const std::any& internal::GetIteratorAdapter(const Iterator<Tp_>&) noexcept;

  template <class Container, class Tp_>
  friend class internal::IteratorAdapter;

  friend class IteratorBase<Number>;
  using base = IteratorBase<Number>;

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
  Iterator(ItAdapter&& it, const internal::IteratorInterface<Number>& inter)
      : IteratorBase<Number>(std::forward<ItAdapter>(it), inter) {}

  explicit Iterator(const base& iterator) : IteratorBase<Number>(iterator) {}
  explicit Iterator(base&& iterator) noexcept : IteratorBase<Number>(std::move(iterator)) {}
};

namespace internal {

template <class IteratorAdapter, class Tp>
Iterator<Tp> MakeIterator(IteratorAdapter&& it, const IteratorInterface<Tp>& inter) {
  return Iterator<Tp>(std::forward<IteratorAdapter>(it), inter);
}

template <class Tp_>
std::any& GetIteratorAdapter(Iterator<Tp_>& iterator) noexcept {
  return iterator.it_;
}

template <class Tp_>
const std::any& GetIteratorAdapter(const Iterator<Tp_>& iterator) noexcept {
  return iterator.it_;
}

template <class It, class Tp>
struct IteratorInterfaceImpl {
  using base = IteratorInterface<Tp>;
  using pointer = typename base::pointer;
  using reference_or_value = typename base::reference_or_value;

  static reference_or_value Indirection(const std::any& obj) noexcept {
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
const IteratorInterface<typename It::value_type>& MakeIteratorInterface() noexcept {
  using value_type = typename It::value_type;
  using impl = IteratorInterfaceImpl<It, value_type>;
  static const IteratorInterface<value_type> inter = [&] {
    IteratorInterface<value_type> interface {};
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

template <class Container, class Tp>
class IteratorAdapter {
 public:
  static_assert(!std::is_reference_v<Tp>);
  using value_type = Tp;

 private:
  using pointer = typename IteratorInterface<value_type>::pointer;
  using reference_or_value = typename IteratorInterface<value_type>::reference_or_value;

  // Iterator<Container, Object> is special, it is used to iterate an interface of the indirect type.
  // What should an adapter of the Object iterator do?
  // The adapter still iterates the underlying container, but returns Object instead of the real value_type.
  // The Object, as well as all other Interface types, are essentially some sort of fat pointers.
  // We don't store any additional data in the adapter. Instead, the Object returned by adapter is created when the
  // corresponding method is called. Therefore, the return type of indirection operator is value type, not
  // reference_or_value type.
  static_assert(is_proxy_type_v<value_type> || std::is_reference_v<reference_or_value>,
                "IteratorAdapter uses reference_or_value for all types except Object or Number");
  using container_type = Container;
  using wrapped_iterator = std::conditional_t<std::is_const_v<container_type>, typename container_type::const_iterator,
                                              typename container_type::iterator>;

 public:
  explicit IteratorAdapter(const wrapped_iterator& it) noexcept : it_(it) {
    static_assert(is_proxy_type_v<value_type> || std::is_same_v<value_type, typename container_type::value_type> ||
                  std::is_same_v<value_type, const typename container_type::value_type>);
    static_assert(std::is_copy_constructible<IteratorAdapter>::value);
    static_assert(std::is_copy_assignable<IteratorAdapter>::value);
    static_assert(std::is_swappable<IteratorAdapter>::value);
  }

  reference_or_value operator*() const {
    if constexpr (std::is_same_v<value_type, Object>) {
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

  bool operator!=(const Iterator<value_type>& rhs) const {
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

// IteratorAdapter isn't suitable for the Object with CV-qualified, disable them.
template <class Container>
class IteratorAdapter<Container, const Object> {
 public:
  IteratorAdapter() = delete;
};

template <class Container>
class IteratorAdapter<Container, volatile Object> {
 public:
  IteratorAdapter() = delete;
};

template <class Container>
class IteratorAdapter<Container, const volatile Object> {
 public:
  IteratorAdapter() = delete;
};

}  // namespace internal

}  // namespace liteproto
