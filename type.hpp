//
// Created by Youtao Guo on 2023/6/24.
//

#pragma

#include <type_traits>

using uint8_t = uint8_t;
using int8_t = int8_t;
using uint32_t = uint32_t;
using int32_t = int32_t;
using uint64_t = uint64_t;
using int64_t = int64_t;
using float_32_t = float;
using float_64_t = double;

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

  // Thees are four data structures abstraction that supported by liteproto: list, map, string, array, and tuple.

  // A list is a flexible-sized sequential container. The elements can be accessed by subscript operator (i.e.,
  // operator[]), but no random accessible required. A list also supports push&pop in both front and back, and insert a
  // new element to arbitrary position.
  // A list also supports size(), empty(), and clear().
  LIST,
  // A map is an associative container that in form of the key-vale structure. Each key must be unique in a map, and the
  // value can be looked up via the corresponding key. A map uses get() to look up and update() to modify.
  // A map also supports size(), empty(), and clear().
  MAP,
  // A string is almost same as a list, except it doesn't be regarded as a container. A string ia represented by a
  // single value, like "123" or "abc". Since String is not container, the underlying type cannot be further inspected.
  // A string supports all the operator that list supports.
  STRING,
  // An array is a fixed-sized sequential container. The elements can be accessed by subscript operator (i.e.,
  // operator[]). It doesn't support any other way to look up or modify. An array also supports size().
  ARRAY,
  // TODO
  TUPLE
};

