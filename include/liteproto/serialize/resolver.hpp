//
// Created by Youtao Guo on 2023/7/2.
//

#pragma once

#include "liteproto/reflect.hpp"

namespace liteproto {

template <class Tp>
class ListResolver {
 public:
  template <class Receiver>
  Receiver& operator()(Receiver& receiver) const {

  }
};

}  // namespace liteproto