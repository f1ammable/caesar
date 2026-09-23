#include "die.hpp"

#include <dwarf.h>
#include <libdwarf.h>

#include <iostream>

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

      if (index.getFunctionSymbols(ctx, cuDie)) {
        std::ranges::sort(index.m_fns, {}, &FnInfo::m_lowpc);
        for (std::size_t i = 0; i < index.m_fns.size(); i++) {
          index.m_fns_name[index.m_fns[i].m_name] = i;
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

Expected<std::monostate, std::string> DwarfIndex::getFunctionSymbols(
    const DwarfContext& ctx, Dwarf_Die childDie) {
  while (true) {
    char* dieName = nullptr;
    Dwarf_Die siblingDie = nullptr;
    const char* tagName = nullptr;
    char* symName = nullptr;
    ScopedDwarfError err{ctx.getDbg().get()};
    Dwarf_Half tag = 0;
    Dwarf_Attribute* attrs = nullptr;
    Dwarf_Addr lowpc = 0;
    Dwarf_Addr highpc = 0;
    Dwarf_Signed attrcount = 0;
    Dwarf_Signed i = 0;
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

    // TODO: Generalise this function to accept any `DW_TAG_*` as an argument
    // and capture those
    if (tag == DW_TAG_subprogram) {
      if (dwarf_get_TAG_name(tag, &tagName) != DW_DLV_OK)
        return Unexpected("Error in dwarf_get_TAG_name\n");

      if (dwarf_attrlist(childDie, &attrs, &attrcount, err.get()) != DW_DLV_OK)
        return Unexpected("Error in dwarf_attlist\n");

      for (i = 0; i < attrcount; ++i) {
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
            if (res != DW_DLV_OK)
              return Unexpected("Error in dwarf_formudata\n");
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
            if (res != DW_DLV_OK)
              return Unexpected("Error in dwarf_formstring!\n");
          }
        }
      }

      m_fns.emplace_back(FnInfo{
          .m_name = std::string(symName),
          .m_lowpc = lowpc,
          .m_highpc = highpc,
      });
    }

    // Cleanup
    for (i = 0; i < attrcount; ++i) {
      Dwarf_Half attrcode = 0;
      if (dwarf_whatattr(attrs[i], &attrcode, err.get()) != DW_DLV_OK)
        return Unexpected("Error in dwarf_whatattr\n");
      if (attrcode == DW_AT_name) continue;
      dwarf_dealloc_attribute(attrs[i]);
    }
    dwarf_dealloc(ctx.getDbg().get(), static_cast<void*>(attrs), DW_DLA_LIST);

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
