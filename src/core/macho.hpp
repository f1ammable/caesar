#ifndef CAESAR_MACHO_HPP
#define CAESAR_MACHO_HPP

#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <mach/machine.h>

#include <fstream>
#include <string>

struct _cpu_type_names {
  cpu_type_t cpu_type;
  std::string cpu_name;
};

class Macho {
 public:
  static void dump_segments(std::ifstream& file);

 private:
  static uint32_t read_magic(std::ifstream& file, int offset);
  static int is_magic_64(uint32_t magic);
  static int should_swap_bytes(uint32_t magic);

  template <typename T>
  static std::unique_ptr<T> load_bytes(std::ifstream& file, int offset) {
    auto buf = std::make_unique<T>();
    file.seekg(offset, std::ios::beg);
    file.read(std::bit_cast<char*>(buf.get()), sizeof(T));
    return buf;
  }

  static void dump_mach_header(std::ifstream& file, int offset, int is_64,
                               int is_swap);
  static void dump_segment_commands(std::ifstream& file, int offset,
                                    int is_swap, uint32_t ncmds);
  static constexpr _cpu_type_names cpu_type_names[] = {
      {CPU_TYPE_I386, "i386"},
      {CPU_TYPE_X86_64, "x86_64"},
      {CPU_TYPE_ARM, "arm"},
      {CPU_TYPE_ARM64, "arm64"}};
  static std::string cpu_type_name(cpu_type_t cpu_type);
};

#endif  // CAESAR_MACHO_HPP
