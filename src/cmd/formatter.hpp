#ifndef FORMATTER_H
#define FORMATTER_H

#include <format>
#include "object.hpp"
#include "callable.hpp"

template <>
struct std::formatter<Object> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const Object& obj, std::format_context& ctx) const {
    auto visitor = [&ctx](const auto& value) -> std::format_context::iterator {
      using T = std::decay_t<decltype(value)>;

      if constexpr (std::is_same_v<T, std::monostate>) {
        return std::format_to(ctx.out(), "emtpy");
      } else if constexpr (std::is_same_v<T, std::string>) {
        return std::format_to(ctx.out(), "{}", value);
      } else if constexpr (std::is_same_v<T, bool>) {
        return std::format_to(ctx.out(), "{}", value ? "true" : "false");
      } else if constexpr (std::is_same_v<T, double>) {
        std::string text = std::format("{}", value);

        if (text.ends_with(".0")) {
          text = text.substr(0, text.length() - 2);
        }

        return std::format_to(ctx.out(), "{}", text);

      } else if constexpr (std::is_same_v<T, std::shared_ptr<Callable>>) {
        return std::format_to(ctx.out(), "{}", value->str());
      }

      else {
        return std::format_to(ctx.out(), "{}", value);
      }
    };

    return std::visit(visitor, obj);
  }
};

#endif
