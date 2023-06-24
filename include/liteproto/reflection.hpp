//
// Created by Youtao Guo on 2023/6/24.
//

#pragma once

#include <any>
#include <iterator>

#include "message.hpp"

class Descriptor {

};

class Reflection {
 public:
  virtual size_t FieldsSize() const = 0;
  virtual Reflection* Fields(size_t index) = 0;
  virtual const Reflection* Fields(size_t index) const = 0;
  virtual std::any Value() = 0;
  virtual std::any Value() const = 0;
};