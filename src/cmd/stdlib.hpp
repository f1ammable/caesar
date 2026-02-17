#ifndef STDLIB_H
#define STDLIB_H

#include <sys/wait.h>

#include <core/context.hpp>
#include <core/util.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

#include "callable.hpp"
#include "cmd/object.hpp"
#include "core/macho/macho.hpp"
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
  using FnPtr = Object (*)(const std::vector<std::string>&);

  FnPtr list = [](const std::vector<std::string>& args) -> Object {
    auto& breakpoints =
        Context::getInstance().getTarget()->getRegisteredBreakpoints();
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
    auto* macho =
        dynamic_cast<Macho*>(Context::getInstance().getTarget().get());
    if (macho->getAslrSlide() == 0) macho->readAslrSlide();

    u64 addr = 0;
    try {
      addr = static_cast<u64>(std::stoull(args[0], nullptr, 0));
      auto& target = Context::getInstance().getTarget();
      i32 res = target->setBreakpoint(addr);
      if (res != 0)
        return "Error setting breakpoint!\n";
      else
        return std::format("Breakpoint set at: {}", toHex(addr));
    } catch (std::invalid_argument& e) {
      return std::format("Could not convert from argument {} to address!\n",
                         args[0]);
    } catch (std::out_of_range& e) {
      return "Address provided is out of range!\n";
    }
  };

  SubcommandHandler m_subcmds{{{sv("list"), list}, {sv("set"), set}},
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
        break;
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
    if (Context::getInstance().getTarget() == nullptr) {
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
    auto& target = Context::getInstance().getTarget();
    if (target == nullptr) {
      std::cerr << "Error: Target is not set!\n";
      CmdError::getInstance().m_had_error = true;
      return std::monostate{};
    }

    i32 pid = target->launch(argList);
    if (pid >= 0)
      std::cout << std::format("Target started with pid {}\n", pid);
    else
      return "Unable to start target (are you running with sudo?)\n";

    i32 res = target->attach();
    if (res != 0) return "Could not attach to target!\n";
    target->setTargetState(TargetState::RUNNING);
    // TODO: Reset this when target exits
    target->m_started = true;
    target->startEventLoop();

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
    auto& target = Context::getInstance().getTarget();
    if (target->getTargetState() != TargetState::STOPPED) {
      std::cout << "Target still seems to think its running?\n";
      return std::monostate{};
    }
    target->resume(ResumeType::RESUME);
    target->startEventLoop();
    return std::monostate{};
  }
};

#endif
