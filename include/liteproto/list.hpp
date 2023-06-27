//
// Created by Youtao Guo on 26/6/23.
//

#pragma once

#include <memory>

#include "liteproto/interface.hpp"
#include "liteproto/reflect/type.hpp"

namespace liteproto {

template <class Tp>
class ListIterator;

template <class Tp, ConstOption ConstOpt>
class List;

template <class Tp>
class List<Tp, ConstOption::NON_CONST> {
  template <class C, class>
  friend auto MakeList(C&& container);

 public:
  void push_back(Tp v) const { interface_.push_back(obj_, std::move(v)); }
  void pop_back() const { interface_.pop_back(obj_); }

  void insert(size_t pos, Tp v) const { interface_.insert(obj_, pos, std::move(v)); }
  void erase(size_t pos) const { interface_.erase(obj_, pos); }

  decltype(auto) operator[](size_t pos) const { return interface_.operator_subscript(obj_, pos); }

  void resize(size_t count) const { return interface_.resize(obj_, count); }
  void resize(size_t count, const Tp& v) { return interface_.resize_append(obj_, count, v); }

  size_t size() const { return interface_.size(obj_); }
  bool empty() const { return interface_.empty(obj_); }
  void clear() const { return interface_.clear(obj_); }

  using iterator = ListIterator<Tp>;
  using const_iterator = ListIterator<const Tp>;

  decltype(auto) begin() const { return interface_.begin(obj_); }
  decltype(auto) end() const { return interface_.end(obj_); }

 private:
  template <class Adapter>
  List(Adapter&& adapter, internal::ListInterface<Tp> interface)
      : obj_(std::forward<Adapter>(adapter)), interface_(interface) {
    static_assert(std::is_nothrow_move_constructible_v<List>);
  }

  internal::ListInterface<Tp> interface_;
  mutable std::any obj_;
};

template <class Tp>
class List<Tp, ConstOption::CONST> {
  template <class C, class>
  friend auto MakeList(C&& container);

 public:
  decltype(auto) operator[](size_t pos) const { return interface_.operator_subscript(obj_, pos); }

  size_t size() const { return interface_.size(obj_); }
  bool empty() const { return interface_.empty(obj_); }

  using iterator = ListIterator<const Tp>;

  decltype(auto) begin() const { return interface_.begin(obj_); }
  decltype(auto) end() const { return interface_.end(obj_); }

 private:
  template <class Adapter>
  List(Adapter&& adapter, internal::ListInterface<const Tp> interface)
      : obj_(std::forward<Adapter>(adapter)), interface_(interface) {}

  internal::ListInterface<const Tp> interface_;
  mutable std::any obj_;
};

template <class Tp, class Cond = void>
class ListAdapter;

template <class Tp>
class ListIteratorInterface {
 public:
  using pointer = Tp*;
  using reference = Tp&;

  virtual ~ListIteratorInterface() = default;
  virtual std::unique_ptr<ListIteratorInterface> Copy() const = 0;

  virtual reference operator*() const = 0;
  virtual pointer operator->() const = 0;
  virtual void Increment() = 0;
  virtual bool operator!=(const ListIteratorInterface&) const = 0;

  bool operator==(const ListIteratorInterface& rhs) const { return !(*this != rhs); }

  virtual uint64_t ContainerTypeId() const = 0;
};

template <class Tp>
class ListIterator {
  template <class Tp_, class Cond>
  friend class ListAdapter;

 public:
  using value_type = Tp;
  using difference_type = std::ptrdiff_t;
  using pointer = Tp*;
  using reference = Tp&;
  using iterator_category = std::forward_iterator_tag;

  using iterator_interface = ListIteratorInterface<Tp>;

  ListIterator(const ListIterator& rhs) : iter_(rhs.iter_->Copy()) {}
  ListIterator& operator=(const ListIterator& rhs) {
    iter_.reset(rhs.iter_->Copy());
    return *this;
  }
  ListIterator(ListIterator&&) noexcept = default;
  ListIterator& operator=(ListIterator&&) noexcept = default;

  reference operator*() { return **iter_; }
  pointer operator->() { return iter_->operator->(); }
  ListIterator& operator++() {
    iter_->Increment();
    return *this;
  }
  ListIterator operator++(int) {
    ListIterator old = *this;
    iter_->Increment();
    return old;
  }
  bool operator==(const ListIterator& rhs) const { return !(*this != rhs); }
  bool operator!=(const ListIterator& rhs) const { return *iter_ != *rhs.iter_; }

 private:
  explicit ListIterator(std::unique_ptr<iterator_interface> iter) : iter_(std::move(iter)) {
    static_assert(std::is_copy_constructible<ListIterator>::value);
    static_assert(std::is_copy_assignable<ListIterator>::value);
    static_assert(std::is_swappable<ListIterator>::value);
  }

  std::unique_ptr<iterator_interface> iter_;
};

template <class Tp>
class ListAdapter<Tp, std::enable_if_t<IsListV<Tp>>> {
  static_assert(!std::is_reference_v<Tp>);
  using list_traits = ListTraits<Tp>;

 public:
  using container_type = typename list_traits::container_type;  // TODO recursive container_type
  using value_type = typename list_traits::value_type;
  using iterator = typename List<value_type, static_cast<ConstOption>(std::is_const_v<container_type>)>::iterator;
  using iterator_value_type = std::conditional_t<std::is_const_v<container_type>, const value_type, value_type>;

 private:
  template <class V>
  class Iterator : public ListIteratorInterface<V> {
    using base = ListIteratorInterface<V>;
    using pointer = typename base::pointer;
    using reference = typename base::reference;
    using wrapped_iterator =
        std::conditional_t<std::is_const_v<container_type>, typename container_type::const_iterator,
                           typename container_type::iterator>;

   public:
    explicit Iterator(const wrapped_iterator& it) : it_(it) {
      static_assert(std::is_same_v<value_type, V> || std::is_same_v<const value_type, V>);
      static_assert(std::is_copy_constructible<Iterator>::value);
      static_assert(std::is_copy_assignable<Iterator>::value);
      static_assert(std::is_swappable<Iterator>::value);
    }

    std::unique_ptr<base> Copy() const override { return std::unique_ptr<base>{new Iterator(*this)}; }

    reference operator*() const override { return *it_; }
    pointer operator->() const override { return it_.operator->(); }
    void Increment() override { ++it_; }
    bool operator!=(const base& rhs) const override {
      if (ContainerTypeId() != rhs.ContainerTypeId()) {
        return false;
      }
      auto& rhs_same_type = static_cast<const Iterator&>(rhs);
      return it_ != rhs_same_type.it_;
    }

    uint64_t ContainerTypeId() const override { return TypeMeta<V>::Id(); }

   private:
    wrapped_iterator it_;
  };

 public:
  explicit ListAdapter(container_type* c) : container_(c) {
    static_assert(std::is_copy_constructible_v<ListAdapter>);
    static_assert(std::is_copy_assignable_v<ListAdapter>);
    static_assert(std::is_nothrow_move_constructible_v<ListAdapter>);
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

  void insert(size_t pos, value_type v) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      using std::begin;
      auto it = begin(*container_);
      std::advance(it, pos);
      container_->insert(it, std::move(v));
    }
  }

  void erase(size_t pos) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!std::is_const_v<container_type>) {
      using std::begin;
      auto it = begin(*container_);
      std::advance(it, pos);
      container_->erase(it);
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
    using interface = ListIteratorInterface<iterator_value_type>;
    interface* ptr = new Iterator<iterator_value_type>(container_->begin());
    return iterator{std::unique_ptr<interface>{ptr}};
  }

  iterator end() const noexcept {
    using interface = ListIteratorInterface<iterator_value_type>;
    interface* ptr = new Iterator<iterator_value_type>(container_->end());
    return iterator{std::unique_ptr<interface>{ptr}};
  }

 private:
  container_type* container_;
};

template <class C, class = ListAdapter<std::remove_reference_t<C>>>
auto MakeList(C&& container) {
  using ref_removed = std::remove_reference_t<C>;
  ListAdapter<ref_removed> adapter{&container};
  using value_type = typename ListAdapter<ref_removed>::value_type;
  constexpr auto const_opt = static_cast<ConstOption>(std::is_const_v<ref_removed>);
  List<value_type, const_opt> list{std::move(adapter), internal::MakeListInterface<decltype(adapter)>()};
  return list;
}

}  // namespace liteproto