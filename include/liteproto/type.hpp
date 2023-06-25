//
// Created by Youtao Guo on 2023/6/24.
//

#pragma

#include <algorithm>
#include <array>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "traits.hpp"
#include "utils.hpp"

using uint8_t = uint8_t;
using int8_t = int8_t;
using uint32_t = uint32_t;
using int32_t = int32_t;
using uint64_t = uint64_t;
using int64_t = int64_t;
using float_32_t = float;
using float_64_t = double;

namespace liteproto {

enum class [[maybe_unused]] Kind : int32_t {
  UINT8,
  INT8,
  UINT32,
  INT32,
  UINT64,
  INT64,
  FLOAT_32,
  FLOAT_64,
  BOOLEAN,

  // Thees are four data structures abstraction that supported by liteproto: list, map, string, array.

  // A list is a flexible-sized sequential container. The elements can be accessed by subscript operator (i.e.,
  // operator[]), but no random accessible required. A list also supports push_back and pop_back, and inserts a
  // new element to arbitrary position.
  // A list also supports size(), empty(), and clear().
  LIST,
  // A map is an key-value pairs collection. Each key must be unique in a map, and the value can be looked up via the
  // corresponding key. A map uses get() to look up and update() to modify. A map also supports size(), empty(), and
  // clear().
  MAP,
  // A string is almost the same as a list, except it doesn't be regarded as a container. A string is represented by a
  // single value, like "123" or "abc". Since the String is not a container, the underlying type cannot be further
  // inspected. A string supports all the operators that list supports.
  STRING,
  // An array is a fixed-sized sequential container. The elements can be accessed by subscript operator (i.e.,
  // operator[]). It doesn't support any other way to look up or modify. An array also supports size().
  ARRAY
};

template <class Tp>
struct TypeMeta {
  static uint64_t Id() {
    static uint64_t alloc_addr = 0;
    uint64_t* addr = &alloc_addr;
    return *reinterpret_cast<uint64_t*>(&addr);
  }
};

template <class Tp>
class ListIterator;

template <class Tp>
class List {
 public:
  virtual ~List() = default;

  virtual void push_back(Tp v) = 0;
  virtual void pop_back() = 0;

  virtual void insert(size_t pos, Tp v) = 0;
  virtual void erase(size_t pos) = 0;

  virtual Tp& operator[](size_t pos) = 0;
  virtual const Tp& operator[](size_t pos) const = 0;

  virtual size_t size() const = 0;
  virtual bool empty() const = 0;
  virtual void clear() = 0;

  using iterator = ListIterator<Tp>;
  using const_iterator = ListIterator<const Tp>;

  virtual iterator begin() = 0;
  virtual iterator end() = 0;
  virtual const_iterator begin() const = 0;
  virtual const_iterator end() const = 0;
};

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

template <class Tp, class Cond = void>
class ListAdapter;

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
  ListIterator(ListIterator&&) = default;
  ListIterator& operator=(ListIterator&&) = default;

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

template <template <class...> class C, class Tp, class... Os>
class ListAdapter<
    C<Tp, Os...>,
    std::enable_if_t<has_push_back_v<C, Tp> && has_pop_back_v<C<Tp>> && has_size_v<C<Tp>> && has_empty_v<C<Tp>> &&
                     has_clear_v<C<Tp>> && is_forward_iterable_v<C<Tp>> && has_insert_v<C, Tp> && has_erase_v<C<Tp>>>>
    : public List<Tp> {
  using base = List<Tp>;

 public:
  using container = C<Tp, Os...>;  // TODO recursive container
  using value_type = Tp;
  using iterator = typename base::iterator;
  using const_iterator = typename base::const_iterator;

 private:
  template <class V>
  class Iterator : public ListIteratorInterface<V> {
    using base = ListIteratorInterface<V>;
    using pointer = typename base::pointer;
    using reference = typename base::reference;
    using wrapped_iterator = typename container::iterator;

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
  explicit ListAdapter(container& c) : container_(c) {}

  void push_back(Tp v) override { container_.push_back(std::move(v)); }

  void pop_back() override { container_.pop_back(); }

  void insert(size_t pos, Tp v) override {
    using std::begin;
    auto it = begin(container_);
    std::advance(it, pos);
    container_.insert(it, std::move(v));
  }

  void erase(size_t pos) override {
    using std::begin;
    auto it = begin(container_);
    std::advance(it, pos);
    container_.erase(it);
  }

  Tp& operator[](size_t pos) override {
    using std::begin;
    auto it = begin(container_);
    std::advance(it, pos);
    return *it;
  }

  const Tp& operator[](size_t pos) const override {
    auto rm_const = const_cast<ListAdapter*>(this);
    return (*rm_const)[pos];
  }

  size_t size() const override { return container_.size(); }
  bool empty() const override { return container_.empty(); }
  void clear() override { container_.clear(); }

  iterator begin() override {
    using interface = ListIteratorInterface<value_type>;
    interface* ptr = new Iterator<value_type>(container_.begin());
    return iterator{std::unique_ptr<interface>{ptr}};
  }

  iterator end() override {
    using interface = ListIteratorInterface<value_type>;
    interface* ptr = new Iterator<value_type>(container_.end());
    return iterator{std::unique_ptr<interface>{ptr}};
  }

  const_iterator begin() const override {
    using interface = ListIteratorInterface<const value_type>;
    interface* ptr = new Iterator<const value_type>(container_.begin());
    return const_iterator{std::unique_ptr<interface>{ptr}};
  }

  const_iterator end() const override {
    using interface = ListIteratorInterface<const value_type>;
    interface* ptr = new Iterator<const value_type>(container_.end());
    return const_iterator{std::unique_ptr<interface>{ptr}};
  }

 private:
  container& container_;
};

}  // namespace liteproto