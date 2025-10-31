#ifndef CAESAR_DWARF_CONTEXT_HPP
#define CAESAR_DWARF_CONTEXT_HPP

#include <libdwarf.h>

#include <memory>

class DwarfContext {
 private:
  std::shared_ptr<Dwarf_Debug_s> m_dbg;
  // TODO: true if .debug_info (dwarf > 4)
  bool m_is_info = true;
  void err(const std::string& msg);

 public:
  DwarfContext(const std::string& path);
  void list_funcs_in_die(Dwarf_Debug dbg);
  void list_func_in_die(Dwarf_Debug dbg, Dwarf_Die die);
  inline auto get_dbg() { return this->m_dbg; }
};

#endif
