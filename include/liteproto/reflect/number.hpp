//
// Created by Youtao Guo on 2023/7/2.
//

#pragma once

#include "liteproto/reflect/type.hpp"

namespace liteproto {

class Number {
 public:
  Number() noexcept : uint64_(0), descriptor_(&TypeMeta<uint64_t>::GetDescriptor()) {}

  template <class Arithmetic, class = std::enable_if_t<std::is_arithmetic_v<Arithmetic>>>
  Number(Arithmetic v) noexcept : uint64_(0), descriptor_(&TypeMeta<Arithmetic>::GetDescriptor()) {
    if constexpr (std::is_floating_point_v<Arithmetic>) {
      float64_ = static_cast<double>(v);
    } else if constexpr (std::is_signed_v<Arithmetic>) {
      int64_ = static_cast<int64_t>(v);
    } else {
      uint64_ = static_cast<uint64_t>(v);
    }
  }

  bool IsSignedInteger() const noexcept { return descriptor_->Traits(traits::is_signed); }
  bool IsUnsigned() const noexcept { return descriptor_->Traits(traits::is_unsigned); }
  bool IsFloating() const noexcept { return descriptor_->Traits(traits::is_floating_point); }

  void SetInt64(int64_t v) noexcept {
    int64_ = v;
    descriptor_ = &TypeMeta<int64_t>::GetDescriptor();
  }
  void SetUInt64(uint64_t v) noexcept {
    uint64_ = v;
    descriptor_ = &TypeMeta<uint64_t>::GetDescriptor();
  }
  void SetFloat64(double v) noexcept {
    float64_ = v;
    descriptor_ = &TypeMeta<double>::GetDescriptor();
  }
  int64_t AsInt64() const noexcept { return int64_; }
  uint64_t AsUInt64() const noexcept { return uint64_; }
  double AsFloat64() const noexcept { return float64_; }

  const TypeDescriptor& Descriptor() const noexcept { return *descriptor_; }

 private:
  union {
    int64_t int64_;
    uint64_t uint64_;
    double float64_;
  };
  const TypeDescriptor* descriptor_{};
  static_assert(sizeof(int64_t) == sizeof(uint64_t) && sizeof(uint64_t) == sizeof(double));
};

namespace internal {

struct NumberInterface {
  using set_int64_t = void(void*, int64_t) noexcept;
  using set_uint64_t = void(void*, uint64_t) noexcept;
  using set_float64_t = void(void*, double) noexcept;
  using as_int64_t = int64_t(const void*) noexcept;
  using as_uint64_t = uint64_t(const void*) noexcept;
  using as_float64_t = double(const void*) noexcept;

  set_int64_t* set_int64;
  set_uint64_t* set_uint64;
  set_float64_t* set_float64;
  as_int64_t* as_int64;
  as_uint64_t* as_uint64;
  as_float64_t* as_float64;
};

template <class Tp>
struct NumberInterfaceImpl {
  template <class V>
  static void Set(void* ptr, V v) noexcept {
    if constexpr (!std::is_const_v<Tp>) {
      auto number = static_cast<Tp*>(ptr);
      *number = static_cast<Tp>(v);
    }
  }

  template <class V>
  static V As(const void* ptr) noexcept {
    auto number = static_cast<const Tp*>(ptr);
    return static_cast<V>(*number);
  }

  static const NumberInterface& MakeNumberInterface() {
    static const NumberInterface inter = [&] {
      NumberInterface interface {};
      interface.set_int64 = &Set<int64_t>;
      interface.set_uint64 = &Set<uint64_t>;
      interface.set_float64 = &Set<double>;
      interface.as_int64 = &As<int64_t>;
      interface.as_uint64 = &As<uint64_t>;
      interface.as_float64 = &As<double>;
      return interface;
    }();
    return inter;
  }
};

}  // namespace internal

template <ConstOption Opt>
class NumberReference;

template <>
class NumberReference<ConstOption::NON_CONST> {
 public:
  template <class Tp, class = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<Tp>>>>
  NumberReference(Tp&& v)
      : ptr_(&v), interface_(&internal::NumberInterfaceImpl<std::remove_reference_t<Tp>>::MakeNumberInterface()) {}

  void SetInt64(int64_t v) noexcept { interface_->set_int64(ptr_, v); }
  void SetUInt64(uint64_t v) noexcept { interface_->set_uint64(ptr_, v); }
  void SetFloat64(double v) noexcept { interface_->set_float64(ptr_, v); }
  int64_t AsInt64() const noexcept { return interface_->as_int64(ptr_); }
  uint64_t AsUInt64() const noexcept { return interface_->as_uint64(ptr_); }
  double AsFloat64() const noexcept { return interface_->as_float64(ptr_); }

 private:
  void* ptr_;
  const internal::NumberInterface* interface_;
};

template <>
class NumberReference<ConstOption::CONST> {
 public:
  template <class Tp, class = std::enable_if_t<std::is_arithmetic_v<Tp>>>
  NumberReference(const Tp& v)
      : ptr_(&v), interface_(&internal::NumberInterfaceImpl<const Tp>::MakeNumberInterface()) {}

  int64_t AsInt64() const noexcept { return interface_->as_int64(ptr_); }
  uint64_t AsUInt64() const noexcept { return interface_->as_uint64(ptr_); }
  double AsFloat64() const noexcept { return interface_->as_float64(ptr_); }

 private:
  const void* ptr_;
  const internal::NumberInterface* interface_;
};

}  // namespace liteproto