#ifndef CAESAR_MACHO_HPP
#define CAESAR_MACHO_HPP

#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <mach/machine.h>

#include <fstream>
#include <string>

#include "util.hpp"
#include "target.hpp"

struct CpuTypeNames {
  cpu_type_t cpu_type;
  const char* cpu_name;
};

class Macho : public Target {
 public:
  explicit Macho(std::ifstream f);
  void dump() override;

 private:
  uint32_t m_magic = 0;
  bool m_is_64 = false;
  bool m_is_swap = false;

  void readMagic() override;
  void is64() override;
  void maybeSwapBytes();

  template <typename T>
  T loadBytesAndMaybeSwap(uint32_t offset) {
    T buf;
    m_file->seekg(offset, std::ios::beg);
    m_file->read(std::bit_cast<char*>(&buf), sizeof(T));
    if (m_is_swap) SwapDescriptor<T>::swap(&buf);
    return buf;
  }

  void dumpMachHeader(int offset);
  void dumpSegmentCommands(int offset, uint32_t ncmds);

  static constexpr std::array<CpuTypeNames, 4> CPU_TYPE_NAMES = {
      {{.cpu_type = CPU_TYPE_I386, .cpu_name = "i386"},
       {.cpu_type = CPU_TYPE_X86_64, .cpu_name = "x86_64"},
       {.cpu_type = CPU_TYPE_ARM, .cpu_name = "arm"},
       {.cpu_type = CPU_TYPE_ARM64, .cpu_name = "arm64"}}};
  static std::string cpuTypeName(cpu_type_t cpuType);
  void dumpSections(uint32_t offset, uint32_t end);
};

#endif  // CAESAR_MACHO_HPP
