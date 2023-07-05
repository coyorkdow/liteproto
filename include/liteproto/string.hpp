//
// Created by Youtao Guo on 2023/7/1.
//

#pragma once

#include <cstring>

#include "liteproto/interface.hpp"
#include "liteproto/iterator.hpp"
#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/type.hpp"
#include "liteproto/traits/traits.hpp"

namespace liteproto {

namespace internal {
template <class Tp, class = void>
class StringAdapter;
}

template <class C, class = internal::StringAdapter<C>>
auto AsString(C* container) noexcept;

template <>
class String<ConstOption::NON_CONST> : public List<char, ConstOption::NON_CONST> {
  template <class C, class>
  friend auto AsString(C* container) noexcept;

  friend class String<ConstOption::CONST>;

 public:
  using list_base = List<char, ConstOption::NON_CONST>;
  using string_interface = internal::StringInterface<char>;
  using interface = typename list_base::interface;

  char* data() const { return string_interface_->data(obj_); }
  const char* c_str() const noexcept { return string_interface_->c_str(obj_); }
  String append(const char* cstr) const {
    string_interface_->append(obj_, cstr, std::strlen(cstr));
    return *this;
  }
  String append(const char* cstr, size_t n) const {
    string_interface_->append(obj_, cstr, n);
    return *this;
  }

  String(const String& rhs) = default;
  String& operator=(const String&) = default;
  String(String&&) noexcept = default;
  String& operator=(String&&) noexcept = default;

 protected:
  template <class Adapter>
  String(Adapter&& adapter, const interface& list_interface, const string_interface& string_interface) noexcept
      : list_base(std::forward<Adapter>(adapter), list_interface), string_interface_(&string_interface) {
    static_assert(IsStringV<String>, "Why is the String<Tp, ConstOption::NON_CONST> itself is not a String?");
    static_assert(std::is_nothrow_move_constructible_v<String>);
  }

  const string_interface* string_interface_;
};

template <>
class String<ConstOption::CONST> : public List<char, ConstOption::CONST> {
  template <class C, class>
  friend auto AsString(C* container) noexcept;

 public:
  using list_base = List<char, ConstOption::CONST>;
  using string_interface = internal::StringInterface<const char>;
  using interface = typename list_base::interface;

  const char* data() const { return string_interface_->data(list_base::obj_); }
  const char* c_str() const noexcept { return string_interface_->c_str(list_base::obj_); }

  String(const String& rhs) = default;
  String& operator=(const String&) = default;
  String(String&&) noexcept = default;
  String& operator=(String&&) noexcept = default;

  String(const String<ConstOption::NON_CONST>& rhs) noexcept
      : list_base(rhs.string_interface_->to_const(rhs.obj_), rhs.interface_->const_interface()),
        string_interface_(&rhs.string_interface_->const_interface()) {}
  String(String<ConstOption::NON_CONST>&& rhs) noexcept
      : list_base(rhs.string_interface_->to_const(rhs.obj_), rhs.interface_->const_interface()),
        string_interface_(&rhs.string_interface_->const_interface()) {}
  String& operator=(const String<ConstOption::NON_CONST>& rhs) noexcept {
    this->obj_ = rhs.string_interface_->to_const(rhs.obj_);
    this->interface_ = &rhs.interface_->const_interface();
    this->string_interface_ = &rhs.string_interface_->const_interface();
    return *this;
  }
  String& operator=(String<ConstOption::NON_CONST>&& rhs) noexcept {
    static_assert(std::is_lvalue_reference_v<decltype(rhs)&>);
    *this = static_cast<decltype(rhs)&>(rhs);
    return *this;
  }

 protected:
  template <class Adapter>
  String(Adapter&& adapter, const interface& list_interface, const string_interface& string_interface) noexcept
      : list_base(std::forward<Adapter>(adapter), list_interface), string_interface_(&string_interface) {
    static_assert(std::is_nothrow_move_constructible_v<String>);
  }

  const string_interface* string_interface_;
};

namespace internal {

template <class Tp>
class StringAdapter<Tp, std::enable_if_t<IsStringV<Tp>>> : public ListAdapter<Tp, false /*no proxy*/> {
  using base = ListAdapter<Tp, false>;

 public:
  using const_adapter = StringAdapter<const Tp, void>;

  explicit StringAdapter(typename base::container_type* c) noexcept : base(c) {}

  decltype(auto) c_str() const noexcept { return base::container_->c_str(); }
  decltype(auto) data() const noexcept { return base::container_->data(); }

  void append(const void* cstr, size_t n) const {
    // If the container is const, do nothing. And it's assured that this method will never be called.
    if constexpr (!base::is_const) {
      base::container_->append(static_cast<const typename base::underlying_value_type*>(cstr), n);
    }
  }

  [[nodiscard]] std::any ToConst() const noexcept { return const_adapter{base::container_}; }
};

}  // namespace internal

template <class C, class>
auto AsString(C* container) noexcept {
  internal::StringAdapter<C> adapter{container};
  static_assert(std::is_trivially_copyable_v<decltype(adapter)>);
  static_assert(std::is_same_v<char, std::remove_cv_t<typename decltype(adapter)::value_type>>);

  constexpr auto const_opt = static_cast<ConstOption>(std::is_const_v<C>);
  auto& string_interface = internal::GetStringInterface<decltype(adapter)>();
  auto& list_interface = internal::GetListInterface<decltype(adapter)>();
  String<const_opt> string{adapter, list_interface, string_interface};
  return string;
}

}  // namespace liteproto