#include "macho.hpp"

#include <architecture/byte_order.h>
#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <mach/machine.h>

#include <iostream>

// Adapted from https://lowlevelbits.org/parsing-mach-o-files/

void Macho::dump_segments(std::ifstream& file) {
  uint32_t magic = read_magic(file, 0);
  int is_64 = is_magic_64(magic);
  int is_swap = should_swap_bytes(magic);
  dump_mach_header(file, 0, is_64, is_swap);
}

uint32_t Macho::read_magic(std::ifstream& file, int offset) {
  uint32_t magic;
  file.seekg(offset, std::ios::beg);
  file.read(std::bit_cast<char*>(&magic), sizeof(uint32_t));
  return magic;
}

int Macho::is_magic_64(uint32_t magic) {
  return magic == MH_MAGIC_64 || magic == MH_CIGAM_64;
}

int Macho::should_swap_bytes(uint32_t magic) {
  return magic == MH_CIGAM || magic == MH_CIGAM_64;
}

void Macho::dump_mach_header(std::ifstream& file, int offset, int is_64,
                             int is_swap) {
  uint32_t ncmds;
  int load_cmds_offset = offset;

  if (is_64) {
    constexpr size_t header_size = sizeof(struct mach_header_64);
    auto header = load_bytes<mach_header_64>(file, offset);
    if (is_swap) {
      swap_mach_header_64(header.get(), (NXByteOrder)0);
    }

    ncmds = header->ncmds;
    load_cmds_offset += header_size;

    std::cout << cpu_type_name(header->cputype) << std::endl;

  } else {
    int header_size = sizeof(struct mach_header);
    auto header = load_bytes<mach_header>(file, offset);
    if (is_swap) {
      swap_mach_header(header.get(), (NXByteOrder)0);
    }
  }

  dump_segment_commands(file, load_cmds_offset, is_swap, ncmds);
}

void Macho::dump_segment_commands(std::ifstream& file, int offset, int is_swap,
                                  uint32_t ncmds) {
  int actual_offset = offset;
  for (int i = 0; i < ncmds; i++) {
    auto cmd = load_bytes<load_command>(file, actual_offset);
    if (is_swap) swap_load_command(cmd.get(), (NXByteOrder)0);
    if (cmd->cmd == LC_SEGMENT_64) {
      auto segment = load_bytes<segment_command_64>(file, actual_offset);
      if (is_swap) swap_segment_command_64(segment.get(), (NXByteOrder)0);
      std::cout << std::format("segname: {}", segment->segname) << std::endl;
    }
    actual_offset += cmd->cmdsize;
  }
}

std::string Macho::cpu_type_name(cpu_type_t cpu_type) {
  for (const auto x : cpu_type_names) {
    if (cpu_type == x.cpu_type) return x.cpu_name;
  }
  return "unknown";
}
