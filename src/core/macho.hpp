#ifndef CAESAR_MACHO_HPP
#define CAESAR_MACHO_HPP

#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <mach/machine.h>

#include <fstream>
#include <string>

#include "util.hpp"

struct _cpu_type_names {
  cpu_type_t cpu_type;
  std::string cpu_name;
};

class Macho {
 public:
  explicit Macho(std::ifstream& f);
  void dump();

 private:
  uint32_t magic;
  bool is_64;
  bool is_swap;
  std::ifstream& file;

  void read_magic(int offset);
  void is_magic_64();
  void maybe_swap_bytes();

  template <typename T>
  std::unique_ptr<T> load_bytes_and_maybe_swap(int offset) {
    auto buf = std::make_unique<T>();
    this->file.seekg(offset, std::ios::beg);
    this->file.read(std::bit_cast<char*>(buf.get()), sizeof(T));
    if (this->is_swap) SwapDescriptor<T>::swap(buf.get());
    return buf;
  }

  void dump_mach_header(int offset);
  void dump_segment_commands(int offset, uint32_t ncmds);
  static constexpr _cpu_type_names cpu_type_names[] = {
      {CPU_TYPE_I386, "i386"},
      {CPU_TYPE_X86_64, "x86_64"},
      {CPU_TYPE_ARM, "arm"},
      {CPU_TYPE_ARM64, "arm64"}};
  std::string cpu_type_name(cpu_type_t cpu_type);
  void dump_sections(int offset, int end);
};

#endif  // CAESAR_MACHO_HPP
