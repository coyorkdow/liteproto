//
// Created by Youtao Guo on 2023/6/24.
//

#pragma once

#include "basic_message.hpp"
#include <iterator>

class Descriptor {

};

class Reflection {
 public:
  virtual size_t FieldsSize() const = 0;
  virtual Reflection* Fields(size_t index) = 0;
  virtual const Reflection* Fields(size_t index) const = 0;
};