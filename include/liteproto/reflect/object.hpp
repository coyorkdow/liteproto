//
// Created by Youtao Guo on 26/6/23.
//

#pragma once

#include <any>
#include <cstring>
#include <sstream>
#include <string>

#include "liteproto/reflect/type.hpp"
#include "liteproto/traits.hpp"

#include "liteproto/list.hpp"
#include "liteproto/pair.hpp"

namespace liteproto {

static_assert(sizeof(void*) == 4 || sizeof(void*) == 8);
using addr_t = std::conditional_t<sizeof(void*) == 4, uint32_t, uint64_t>;
static_assert(sizeof(addr_t) == sizeof(void*));

class Object {
 public:
  Type TypeEnum() const noexcept { return descriptor_.TypeEnum(); }

  addr_t Addr() const noexcept {
    addr_t addr = 0;
    std::memcpy(&addr, &addr_, sizeof(addr_t));
    return addr;
  }

  std::string Memory() const {
    //    std::stringstream stringstream;
    std::string str;
    if (addr_) {
      size_t size = descriptor_.SizeOf();
      str.append(static_cast<const char*>(addr_), size);
      //      auto* ptr = static_cast<uint8_t*>(addr_);
      //      for (size_t i = 0; i < size; i++, ptr++) {
      //        stringstream << std::hex << *ptr;
      //      }
    }
    return str;
    //    return stringstream.str();
  }

  std::any Value() const noexcept { return ptr_to_value_; }

  bool empty() const noexcept { return addr_ == nullptr; }
  TypeDescriptor Descriptor() const noexcept { return descriptor_; }

 private:
  TypeDescriptor descriptor_;
  std::any ptr_to_value_;
  std::any view_;
  void* addr_;
};

}  // namespace liteproto