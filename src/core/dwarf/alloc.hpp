#ifndef CAESAR_DWARF_ALLOC_HPP
#define CAESAR_DWARF_ALLOC_HPP

#include <libdwarf.h>

#include <memory>

struct IDwarfDeleter {
  virtual void operator()(void* ptr) const = 0;
  virtual ~IDwarfDeleter() = default;
};

struct DwarfDebugDeleter : public IDwarfDeleter {
  virtual void operator()(void* ptr) const override {
    if (ptr) dwarf_finish(static_cast<Dwarf_Debug>(ptr));
  };
};

// RAII wrapper for Dwarf_Error
class ScopedDwarfError {
  Dwarf_Error* m_err_ptr;
  std::shared_ptr<Dwarf_Debug_s> m_dbg;

 public:
  explicit ScopedDwarfError(Dwarf_Error* err_ptr,
                            std::shared_ptr<Dwarf_Debug_s> dbg)
      : m_err_ptr(err_ptr), m_dbg(std::move(dbg)) {
    *m_err_ptr = nullptr;
  }

  ~ScopedDwarfError() {
    if (*m_err_ptr && m_dbg) {
      dwarf_dealloc_error(m_dbg.get(), *m_err_ptr);
    }
  }

  ScopedDwarfError(const ScopedDwarfError&) = delete;
  ScopedDwarfError& operator=(const ScopedDwarfError&) = delete;

  Dwarf_Error* get() { return m_err_ptr; }
  operator Dwarf_Error*() { return m_err_ptr; }
};

#endif
