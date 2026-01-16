#include "macho.hpp"

#include <architecture/byte_order.h>
#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <mach/machine.h>

#include <cstring>
#include <iostream>

// Adapted from https://lowlevelbits.org/parsing-mach-o-files/

void Macho::dump() { dumpMachHeader(0); }

void Macho::readMagic() {
  uint32_t magic = 0;
  m_file->seekg(0, std::ios::beg);
  m_file->read(std::bit_cast<char*>(&magic), sizeof(uint32_t));
  m_magic = magic;
}

void Macho::is64() {
  if (m_magic == MH_MAGIC_64 || m_magic == MH_CIGAM_64) m_is_64 = true;
}

void Macho::maybeSwapBytes() {
  if (m_magic == MH_CIGAM || m_magic == MH_CIGAM_64) m_is_swap = true;
}

void Macho::dumpMachHeader(int offset) {
  uint32_t ncmds = 0;
  int loadCmdsOffset = offset;

  if (m_is_64) {
    constexpr size_t headerSize = sizeof(struct mach_header_64);
    auto header = loadBytesAndMaybeSwap<mach_header_64>(offset);
    ncmds = header.ncmds;
    loadCmdsOffset += headerSize;
    std::cout << Macho::cpuTypeName(header.cputype) << '\n';
  } else {
    int headerSize = sizeof(struct mach_header);
    auto header = loadBytesAndMaybeSwap<mach_header>(offset);
  }

  dumpSegmentCommands(loadCmdsOffset, ncmds);
}

void Macho::dumpSegmentCommands(int offset, uint32_t ncmds) {
  uint32_t actualOffset = offset;
  for (int i = 0; i < ncmds; i++) {
    auto cmd = loadBytesAndMaybeSwap<load_command>(actualOffset);
    if (cmd.cmd == LC_SEGMENT_64) {
      auto segment = loadBytesAndMaybeSwap<segment_command_64>(actualOffset);
      std::cout << std::format(
                       "segname: {:<25} offset: 0x{:<12x} vmaddr: 0x{:<18x} "
                       "vmsize: 0x{:x}",
                       segment.segname, segment.fileoff, segment.vmaddr,
                       segment.vmsize)
                << '\n';
      dumpSections(actualOffset + sizeof(segment_command_64),
                   actualOffset + cmd.cmdsize);
    }
    actualOffset += cmd.cmdsize;
  }
}

std::string Macho::cpuTypeName(cpu_type_t cpuType) {
  for (const auto& x : CPU_TYPE_NAMES) {
    if (cpuType == x.cpu_type) return x.cpu_name;
  }
  return "unknown";
}

void Macho::dumpSections(uint32_t offset, uint32_t end) {
  uint32_t actualOffset = offset;
  while (actualOffset != end) {
    auto section = loadBytesAndMaybeSwap<section_64>(actualOffset);
    std::cout << std::format("Section: {}; Address: 0x{:x}", section.sectname,
                             section.addr)
              << '\n';
    actualOffset += sizeof(section_64);
  }
}

Macho::Macho(std::ifstream f) : Target(std::move(f)) {
  readMagic();
  is64();
  maybeSwapBytes();
}
