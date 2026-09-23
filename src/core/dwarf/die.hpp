#ifndef CAESAR_DWARF_DIE_H
#define CAESAR_DWARF_DIE_H

#include <libdwarf.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/decoder.hpp"
#include "core/dwarf/context.hpp"

enum class SymType : std::uint8_t { FUN, TYPE };

struct SymInfo {
  std::string m_name;
  SymType m_type;
  Dwarf_Addr m_lowpc{};
  Dwarf_Addr m_highpc{};
};

class DwarfIndex {
 private:
  std::vector<SymInfo> m_symbols;
  std::unordered_map<std::string, std::size_t> m_symbols_name;

  Expected<SymInfo, std::string> resolveFunctionFromTag(const DwarfContext& ctx,
                                                        Dwarf_Die& die,
                                                        Dwarf_Half tag);
  Expected<std::string, std::string> resolveFunctionTypeRef(
      const DwarfContext& ctx, Dwarf_Die& die);

  Expected<std::monostate, std::string> getSymbols(const DwarfContext& ctx,
                                                   Dwarf_Die childDie);
  [[nodiscard]] Expected<Dwarf_Die, std::string> getCUDie(
      const DwarfContext& ctx) const;

 public:
  static DwarfIndex build(const std::string& path);
  [[nodiscard]] const SymInfo& findByName(const std::string& name) const;
  [[nodiscard]] const SymInfo& findByAddr(AddrType addr) const;
  [[nodiscard]] const std::vector<SymInfo>& getSymbols() const {
    return m_symbols;
  };
};

#endif  // CAESAR_DWARF_DIE_H
