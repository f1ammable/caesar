#include "context.hpp"

#include <dwarf.h>
#include <libdwarf.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include "alloc.hpp"

DwarfContext::DwarfContext(const std::string& path) {
  Dwarf_Debug raw_dbg = nullptr;
  Dwarf_Error raw_err = nullptr;
  int res = dwarf_init_path(path.c_str(), nullptr, 0, DW_GROUPNUMBER_ANY,
                            nullptr, nullptr, &raw_dbg, &raw_err);

  if (res != DW_DLV_OK) {
    dwarf_dealloc_error(raw_dbg, raw_err);
    throw std::runtime_error("Failed to initialise libdwarf");
  }

  m_dbg.reset(raw_dbg, DwarfDebugDeleter{});
}

void DwarfContext::err(const std::string& msg) {
  throw std::runtime_error(msg);
}

void DwarfContext::list_funcs_in_die(Dwarf_Debug dbg) {
  Dwarf_Unsigned cu_header_length, abbrev_offset, next_cu_header, type_offset;
  Dwarf_Half version_stamp, address_size, length_size, extension_size,
      header_cu_type;
  Dwarf_Error raw_err;
  ScopedDwarfError err(&raw_err, m_dbg);
  Dwarf_Die no_die = 0, cu_die, child_die;
  Dwarf_Sig8 type_signature;

  int rc = dwarf_next_cu_header_e(dbg, m_is_info, &cu_die, &cu_header_length,
                                  &version_stamp, &abbrev_offset, &address_size,
                                  &length_size, &extension_size,
                                  &type_signature, &type_offset,
                                  &next_cu_header, &header_cu_type, err.get());

  /* Find compilation unit header */
  if (rc == DW_DLV_ERROR) this->err("Error reading DWARF cu header\n");

  /* Expect the CU to have a single sibling - a DIE */
  rc = dwarf_siblingof_c(cu_die, &child_die, err.get());
  if (rc == DW_DLV_ERROR) {
    std::cout << dwarf_errmsg(raw_err) << std::endl;
    this->err("Error getting sibling of CU\n");
  }

  /* Expect the CU DIE to have children */
  rc = dwarf_child(cu_die, &child_die, err.get());
  if (rc == DW_DLV_ERROR) {
    std::cout << dwarf_errmsg(raw_err) << std::endl;
    this->err("Error getting child of CU DIE\n");
  }

  /* Now go over all children DIEs */
  while (1) {
    int rc;
    list_func_in_die(dbg, child_die);

    rc = dwarf_siblingof_c(child_die, &child_die, err.get());

    if (rc == DW_DLV_ERROR)
      this->err("Error getting sibling of DIE\n");
    else if (rc == DW_DLV_NO_ENTRY)
      break; /* done */
  }
}

void DwarfContext::list_func_in_die(Dwarf_Debug dbg, Dwarf_Die die) {
  char* die_name = 0;
  const char* tag_name = 0;
  Dwarf_Error raw_err;
  ScopedDwarfError err(&raw_err, m_dbg);
  Dwarf_Half tag;
  Dwarf_Attribute* attrs;
  Dwarf_Addr lowpc = 0, highpc = 0;
  Dwarf_Signed attrcount, i;
  int rc = dwarf_diename(die, &die_name, err.get());

  if (rc == DW_DLV_ERROR)
    this->err("Error in dwarf_diename\n");
  else if (rc == DW_DLV_NO_ENTRY)
    return;

  if (dwarf_tag(die, &tag, err.get()) != DW_DLV_OK)
    this->err("Error in dwarf_tag\n");

  /* Only interested in subprogram DIEs here */
  if (tag != DW_TAG_subprogram) return;

  if (dwarf_get_TAG_name(tag, &tag_name) != DW_DLV_OK)
    this->err("Error in dwarf_get_TAG_name\n");

  printf("DW_TAG_subprogram: '%s'\n", die_name);

  if (dwarf_attrlist(die, &attrs, &attrcount, err.get()) != DW_DLV_OK)
    this->err("Error in dwarf_attlist\n");

  for (i = 0; i < attrcount; ++i) {
    Dwarf_Half attrcode;
    if (dwarf_whatattr(attrs[i], &attrcode, err.get()) != DW_DLV_OK)
      this->err("Error in dwarf_whatattr\n");

    if (attrcode == DW_AT_low_pc) {
      Dwarf_Half form;
      if (dwarf_whatform(attrs[i], &form, err.get()) != DW_DLV_OK) {
        this->err("Error in dwarf_whatform\n");
      }

      // DW_FORM_addr (0x01) or DW_FORM_addrx (0x1b) - use dwarf_formaddr
      if (form == DW_FORM_addr || form == DW_FORM_addrx) {
        int res = dwarf_formaddr(attrs[i], &lowpc, err.get());
      } else {
        // DW_FORM_data4, DW_FORM_data8, etc.
        Dwarf_Unsigned udata;
        int res = dwarf_formudata(attrs[i], &udata, err.get());
        if (res == DW_DLV_OK) lowpc = udata;
      }
    } else if (attrcode == DW_AT_high_pc) {
      Dwarf_Half form;
      if (dwarf_whatform(attrs[i], &form, err.get()) != DW_DLV_OK)
        this->err("Error in dwarf_whatform\n");

      if (form == DW_FORM_addr) {
        int res = dwarf_formaddr(attrs[i], &highpc, err.get());
      } else {
        Dwarf_Unsigned offset;
        int res = dwarf_formudata(attrs[i], &offset, err.get());
        if (res == DW_DLV_OK) highpc = lowpc + offset;
      }
    }
  }

  printf("low pc  : 0x%08llx\n", lowpc);
  printf("high pc : 0x%08llx\n", highpc);

  /* Clean up attributes */
  for (i = 0; i < attrcount; ++i) {
    dwarf_dealloc_attribute(attrs[i]);
  }
  dwarf_dealloc(dbg, attrs, DW_DLA_LIST);
}
