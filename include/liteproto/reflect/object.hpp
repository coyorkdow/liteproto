//
// Created by Youtao Guo on 26/6/23.
//

#pragma once

#include <any>
#include <cstring>
#include <optional>
#include <sstream>
#include <string>

#include "liteproto/reflect/number.hpp"
#include "liteproto/reflect/type.hpp"
#include "liteproto/traits/traits.hpp"

namespace liteproto {

static_assert(sizeof(void*) == 4 || sizeof(void*) == 8);
using addr_t = std::conditional_t<sizeof(void*) == 4, uint32_t, uint64_t>;
static_assert(sizeof(addr_t) == sizeof(void*));

class Object;

template <class Tp, ConstOption Opt>
class List;

template <ConstOption Opt>
class String;

template <class K, class V, ConstOption Opt>
class Map;

template <ConstOption Opt = ConstOption::NON_CONST>
std::optional<NumberReference<Opt>> NumberCast(const Object& object) noexcept;

template <class Tp, ConstOption Opt = ConstOption::NON_CONST>
std::optional<List<Tp, Opt>> ListCast(const Object& object) noexcept;

template <ConstOption Opt = ConstOption::NON_CONST>
std::optional<String<Opt>> StringCast(const Object& object) noexcept;

class Object {
  template <class Tp>
  friend Object GetReflection(Tp* v) noexcept;

  template <ConstOption Opt>
  friend std::optional<NumberReference<Opt>> NumberCast(const Object& object) noexcept;

  template <class Tp, ConstOption Opt>
  friend std::optional<List<Tp, Opt>> ListCast(const Object& object) noexcept;

  template <ConstOption Opt>
  friend std::optional<String<Opt>> StringCast(const Object& object) noexcept;

 public:
  [[nodiscard]] addr_t Addr() const noexcept {
    addr_t addr = 0;
    std::memcpy(&addr, &addr_, sizeof(addr_t));
    return addr;
  }

  [[nodiscard]] std::string Memory() const {
    std::string str;
    if (addr_) {
      size_t size = descriptor_->SizeOf();
      str.append(static_cast<const char*>(addr_), size);
    }
    return str;
  }

  [[nodiscard]] const std::any& Value() const noexcept { return ptr_to_value_; }
  [[nodiscard]] bool empty() const noexcept { return addr_ == nullptr; }
  [[nodiscard]] const TypeDescriptor& Descriptor() const noexcept { return *descriptor_; }

  Object() noexcept : descriptor_(), addr_(nullptr) {}
  Object(const Object&) = default;
  Object(Object&& rhs) noexcept
      : descriptor_(rhs.descriptor_), ptr_to_value_(std::move(rhs.ptr_to_value_)), interface_(std::move(rhs.interface_)), addr_(rhs.addr_) {
    rhs.addr_ = nullptr;
  }
  Object& operator=(const Object&) = default;
  Object& operator=(Object&& rhs) noexcept {
    descriptor_ = rhs.descriptor_;
    ptr_to_value_ = std::move(rhs.ptr_to_value_);
    interface_ = std::move(rhs.interface_);
    addr_ = rhs.addr_;
    rhs.addr_ = nullptr;
    return *this;
  }

 private:
  template <class Tp, class Interface>
  Object(Tp* value_ptr, Interface interface) noexcept
      : descriptor_(&TypeMeta<Tp>::GetDescriptor()), ptr_to_value_(value_ptr), interface_(std::move(interface)), addr_(value_ptr) {}

  template <class Tp, class = std::enable_if_t<std::is_scalar_v<Tp>>>
  explicit Object(Tp* value_ptr) noexcept : descriptor_(&TypeMeta<Tp>::GetDescriptor()), ptr_to_value_(value_ptr), addr_(value_ptr) {}

  const TypeDescriptor* descriptor_;
  std::any ptr_to_value_;
  std::any interface_;
  const void* addr_;
};

template <class Tp>
[[nodiscard]] Object GetReflection(Tp* v) noexcept;

template <class Tp>
Tp* ObjectCast(const Object& object) noexcept {
  auto indirect_ptr = std::any_cast<Tp*>(&object.Value());
  if (indirect_ptr == nullptr) {
    return nullptr;
  }
  return *indirect_ptr;
}

template <ConstOption Opt>
std::optional<NumberReference<Opt>> NumberCast(const Object& object) noexcept {
  using return_type = std::optional<NumberReference<Opt>>;
  auto indirect_ptr = std::any_cast<NumberReference<Opt>>(&object.interface_);
  if (indirect_ptr == nullptr) {
    return return_type{};
  }
  return return_type{*indirect_ptr};
}

}  // namespace liteproto