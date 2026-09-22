#include "context.hpp"

#include <dwarf.h>
#include <libdwarf.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

#include "alloc.hpp"

DwarfContext::DwarfContext(const std::string& path) {
  Dwarf_Debug rawDbg = nullptr;
  ScopedDwarfError err{rawDbg};
  std::array<char, PATH_MAX> truePathBuf{};
  int res =
      dwarf_init_path(path.c_str(), truePathBuf.data(), truePathBuf.size(),
                      DW_GROUPNUMBER_ANY, nullptr, nullptr, &rawDbg, err.get());

  if (res != DW_DLV_OK) {
    std::cout << dwarf_errmsg(err.raw()) << '\n';
    throw std::runtime_error("Failed to initialise libdwarf");
  }

  m_dbg.reset(rawDbg);
}

void DwarfContext::err(const std::string& msg) const {
  throw std::runtime_error(msg);
}
