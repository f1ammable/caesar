#ifndef CAESAR_DWARF_READER_HPP
#define CAESAR_DWARF_READER_HPP

#include <libdwarf.h>

class dwarf {
 private:
  Dwarf_Debug dbg;

 public:
  void read_cu_list();
};

#endif
