#ifndef CAESAR_DWARF_ALLOC_HPP
#define CAESAR_DWARF_ALLOC_HPP

#include <libdwarf.h>

#include <memory>
#include <vector>

struct IDwarfDeleter {
  virtual void operator()(void* ptr) const = 0;
  virtual ~IDwarfDeleter() = default;
};

template <auto Deleter>
struct DwarfDeleter {
  template <typename T>
  void operator()(T* ptr) const {
    if (ptr != nullptr) Deleter(ptr);
  };
};

using DwarfDebugDeleter = DwarfDeleter<dwarf_finish>;

// RAII wrapper for Dwarf_Error
class ScopedDwarfError {
  Dwarf_Error m_err = nullptr;
  Dwarf_Debug m_dbg;

 public:
  ScopedDwarfError(const ScopedDwarfError&) = default;
  ScopedDwarfError(ScopedDwarfError&&) = delete;
  ScopedDwarfError& operator=(const ScopedDwarfError&) = default;
  ScopedDwarfError& operator=(ScopedDwarfError&&) = delete;
  explicit ScopedDwarfError(Dwarf_Debug dbg) : m_dbg(dbg) {}
  ~ScopedDwarfError() {
    if (m_err != nullptr) dwarf_dealloc_error(m_dbg, m_err);
  }
  Dwarf_Error* get() { return &m_err; }
  [[nodiscard]] Dwarf_Error raw() const { return m_err; }
  explicit operator Dwarf_Error*() { return &m_err; }
};

// RAII wrapper for the Dwarf_Attribute list returned by dwarf_attrlist.
class ScopedDwarfAttrList {
  Dwarf_Debug m_dbg;
  Dwarf_Attribute* m_attrs;
  Dwarf_Signed m_count;
  std::vector<bool> m_keep;

 public:
  ScopedDwarfAttrList(const ScopedDwarfAttrList&) = delete;
  ScopedDwarfAttrList& operator=(const ScopedDwarfAttrList&) = delete;
  ScopedDwarfAttrList(ScopedDwarfAttrList&&) = delete;
  ScopedDwarfAttrList& operator=(ScopedDwarfAttrList&&) = delete;

  ScopedDwarfAttrList(Dwarf_Debug dbg, Dwarf_Attribute* attrs,
                      Dwarf_Signed count)
      : m_dbg(dbg),
        m_attrs(attrs),
        m_count(count),
        m_keep(static_cast<std::size_t>(count), false) {}

  ~ScopedDwarfAttrList() {
    for (Dwarf_Signed i = 0; i < m_count; ++i) {
      if (!m_keep[static_cast<std::size_t>(i)])
        dwarf_dealloc_attribute(m_attrs[i]);
    }
    dwarf_dealloc(m_dbg, static_cast<void*>(m_attrs), DW_DLA_LIST);
  }

  Dwarf_Attribute& operator[](Dwarf_Signed i) { return m_attrs[i]; }
  [[nodiscard]] Dwarf_Signed size() const { return m_count; }
  // Keep attrs[i] alive past this wrapper's destruction
  void keep(Dwarf_Signed i) { m_keep[static_cast<std::size_t>(i)] = true; }
};

#endif
