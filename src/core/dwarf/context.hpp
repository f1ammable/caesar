#ifndef CAESAR_DWARF_CONTEXT_HPP
#define CAESAR_DWARF_CONTEXT_HPP

#include <libdwarf.h>

#include <memory>

#include "alloc.hpp"

class DwarfContext {
 private:
  std::unique_ptr<Dwarf_Debug_s, DwarfDebugDeleter> m_dbg;
  // TODO: true if .debug_info (dwarf > 4)
  bool m_is_info = true;
  void err(const std::string& msg);

 public:
  explicit DwarfContext(const std::string& path);
  void listFuncsInDie();
  void listFuncInDie(Dwarf_Debug dbg, Dwarf_Die die);
  auto& getDbg() { return this->m_dbg; }
  ScopedDwarfError makeError() { return ScopedDwarfError(m_dbg.get()); }
  static char* getError(const ScopedDwarfError& err) { return dwarf_errmsg(err.raw()); }
};

#endif
