#ifndef CALLABLE_H
#define CALLABLE_H

#include <concepts>
#include <format>
#include <vector>

#include "cmd/subcommand.hpp"
#include "cmd/util.hpp"
#include "core/context.hpp"
#include "core/target.hpp"
#include "expected.hpp"
#include "object.hpp"
#include "typedefs.hpp"

class Interpreter;

class Callable {
 protected:
  std::unique_ptr<Target>& m_target = Context::getTarget();

 public:
  virtual ~Callable() = default;
  virtual Object call(std::vector<Object> args) = 0;
  [[nodiscard]] virtual int arity() const = 0;
  [[nodiscard]] virtual std::string str() const = 0;
};

template <typename T>
  requires std::derived_from<T, Callable>
struct std::formatter<T> {  // NOLINT(cert-dcl58-cpp)
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const T& fn, std::format_context& ctx) const {
    return std::format_to(ctx.out(), fn.str());
  }
};

class SubcommandCallable : public Callable {
 protected:
  SubcommandHandler m_subcmds;

  explicit SubcommandCallable(SubcommandHandler subcmds)
      : m_subcmds(std::move(subcmds)) {}

 public:
  Object call(std::vector<Object> args) override {
    if (args.empty()) return std::monostate{};
    auto* subcmdStr = std::get_if<std::string>(&args.front());
    if (!subcmdStr) return "Subcommand must be a string";
    std::string subcmd = *subcmdStr;
    args.erase(args.begin());
    return m_subcmds.exec(subcmd, args);
  }
};

inline FnPtr requiresRunningTarget(FnPtr fn) {
  return [fn = std::move(fn)](const std::vector<Object>& args) -> Object {
    auto& target = Context::getTarget();
    if (!target || !target->m_started) return "Target is not running!";
    return fn(args);
  };
}

template <typename T, typename E, typename Fn>
auto andThen(Expected<T, E>& exp, Fn&& fn) -> decltype(fn(*exp)) {
  if (!exp) return Unexpected{exp.error()};
  return fn(*exp);
}

#endif
