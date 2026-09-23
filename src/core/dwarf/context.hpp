#ifndef CAESAR_DWARF_CONTEXT_HPP
#define CAESAR_DWARF_CONTEXT_HPP

#include <libdwarf.h>

#include <memory>

#include "alloc.hpp"
#include "expected.hpp"

class DwarfContext {
 private:
  std::unique_ptr<Dwarf_Debug_s, DwarfDebugDeleter> m_dbg;
  // TODO: true if .debug_info (dwarf > 4)
  bool m_is_info = true;

  explicit DwarfContext(std::unique_ptr<Dwarf_Debug_s, DwarfDebugDeleter> ctx);

 public:
  DwarfContext() = default;
  static Expected<DwarfContext, std::string> create(const std::string& path);
  void listFuncsInDie();
  void listFuncInDie(Dwarf_Debug dbg, Dwarf_Die die);
  [[nodiscard]] auto& getDbg() const { return this->m_dbg; }
  [[nodiscard]] bool getIsInfo() const { return this->m_is_info; }
  ScopedDwarfError makeError() { return ScopedDwarfError(m_dbg.get()); }
  static char* getError(const ScopedDwarfError& err) {
    return dwarf_errmsg(err.raw());
  }
  [[nodiscard]] const std::string& err(std::string msg) const;
};

#endif
