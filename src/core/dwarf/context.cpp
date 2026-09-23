#include "context.hpp"

#include <dwarf.h>
#include <libdwarf.h>

#include <array>
#include <format>
#include <string>

#include "alloc.hpp"
#include "expected.hpp"

Expected<DwarfContext, std::string> DwarfContext::create(
    const std::string& path) {
  Dwarf_Debug rawDbg = nullptr;
  ScopedDwarfError err{rawDbg};
  std::array<char, PATH_MAX> truePathBuf{};
  int res =
      dwarf_init_path(path.c_str(), truePathBuf.data(), truePathBuf.size(),
                      DW_GROUPNUMBER_ANY, nullptr, nullptr, &rawDbg, err.get());
  if (res == DW_DLV_NO_ENTRY)
    return Unexpected{""};

  else if (res != DW_DLV_OK) {
    return Unexpected{std::format("Failed to initialise libdwarf {} \n",
                                  dwarf_errmsg(err.raw()))};
  }

  return DwarfContext{
      std::unique_ptr<Dwarf_Debug_s, DwarfDebugDeleter>(rawDbg)};
}

DwarfContext::DwarfContext(
    std::unique_ptr<Dwarf_Debug_s, DwarfDebugDeleter> ctx)
    : m_dbg(std::move(ctx)) {}
