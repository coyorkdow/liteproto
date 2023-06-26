//
// Created by Youtao Guo on 2023/6/24.
//

#pragma once

#include <cstddef>
#include <type_traits>

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

  // A list is a flexible-sized sequential container_type. The elements can be accessed by subscript operator (i.e.,
  // operator[]), but no random accessible required. A list also supports push_back and pop_back, and inserts a
  // new element to arbitrary position.
  // A list also supports size(), empty(), and clear().
  LIST,
  // A map is an key-value pairs collection. Each key must be unique in a map, and the value can be looked up via the
  // corresponding key. A map uses get() to look up and update() to modify. A map also supports size(), empty(), and
  // clear().
  MAP,
  // A string is almost the same as a list, except it doesn't be regarded as a container_type. A string is represented
  // by a single value, like "123" or "abc". Since the String is not a container_type, the underlying type cannot be
  // further inspected. A string supports all the operators that list supports.
  STRING,
  // An array is a fixed-sized sequential container_type. The elements can be accessed by subscript operator (i.e.,
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

enum class ConstOption : bool { NON_CONST = false, CONST = true };

}  // namespace liteproto