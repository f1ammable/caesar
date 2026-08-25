#ifndef CAESAR_DWARF_ALLOC_HPP
#define CAESAR_DWARF_ALLOC_HPP

#include <libdwarf.h>

#include <memory>

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

#endif
