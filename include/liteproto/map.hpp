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
  friend auto AsMap(C* container) noexcept;

  friend class Map<K, V, ConstOption::CONST>;

  using key_traits = InterfaceTraits<K, ConstOption::NON_CONST>;
  using const_key_traits = InterfaceTraits<K, ConstOption::CONST>;
  using mapped_traits = InterfaceTraits<V, ConstOption::NON_CONST>;
  using const_mapped_traits = InterfaceTraits<V, ConstOption::CONST>;

 public:
  using key_type = typename key_traits::value_type;
  using mapped_type = typename mapped_traits::value_type;
  using value_type = std::pair<typename const_key_traits::value_type, mapped_type>;
  using pointer = internal::DummyPointer;
  using reference = value_type;
  using const_value_type = std::pair<typename const_key_traits::value_type, typename const_mapped_traits::value_type>;
  using const_pointer = internal::DummyPointer;
  using const_reference = const_value_type;
  using iterator = Iterator<value_type, pointer, reference, std::forward_iterator_tag>;
  using interface = internal::MapInterface<value_type, pointer, reference, const_value_type, const_pointer, const_reference>;

  Map(const Map& rhs) = default;
  Map& operator=(const Map&) = default;
  Map(Map&&) noexcept = default;
  Map& operator=(Map&&) noexcept = default;

  std::pair<iterator, bool> insert(const value_type& value) noexcept { return interface_->insert(obj_, value); }

 private:
  template <class Adapter>
  Map(Adapter&& adapter, const interface& interface) noexcept : obj_(std::forward<Adapter>(adapter)), interface_(&interface) {
    //    static_assert(IsMapV<Map>, "Why the Map<Tp, ConstOption::NON_CONST> itself is not a Map?");
    static_assert(std::is_nothrow_move_constructible_v<Map>);
  }

  std::any obj_;
  const interface* interface_;
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
  using key_traits = InterfaceTraits<typename ProxyType<underlying_key_type>::type, ConstOption::NON_CONST>;
  using const_key_traits = InterfaceTraits<typename ProxyType<underlying_key_type>::type, ConstOption::CONST>;
  using mapped_traits = InterfaceTraits<typename ProxyType<underlying_mapped_type>::type, static_cast<ConstOption>(is_const)>;
  using const_mapped_traits = InterfaceTraits<typename ProxyType<underlying_mapped_type>::type, ConstOption::CONST>;

 public:
  using key_type = typename key_traits::value_type;
  using mapped_type = typename mapped_traits::value_type;
  using value_type = std::pair<typename const_key_traits::value_type, mapped_type>;
  using pointer = DummyPointer;
  using reference = value_type;
  using const_value_type = std::pair<typename const_key_traits::value_type, typename const_mapped_traits::value_type>;
  using const_pointer = DummyPointer;
  using const_reference = const_value_type;
  using iterator = Iterator<value_type, pointer, reference, std::forward_iterator_tag>;

  using iterator_adapter =
      IteratorAdapter<container_type, value_type, pointer, reference, std::forward_iterator_tag, PairWrapper<reference>>;
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

  template <class Value, class = std::enable_if_t<IsPairV<std::remove_reference_t<Value>>>>
  std::pair<iterator, bool> insert(Value&& v) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (is_const) {
      return std::make_pair(end(), false);
    } else {
      if constexpr (IsProxyTypeV<value_type>) {
        auto value = RestoreFromProxy<underlying_mapped_type>(std::forward<Value>(v).second);
        if (value.has_value()) {
          if (auto key = RestoreFromProxy<underlying_key_type>(std::forward<Value>(v).first); key.has_value()) {
            auto [iter, ok] = container_->insert(std::make_pair(std::move(key), std::move(value)));
            return std::make_pair(MakeIterator(iter), ok);
          } else if (auto const_key = RestoreFromProxy<const underlying_key_type>(std::forward<Value>(v).first); const_key.has_value()) {
            auto [iter, ok] = container_->insert(std::make_pair(const_key, std::move(value)));
            return std::make_pair(MakeIterator(iter), ok);
          }
          return std::make_pair(end(), false);
        }
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

template <class C, class>
auto AsMap(C* container) noexcept {
  internal::MapAdapter<C> adapter{container};

  static_assert(std::is_trivially_copyable_v<decltype(adapter)>);

  using key_type = typename internal::MapAdapter<C>::key_type;
  using mapped_type = typename internal::MapAdapter<C>::mapped_type;
  constexpr auto const_opt = static_cast<ConstOption>(std::is_const_v<C>);
  Map<key_type, mapped_type, const_opt> map{adapter, internal::GetMapInterface<decltype(adapter)>()};
  return map;
}

}  // namespace liteproto