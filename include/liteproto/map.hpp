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
  using temp_proxied = std::pair<typename ProxyType<typename underlying_value_type::first_type>::type,
                                 typename ProxyType<typename underlying_value_type::second_type>::type>;
  using traits = InterfaceTraits<temp_proxied, static_cast<ConstOption>(is_const)>;
  using const_traits = InterfaceTraits<typename ProxyType<underlying_value_type>::type, ConstOption::CONST>;

 public:
  using key_type = typename traits::key_type;
  using mapped_type = typename traits::mapped_type;
  using value_type = typename traits::value_type;
  using pointer = typename traits::pointer;
  using reference = typename traits::reference;
  using const_value_type = typename const_traits::value_type;
  using const_pointer = typename const_traits::pointer;
  using const_reference = typename const_traits::reference;
  using iterator = Iterator<value_type, pointer, reference, std::forward_iterator_tag>;

  using iterator_adapter = IteratorAdapter<container_type, value_type, pointer, reference>;
  using const_adapter = ListAdapter<const Tp, void>;

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
    return internal::MakeIterator<std::forward_iterator_tag>(std::move(it_adapter), GetIteratorInterface<decltype(it_adapter)>());
  }

  iterator end() const noexcept {
    iterator_adapter it_adapter{container_->end()};
    return internal::MakeIterator<std::forward_iterator_tag>(std::move(it_adapter), GetIteratorInterface<decltype(it_adapter)>());
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
  container_type* container_;
};

}  // namespace internal
}  // namespace liteproto