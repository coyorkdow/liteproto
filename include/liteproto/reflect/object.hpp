//
// Created by Youtao Guo on 26/6/23.
//

#pragma once

#include <any>

#include "liteproto/traits.hpp"
#include "liteproto/reflect/type.hpp"

namespace liteproto {

class Object {
 public:
  Type type() const noexcept;

  std::any Value() const noexcept;

 private:

};

}  // namespace liteproto