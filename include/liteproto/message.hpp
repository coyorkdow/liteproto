//
// Created by Youtao Guo on 23/6/23.
//

#pragma once

#include <array>
#include <iostream>
#include <map>
#include <type_traits>
#include <utility>

#include "liteproto/reflect/object.hpp"
#include "liteproto/reflect/type.hpp"
#include "liteproto/utils.hpp"

namespace liteproto {

class Message {
 public:
  virtual Object Field(size_t index) = 0;
  virtual Object Field(const std::string& name) = 0;
  virtual Object Field(size_t index) const = 0;
  virtual Object Field(const std::string& name) const = 0;
  virtual const std::string& FieldName(size_t index) const = 0;
  virtual bool HasName(const std::string& name) const = 0;
  virtual size_t FieldsSize() const noexcept = 0;
};

template <class Msg, int32_t Line>
class MessageBase : public Message {
  friend constexpr decltype(auto) internal::GetAllFields<Msg>();
  friend constexpr decltype(auto) internal::GetAllFields2<Msg>();

  static constexpr int32_t FIELDS_start = Line;

  struct FieldsIndices {
#if defined(LITE_PROTO_DISABLE_COMPATIBLE_MODE_)
    static constexpr auto value = internal::GetAllFields<Msg>();
#else
    static constexpr auto value = internal::GetAllFields2<Msg>();
#endif
  };

  struct VariantType {
   private:
    static constexpr auto indices = FieldsIndices::value;
    template <size_t... I>
    static auto VariantTypeHelper(std::index_sequence<I...>)
        -> std::variant<decltype(std::declval<Msg>().FIELD_ptr(int32_constant<indices[I].second>{}))...>;

   public:
    using type = decltype(VariantTypeHelper(std::make_index_sequence<FieldsIndices::value.size()>{}));
  };

 public:
  [[nodiscard]] auto DumpTuple() const noexcept { return DumpTupleImpl(std::make_index_sequence<FieldsIndices::value.size()>{}); }
  [[nodiscard]] auto DumpTuple() noexcept { return DumpTupleImpl(std::make_index_sequence<FieldsIndices::value.size()>{}); }

  template <class Fn>
  void Visit(size_t index, Fn&& fn) {
    std::visit([this, fn = std::forward<Fn>(fn)](auto&& ptr) {
      auto& msg = static_cast<Msg&>(*this);
      fn(msg.*ptr);
    }, VariantArr()[index]);
  }

  Object Field(size_t index) override {
    [[maybe_unused]] static const bool dummy = ApplyReflectForEachField(static_cast<Msg*>(this), &fields_);
    return fields_.at(FieldsIndices::value[index].first);
  }
  Object Field(const std::string& name) override { return Field(fields_name_.at(name)); }

  Object Field(size_t index) const override {
    [[maybe_unused]] static const bool dummy = ApplyReflectForEachField(static_cast<const Msg*>(this), &const_fields_);
    return const_fields_.at(FieldsIndices::value[index].first);
  }
  Object Field(const std::string& name) const override { return Field(fields_name_.at(name)); }

  const std::string& FieldName(size_t index) const override { return fields_name_inverse_.at(index); }
  bool HasName(const std::string& name) const override { return fields_name_.count(name); }

  size_t FieldsSize() const noexcept override { return FieldsIndices::value.size(); }

  template <class Tp, class Fn>
  static constexpr auto ForEach(Fn&& fn) noexcept {
    return ForEachImpl<Tp>(std::make_index_sequence<FieldsIndices::value.size()>{}, std::forward<Fn>(fn));
  }

 private:
  template <size_t I = 0, size_t N>
  static constexpr auto GetFieldIndexByName(const char (&str)[N]) {
    constexpr auto indices = FieldsIndices::value;
    if constexpr (I >= indices.size()) {
      return internal::PII{-1, -1};
    } else if (StrLiteralEQ(str, Msg::FIELD_name(int32_constant<indices[I].second>{}))) {
      return indices[I];
    } else {
      return GetFieldIndexByName<I + 1>(str);
    }
  }

  template <size_t... I>
  [[nodiscard]] auto DumpTupleImpl(std::index_sequence<I...>) const noexcept {
    auto& msg = static_cast<const Msg&>(*this);
    constexpr auto indices = FieldsIndices::value;
    return std::forward_as_tuple(msg.FIELD_value(int32_constant<indices[I].second>{})...);
  }

  template <size_t... I>
  [[nodiscard]] auto DumpTupleImpl(std::index_sequence<I...>) noexcept {
    auto& msg = static_cast<Msg&>(*this);
    constexpr auto indices = FieldsIndices::value;
    return std::forward_as_tuple(msg.FIELD_value(int32_constant<indices[I].second>{})...);
  }

  template <size_t... I>
  [[nodiscard]] static constexpr auto MakeVariantArrImpl(std::index_sequence<I...>) noexcept {
    return std::array{MakeVariant<I>()...};
  }

  template <size_t I>
  static constexpr auto MakeVariant() noexcept {
    using var_t = typename VariantType::type;
    constexpr auto indices = FieldsIndices::value;
    auto pseudo_obj = static_cast<Msg*>(nullptr);
    return var_t{pseudo_obj->FIELD_ptr(int32_constant<indices[I].second>{})};
  }

  template <class Tp, class Fn, size_t... I>
  static constexpr auto ForEachImpl(std::index_sequence<I...>, Fn&& fn) noexcept {
    Tp res{};
    (std::forward<Fn>(fn).template operator()<I>(&res), ...);
    return res;
  }

  struct GetName {
    template <size_t I>
    void operator()(std::map<std::string, size_t>* field_name) noexcept {
      constexpr auto index = FieldsIndices::value[I];
      (*field_name)[Msg::FIELD_name(int32_constant<index.second>{})] = I;
    }
  };

  struct GetNameInverse {
    template <size_t I>
    void operator()(std::vector<std::string>* field_name) noexcept {
      constexpr auto index = FieldsIndices::value[I];
      field_name->emplace_back(Msg::FIELD_name(int32_constant<index.second>{}));
    }
  };

  template <size_t I = 0, class Msg_>
  static bool ApplyReflectForEachField(Msg_* msg, std::map<int32_t, Object>* field) {
    if constexpr (I < FieldsIndices::value.size()) {
      constexpr auto index = FieldsIndices::value[I];
      field->insert(std::make_pair(index.first, GetReflection(&msg->FIELD_value(int32_constant<index.second>{}))));
      return ApplyReflectForEachField<I + 1>(msg, field);
    } else {
      return true;
    }
  }

  static decltype(auto) VariantArr() noexcept {
    static const auto arr = MakeVariantArrImpl(std::make_index_sequence<FieldsIndices::value.size()>{});
    return arr;
  }

  std::map<int32_t, Object> fields_;
  mutable std::map<int32_t, Object> const_fields_;

  static inline const std::map<std::string, size_t> fields_name_ = ForEach<std::map<std::string, size_t>>(GetName{});
  static inline const std::vector<std::string> fields_name_inverse_ = ForEach<std::vector<std::string>>(GetNameInverse{});
};

}  // namespace liteproto
