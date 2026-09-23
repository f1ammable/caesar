#ifndef CAESAR_DWARF_DIE_H
#define CAESAR_DWARF_DIE_H

#include <libdwarf.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "core/decoder.hpp"
#include "core/dwarf/context.hpp"

struct FnInfo {
  std::string m_name;
  Dwarf_Addr m_lowpc{};
  Dwarf_Addr m_highpc{};
};

class DwarfIndex {
 private:
  std::vector<FnInfo> m_fns;
  std::unordered_map<std::string, std::size_t> m_fns_name;

  Expected<std::monostate, std::string> getFunctionSymbols(
      const DwarfContext& ctx, Dwarf_Die childDie);
  [[nodiscard]] Expected<Dwarf_Die, std::string> getCUDie(
      const DwarfContext& ctx) const;

 public:
  static DwarfIndex build(const std::string& path);
  [[nodiscard]] const FnInfo& findByName(const std::string& name) const;
  [[nodiscard]] const FnInfo& findByAddr(AddrType addr) const;
  [[nodiscard]] const std::vector<FnInfo>& getFunctions() const {
    return m_fns;
  };
};

#endif  // CAESAR_DWARF_DIE_H
