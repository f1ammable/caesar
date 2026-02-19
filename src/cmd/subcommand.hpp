#ifndef CAESAR_SUBCOMMAND_H
#define CAESAR_SUBCOMMAND_H

#include <functional>
#include <initializer_list>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "error.hpp"
#include "object.hpp"
#include "token_type.hpp"
#include "typedefs.hpp"

class SubcommandHandler {
 private:
  std::string_view m_callee;
  std::unordered_map<std::string_view, FnPtr> m_subcommands;

 public:
  SubcommandHandler(
      std::initializer_list<std::pair<const std::string_view, FnPtr>> fns,
      const std::string_view callee)
      : m_subcommands(fns), m_callee(callee) {};

  Object exec(const std::string& subcmd, const std::vector<std::string>& args) {
    if (m_subcommands.contains(subcmd))
      return std::invoke(m_subcommands[subcmd], args);
    CmdError::error(TokenType::IDENTIFIER,
                    std::format("Subcommand {} is not valid for {} command",
                                subcmd, m_callee),
                    CmdErrorType::RUNTIME_ERROR);
    return std::monostate{};
  }
};

#endif
