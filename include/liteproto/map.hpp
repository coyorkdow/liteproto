//
// Created by Youtao Guo on 2023/7/3.
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
class MapAdapter;
}

template <class C, class = internal::MapAdapter<C>>
auto AsMap(C* container) noexcept;

template <class K, class V>
class Map<K, V, ConstOption::NON_CONST> {
  template <class C, class>
  friend auto AsList(C* container) noexcept;

  friend class Map<K, V, ConstOption::CONST>;

  //  using traits = InterfaceTraits<Tp, ConstOption::NON_CONST>;
  //  using const_traits = InterfaceTraits<Tp, ConstOption::CONST>;

 public:
};

namespace internal {

template <class Tp>
class MapAdapter<Tp, std::enable_if_t<IsMapV<Tp>>> {
  static_assert(!std::is_reference_v<Tp>);
  using map_traits = MapTraits<Tp>;

 public:
  using container_type = typename map_traits::container_type;
  using underlying_key_type = typename map_traits::key_type;
  using underlying_mapped_type = typename map_traits::mapped_type;
  using underlying_value_type = typename map_traits::value_type;
  static inline constexpr bool is_const = std::is_const_v<container_type>;

 private:
  using key_traits = InterfaceTraits<typename ProxyType<underlying_key_type>::type, ConstOption::CONST>;
  using mapped_traits = InterfaceTraits<typename ProxyType<underlying_mapped_type>::type, static_cast<ConstOption>(is_const)>;
  using const_mapped_traits = InterfaceTraits<typename ProxyType<underlying_mapped_type>::type, ConstOption::CONST>;
  using value_traits = InterfaceTraits<typename ProxyType<underlying_value_type>::type, static_cast<ConstOption>(is_const)>;
  using const_value_traits = InterfaceTraits<typename ProxyType<underlying_value_type>::type, ConstOption::CONST>;

 public:
  using key_type = typename key_traits::value_type;
  using mapped_type = typename map_traits::value_type;
  using value_type = typename value_traits::value_type;
  using pointer = DummyPointer;
  using reference = value_type;
  using const_value_type = typename const_value_traits::value_type;
  using const_pointer = DummyPointer;
  using const_reference = const_value_type;
  using iterator = Iterator<value_type, pointer, reference, std::forward_iterator_tag>;

  using iterator_adapter = IteratorAdapter<container_type, value_type, pointer, reference, PairWrapper<reference>>;
  using const_adapter = MapAdapter<const Tp, void>;

  explicit MapAdapter(container_type* c) noexcept : container_(c) {
    static_assert(std::is_copy_constructible_v<MapAdapter>);
    static_assert(std::is_copy_assignable_v<MapAdapter>);
    static_assert(std::is_nothrow_move_constructible_v<MapAdapter>);
    static_assert(std::is_trivially_copyable_v<MapAdapter>);
  }

  size_t size() const noexcept { return container_->size(); }
  bool empty() const noexcept { return container_->empty(); }
  void clear() const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!is_const) {
      container_->clear();
    }
  }

  iterator begin() const noexcept {
    iterator_adapter it_adapter{container_->begin()};
    return internal::MakeIterator<std::forward_iterator_tag>(std::move(it_adapter), GetIteratorInterface<decltype(it_adapter)>());
  }

  iterator end() const noexcept {
    iterator_adapter it_adapter{container_->end()};
    return internal::MakeIterator<std::forward_iterator_tag>(std::move(it_adapter), GetIteratorInterface<decltype(it_adapter)>());
  }

  template <class Value>
  std::pair<iterator, bool> insert(Value&& v) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (is_const) {
      return std::make_pair(end(), false);
    } else {
      if constexpr (IsProxyTypeV<value_type>) {
        auto real_v = RestoreFromProxy<underlying_value_type>(std::forward<Value>(v));
        if (real_v.has_value()) {
          auto [iter, ok] = container_->insert(std::move(*real_v));
          return std::make_pair(MakeIterator(iter), ok);
        }
        return std::make_pair(end(), false);
      } else {
        auto [iter, ok] = container_->insert(std::forward<Value>(v));
        return std::make_pair(MakeIterator(iter), ok);
      }
    }
  }

  iterator find(const key_type& key) const {
    auto iter = container_->end();
    if constexpr (IsObjectV<key_type>) {
      auto k_ptr = ObjectCast<underlying_key_type>(key);
      if (k_ptr == nullptr) {
        return end();
      }
      iter = container_->find(*k_ptr);
    } else if constexpr (IsNumberV<key_type>) {
      if (key.IsSignedInteger()) {
        iter = container_->find(key.AsInt64());
      } else if (key.IsUnsigned()) {
        iter = container_->find(key.AsUInt64());
      } else {
        iter = container_->find(key.AsFloat64());
      }
    } else {
      iter = container_->find(key);
    }
    return MakeIterator(std::move(iter));
  }

  iterator erase(iterator pos) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!is_const) {
      auto& any_iter = internal::GetIteratorAdapter(pos);
      auto rhs_it = std::any_cast<iterator_adapter>(&any_iter);
      if (rhs_it != nullptr) {
        rhs_it->EraseMyself(container_);
        return pos;
      }
    }
    return end();
  }

  size_t erase(const key_type& key) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (std::is_const_v<container_type>) {
      return 0;
    } else {
      if constexpr (IsObjectV<key_type>) {
        auto k_ptr = ObjectCast<underlying_key_type>(key);
        if (k_ptr == nullptr) {
          return 0;
        }
        return container_->erase(*k_ptr);
      } else if constexpr (IsNumberV<key_type>) {
        if (key.IsSignedInteger()) {
          return container_->erase(key.AsInt64());
        } else if (key.IsUnsigned()) {
          return container_->erase(key.AsUInt64());
        } else {
          return container_->erase(key.AsFloat64());
        }
      } else {
        return container_->erase(key);
      }
    }
  }

  [[nodiscard]] std::any ToConst() const noexcept { return const_adapter{container_}; }

 private:
  template <class It>
  [[nodiscard]] iterator MakeIterator(It&& iterator) const noexcept {
    iterator_adapter it_adapter{std::forward<It>(iterator)};
    return internal::MakeIterator<std::forward_iterator_tag>(std::move(it_adapter), GetIteratorInterface<decltype(it_adapter)>());
  }

  container_type* container_;
};

}  // namespace internal
}  // namespace liteproto