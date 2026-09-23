#include "die.hpp"

#include <dwarf.h>
#include <libdwarf.h>

#include <iostream>
#include <variant>

#include "core/dwarf/alloc.hpp"
#include "core/dwarf/context.hpp"
#include "expected.hpp"

DwarfIndex DwarfIndex::build(const std::string& path) {
  DwarfIndex index;
  if (Expected<DwarfContext, std::string> ctxRes = DwarfContext::create(path);
      ctxRes) {
    const DwarfContext& ctx = ctxRes.value();

    if (Expected<Dwarf_Die, std::string> cuDieRes = index.getCUDie(ctx);
        cuDieRes) {
      Dwarf_Die cuDie = cuDieRes.value();

      if (index.getSymbols(ctx, cuDie)) {
        std::ranges::sort(index.m_symbols, {}, &SymInfo::m_lowpc);
        for (std::size_t i = 0; i < index.m_symbols.size(); i++) {
          index.m_symbols_name[index.m_symbols[i].m_name] = i;
        }
      }
    }
  }
  return index;
}

Expected<Dwarf_Die, std::string> DwarfIndex::getCUDie(
    const DwarfContext& ctx) const {
  Dwarf_Unsigned cuHeaderLength = 0;
  Dwarf_Unsigned abbrevOffset = 0;
  Dwarf_Unsigned nextCuHeader = 0;
  Dwarf_Unsigned typeOffset = 0;
  Dwarf_Half versionStamp = 0;
  Dwarf_Half addressSize = 0;
  Dwarf_Half lengthSize = 0;
  Dwarf_Half extensionSize = 0;
  Dwarf_Half headerCuType = 0;
  ScopedDwarfError err{ctx.getDbg().get()};
  Dwarf_Die cuDie = nullptr;
  Dwarf_Die childDie = nullptr;
  Dwarf_Sig8 typeSignature;

  int rc = dwarf_next_cu_header_e(
      ctx.getDbg().get(), static_cast<Dwarf_Bool>(ctx.getIsInfo()), &cuDie,
      &cuHeaderLength, &versionStamp, &abbrevOffset, &addressSize, &lengthSize,
      &extensionSize, &typeSignature, &typeOffset, &nextCuHeader, &headerCuType,
      err.get());

  if (rc == DW_DLV_ERROR) return Unexpected("Error reading DWARF cu header\n");

  // Expect the CU to have a single DIE sibling
  rc = dwarf_siblingof_c(cuDie, &childDie, err.get());
  if (rc == DW_DLV_ERROR) {
    std::cout << DwarfContext::getError(err) << '\n';
    return Unexpected("Error getting sibling of CU\n");
  }
  if (rc == DW_DLV_OK) dwarf_dealloc_die(childDie);

  // Expect the CU DIE to have children
  rc = dwarf_child(cuDie, &childDie, err.get());
  if (rc == DW_DLV_ERROR) {
    std::cout << DwarfContext::getError(err) << '\n';
    return Unexpected("Error getting child of CU DIE\n");
  }

  dwarf_dealloc_die(cuDie);
  return childDie;
}

Expected<std::monostate, std::string> DwarfIndex::getSymbols(
    const DwarfContext& ctx, Dwarf_Die childDie) {
  while (true) {
    char* dieName = nullptr;
    Dwarf_Die siblingDie = nullptr;
    const char* tagName = nullptr;
    char* symName = nullptr;
    ScopedDwarfError err{ctx.getDbg().get()};
    Dwarf_Half tag = 0;
    int rc = dwarf_diename(childDie, &dieName, err.get());

    if (rc == DW_DLV_ERROR)
      return Unexpected("Error in dwarf_diename\n");
    else if (rc == DW_DLV_NO_ENTRY) {
      rc = dwarf_siblingof_c(childDie, &siblingDie, err.get());
      dwarf_dealloc_die(childDie);
      childDie = siblingDie;
      continue;
    }

    if (dwarf_tag(childDie, &tag, err.get()) != DW_DLV_OK)
      return Unexpected("Error in dwarf_tag\n");

    if (tag == DW_TAG_subprogram) {
      auto res = resolveFunctionFromTag(ctx, childDie, tag);
      if (res) m_symbols.emplace_back(std::move(res.value()));
    }

    rc = dwarf_siblingof_c(childDie, &siblingDie, err.get());
    dwarf_dealloc_die(childDie);

    if (rc == DW_DLV_ERROR)
      return Unexpected("Error getting sibling of DIE\n");
    else if (rc == DW_DLV_NO_ENTRY)
      break;

    childDie = siblingDie;
  }

  return std::monostate{};
}

Expected<SymInfo, std::string> DwarfIndex::resolveFunctionFromTag(
    const DwarfContext& ctx, Dwarf_Die& die, Dwarf_Half tag) {
  std::string typeName{};
  char* symName = nullptr;
  const char* tagName = nullptr;
  Dwarf_Attribute* rawAttrs = nullptr;
  ScopedDwarfError err{ctx.getDbg().get()};
  Dwarf_Addr lowpc = 0;
  Dwarf_Addr highpc = 0;
  Dwarf_Signed i = 0;
  Dwarf_Signed attrcount = 0;

  if (dwarf_get_TAG_name(tag, &tagName) != DW_DLV_OK)
    return Unexpected("Error in dwarf_get_TAG_name\n");

  if (dwarf_attrlist(die, &rawAttrs, &attrcount, err.get()) != DW_DLV_OK)
    return Unexpected("Error in dwarf_attlist\n");
  ScopedDwarfAttrList attrs{ctx.getDbg().get(), rawAttrs, attrcount};

  for (i = 0; i < attrs.size(); ++i) {
    Dwarf_Half attrcode = 0;
    if (dwarf_whatattr(attrs[i], &attrcode, err.get()) != DW_DLV_OK)
      return Unexpected("Error in dwarf_whatattr\n");

    // low_pc is always a relative address to the binary whilst high_pc is
    // an offset to low_pc
    if (attrcode == DW_AT_low_pc) {
      Dwarf_Half form = 0;
      if (dwarf_whatform(attrs[i], &form, err.get()) != DW_DLV_OK) {
        return Unexpected("Error in dwarf_whatform\n");
      }

      if (form == DW_FORM_addr || form == DW_FORM_addrx) {
        int res = dwarf_formaddr(attrs[i], &lowpc, err.get());
      }
    } else if (attrcode == DW_AT_high_pc) {
      Dwarf_Half form = 0;
      if (dwarf_whatform(attrs[i], &form, err.get()) != DW_DLV_OK)
        return Unexpected("Error in dwarf_whatform\n");

      if (form == DW_FORM_addr) {
        int res = dwarf_formaddr(attrs[i], &highpc, err.get());
      } else {
        Dwarf_Unsigned offset = 0;
        int res = dwarf_formudata(attrs[i], &offset, err.get());
        if (res != DW_DLV_OK) return Unexpected("Error in dwarf_formudata\n");
        highpc = lowpc + offset;
      }
    }

    if (attrcode == DW_AT_name) {
      Dwarf_Half form = 0;
      if (dwarf_whatform(attrs[i], &form, err.get()) != DW_DLV_OK)
        return Unexpected("Error in dwarf_whatform\n");
      if (form == DW_FORM_string || form == DW_FORM_strp ||
          form == DW_FORM_strp_sup || form == DW_FORM_line_strp ||
          form == DW_FORM_strx || form == DW_FORM_strx1 || DW_FORM_strx2 ||
          form == DW_FORM_strx3 || form == DW_FORM_strx4) {
        int res = dwarf_formstring(attrs[i], &symName, err.get());
        if (res != DW_DLV_OK) return Unexpected("Error in dwarf_formstring!\n");
      }
    }

    if (attrcode == DW_AT_type) {
      auto res = resolveFunctionTypeRef(ctx, die);
      if (res) typeName = res.value();
    }
  }


  return SymInfo{.m_name = std::string{std::format("{} {}", typeName, symName)},
                 .m_type = SymType::FUN,
                 .m_lowpc = lowpc,
                 .m_highpc = highpc};
}

Expected<std::string, std::string> DwarfIndex::resolveFunctionTypeRef(
    const DwarfContext& ctx, Dwarf_Die& die) {
  Dwarf_Die currentDie = die;
  std::string name;
  bool haveName = false;

  while (true) {
    ScopedDwarfError err{ctx.getDbg().get()};
    Dwarf_Attribute* rawAttrs = nullptr;
    Dwarf_Signed attrCount = 0;

    if (dwarf_attrlist(currentDie, &rawAttrs, &attrCount, err.get()) !=
        DW_DLV_OK)
      return Unexpected("Error in dwarf_attrlist\n");
    ScopedDwarfAttrList attrs{ctx.getDbg().get(), rawAttrs, attrCount};

    bool hasTypeRef = false;
    Dwarf_Off nextDieOffset = 0;
    Dwarf_Bool nextDieIsInfo = 0;

    for (Dwarf_Signed i = 0; i < attrs.size(); i++) {
      Dwarf_Half attrcode = 0;
      if (dwarf_whatattr(attrs[i], &attrcode, err.get()) != DW_DLV_OK)
        return Unexpected("Error in dwarf_whatattr\n");

      Dwarf_Half form = 0;
      if (dwarf_whatform(attrs[i], &form, err.get()) != DW_DLV_OK)
        return Unexpected("Error in dwarf_whatform\n");

      if (attrcode == DW_AT_name &&
          (form == DW_FORM_string || form == DW_FORM_strp ||
           form == DW_FORM_strp_sup || form == DW_FORM_line_strp ||
           form == DW_FORM_strx || form == DW_FORM_strx1 ||
           form == DW_FORM_strx2 || form == DW_FORM_strx3 ||
           form == DW_FORM_strx4)) {
        char* rawName = nullptr;
        if (dwarf_formstring(attrs[i], &rawName, err.get()) != DW_DLV_OK)
          return Unexpected("Error in dwarf_formstring!\n");
        name = rawName;
        haveName = true;
      }

      if (attrcode == DW_AT_type) {
        hasTypeRef = true;
        if (dwarf_global_formref_b(attrs[i], &nextDieOffset, &nextDieIsInfo,
                                   err.get()) != DW_DLV_OK)
          return Unexpected("Error in dwarf_global_formref_b\n");
      }
    }

    // No further DW_AT_type ref; currentDie is the base type
    if (!hasTypeRef) break;

    Dwarf_Die nextDie = nullptr;
    if (dwarf_offdie_b(ctx.getDbg().get(), nextDieOffset, nextDieIsInfo,
                       &nextDie, err.get()) != DW_DLV_OK)
      return Unexpected("Error in dwarf_offdie_b\n");

    if (currentDie != die) dwarf_dealloc_die(currentDie);
    currentDie = nextDie;
  }

  if (currentDie != die) dwarf_dealloc_die(currentDie);
  if (!haveName) return Unexpected("Type DIE has no name\n");
  return name;
}
