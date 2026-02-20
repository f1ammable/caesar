#ifndef STDLIB_H
#define STDLIB_H

#include <sys/wait.h>

#include <core/context.hpp>
#include <core/util.hpp>
#include <iostream>
#include <string>
#include <variant>

#include "callable.hpp"
#include "cmd/object.hpp"
#include "core/target.hpp"
#include "error.hpp"
#include "subcommand.hpp"
#include "token_type.hpp"

class LenFn : public Callable {
 public:
  [[nodiscard]] int arity() const override { return 1; }
  [[nodiscard]] std::string str() const override { return "<native fn: len>"; }

  Object call(std::vector<Object> args) override {
    auto* val = std::get_if<std::string>(args.data());

    if (val == nullptr) {
      CmdError::error(TokenType::IDENTIFIER,
                      "len can only be called on a string",
                      CmdErrorType::RUNTIME_ERROR);
      return std::monostate{};
    }

    return static_cast<double>(val->length());
  }
};

class PrintFn : public Callable {
 public:
  [[nodiscard]] int arity() const override { return 1; }
  [[nodiscard]] std::string str() const override {
    return "<native fn: print>";
  }

  Object call(std::vector<Object> args) override { return args[0]; }
};

class BreakpointFn : public Callable {
 private:
  using sv = std::string_view;

  static Object rmOrToggleBreakpoint(const std::vector<std::string>& args,
                                     bool toggle = false) {
    u64 addr = strToAddr(args[0]);
    if (addr == -1)
      return std::format("Could not convert from {} to address!", args[0]);
    else if (addr == -2)
      return "Address provided is out of range!";

    auto& target = Context::getTarget();
    auto& bps = target->getRegisteredBreakpoints();

    if (!bps.contains(addr)) return std::format("No breakpoint at {}", addr);

    if (toggle) {
      i32 res = target->disableBreakpoint(addr, true);
      if (res != 0) return "Error toggling breakpoint";
      return std::format("Toggled breakpoint at {}, now {}", addr,
                         bps[addr].enabled);
    }

    i32 res = target->disableBreakpoint(addr, false);
    if (res != 0) return "Error disabling breakpoint";
    return std::format("Removed breakpoint at {}", addr);
  }

  FnPtr list = [](const std::vector<std::string>& args) -> Object {
#pragma unused(args)
    auto& breakpoints = Context::getTarget()->getRegisteredBreakpoints();
    std::string retStr{};

    if (breakpoints.empty()) return "No breakpoints set!";

    for (const auto& [k, v] : breakpoints) {
      retStr += std::format("Breakpoint @ {} ({})\n", toHex(k),
                            breakpoints[k].enabled);
    }

    retStr.pop_back();
    return retStr;
  };

  FnPtr set = [](const std::vector<std::string>& args) -> Object {
    // TODO: This whole thing can be extracted but `Object` does not support
    // `u64`
    u64 addr = strToAddr(args[0]);
    if (addr == -1)
      return std::format("Could not convert from {} to address!", args[0]);
    else if (addr == -2)
      return "Address provided is out of range!";

    i32 res = Context::getTarget()->setBreakpoint(addr);
    if (res != 0)
      return "Error setting breakpoint!";
    else
      return std::format("Breakpoint set at: {}", toHex(addr));
  };

  FnPtr remove = [](const std::vector<std::string>& args) -> Object {
    return BreakpointFn::rmOrToggleBreakpoint(args, false);
  };

  FnPtr toggle = [](const std::vector<std::string>& args) -> Object {
    return BreakpointFn::rmOrToggleBreakpoint(args, true);
  };

  SubcommandHandler m_subcmds{{{sv("list"), list},
                               {sv("set"), set},
                               {sv("remove"), remove},
                               {sv("toggle"), toggle}},
                              "breakpoint"};

  static std::vector<std::string> convertToStr(const std::vector<Object>& vec) {
    std::vector<std::string> res{};
    res.reserve(vec.size());

    for (const auto& x : vec) {
      const auto* const str = std::get_if<std::string>(&x);
      if (str == nullptr) {
        CmdError::error(TokenType::IDENTIFIER,
                        "Please provide all arguments as strings",
                        CmdErrorType::RUNTIME_ERROR);
        return {};  // Return empty on error, not partial
      }
      res.emplace_back(*str);
    }

    return res;
  }

 public:
  [[nodiscard]] int arity() const override { return 1; }
  [[nodiscard]] std::string str() const override {
    return "<native fn: breakpoint>";
  }

  Object call(std::vector<Object> args) override {
    if (m_target == nullptr) {
      CoreError::error("Target is not running!");
      return std::monostate{};
    }
    std::vector<std::string> convertedArgs = BreakpointFn::convertToStr(args);
    if (convertedArgs.size() < 1) return std::monostate{};
    const std::string subcmd = convertedArgs.front();
    convertedArgs.erase(convertedArgs.begin());
    return m_subcmds.exec(subcmd, convertedArgs);
  }
};

class RunFn : public Callable {
 private:
  static CStringArray concatElems(const std::vector<Object>& v) {
    auto ensureStr = [](const auto& obj) -> std::string {
      using T = std::decay_t<decltype(obj)>;

      if constexpr (std::is_same_v<T, double>)
        return std::to_string(obj);
      else if constexpr (std::is_same_v<T, std::string>)
        return obj;
      else if constexpr (std::is_same_v<T, bool>)
        return obj ? "true" : "false";
      else
        return "";
    };

    CStringArray res{};
    res.storage.reserve(v.size());
    res.ptrs.reserve(v.size() + 1);

    for (const auto& x : v) {
      std::string temp = std::visit(ensureStr, x);
      if (temp.empty()) {
        CmdError::error(
            TokenType::IDENTIFIER,
            "Only strings, bools and numbers are supported as arguments",
            CmdErrorType::RUNTIME_ERROR);
        break;
      }

      res.storage.emplace_back(temp.begin(), temp.end());
      res.storage.back().push_back('\0');
      res.ptrs.push_back(res.storage.back().data());
    }
    res.ptrs.push_back(nullptr);
    return res;
  };

 public:
  // TODO: Allow for inline target args (just one string containing all args)
  [[nodiscard]] int arity() const override { return 0; }
  [[nodiscard]] std::string str() const override { return "<native fn: run>"; }

  Object call(std::vector<Object> args) override {
    CStringArray argList = RunFn::concatElems(args);
    if (m_target == nullptr) {
      std::cerr << "Error: Target is not set!\n";
      CmdError::getInstance().m_had_error = true;
      return std::monostate{};
    }

    i32 pid = m_target->launch(argList);
    if (pid >= 0)
      std::cout << std::format("Target started with pid {}\n", pid);
    else
      return "Unable to start target (are you running with sudo?)\n";

    i32 res = m_target->attach();
    if (res != 0) return "Could not attach to target!\n";
    m_target->setTargetState(TargetState::RUNNING);
    // TODO: Reset this when target exits
    m_target->m_started = true;
    m_target->startEventLoop();

    return std::monostate{};
  }
};

class ContinueFn : public Callable {
 public:
  [[nodiscard]] int arity() const override { return 0; }
  [[nodiscard]] std::string str() const override {
    return "<native fn: continue>";
  }
  Object call(std::vector<Object> args) override {
#pragma unused(args)
    if (m_target->getTargetState() != TargetState::STOPPED) {
      std::cout << "Target still seems to think its running?\n";
      return std::monostate{};
    }
    m_target->resume(ResumeType::RESUME);
    m_target->startEventLoop();
    return std::monostate{};
  }
};

#endif
