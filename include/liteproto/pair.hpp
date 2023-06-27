//
// Created by Youtao Guo on 2023/6/27.
//

#pragma once

#include "liteproto/interface.hpp"
#include "liteproto/reflect/type.hpp"

namespace liteproto {

template <class Car, class Cdr, ConstOption ConstOpt>
class Pair;

template <class Car, class Cdr>
class Pair<Car, Cdr, ConstOption::NON_CONST> {
 public:
  Car& CAR() const noexcept { return interface_.car(obj_); }
  Cdr& CDR() const noexcept { return interface_.cdr(obj_); }

 private:
  internal::PairInterface<Car, Cdr> interface_;
  mutable std::any obj_;
};

template <class Car, class Cdr>
class Pair<Car, Cdr, ConstOption::CONST> {
 public:
  const Car& CAR() const noexcept { return interface_.car(obj_); }
  const Cdr& CDR() const noexcept { return interface_.cdr(obj_); }

 private:
  internal::PairInterface<const Car, const Cdr> interface_;
  mutable std::any obj_;
};

}  // namespace liteproto