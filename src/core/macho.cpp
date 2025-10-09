#include "macho.hpp"

#include <architecture/byte_order.h>
#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <mach/machine.h>

#include <cstring>
#include <iostream>

// Adapted from https://lowlevelbits.org/parsing-mach-o-files/

void Macho::dump() { dump_mach_header(0); }

void Macho::read_magic(int offset) {
  uint32_t magic;
  this->file.seekg(offset, std::ios::beg);
  this->file.read(std::bit_cast<char*>(&magic), sizeof(uint32_t));
  this->magic = magic;
}

void Macho::is_magic_64() {
  if (magic == MH_MAGIC_64 || magic == MH_CIGAM_64) this->is_64 = true;
}

void Macho::maybe_swap_bytes() {
  if (magic == MH_CIGAM || magic == MH_CIGAM_64) this->is_swap = true;
}

void Macho::dump_mach_header(int offset) {
  uint32_t ncmds;
  int load_cmds_offset = offset;

  if (this->is_64) {
    constexpr size_t header_size = sizeof(struct mach_header_64);
    auto header = this->load_bytes_and_maybe_swap<mach_header_64>(offset);
    ncmds = header->ncmds;
    load_cmds_offset += header_size;
    std::cout << this->cpu_type_name(header->cputype) << std::endl;
  } else {
    int header_size = sizeof(struct mach_header);
    auto header = this->load_bytes_and_maybe_swap<mach_header>(offset);
  }

  dump_segment_commands(load_cmds_offset, ncmds);
}

void Macho::dump_segment_commands(int offset, uint32_t ncmds) {
  int actual_offset = offset;
  for (int i = 0; i < ncmds; i++) {
    auto cmd = this->load_bytes_and_maybe_swap<load_command>(actual_offset);
    if (cmd->cmd == LC_SEGMENT_64) {
      auto segment =
          this->load_bytes_and_maybe_swap<segment_command_64>(actual_offset);
      std::cout << std::format(
                       "segname: {:<25} offset: 0x{:<12x} vmaddr: 0x{:<18x} "
                       "vmsize: 0x{:x}",
                       segment->segname, segment->fileoff, segment->vmaddr,
                       segment->vmsize)
                << std::endl;
      dump_sections(actual_offset + sizeof(segment_command_64),
                    actual_offset + cmd->cmdsize);
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

void Macho::dump_sections(int offset, int end) {
  int actual_offset = offset;
  while (actual_offset != end) {
    auto section = this->load_bytes_and_maybe_swap<section_64>(actual_offset);
    std::cout << std::format("Section: {}; Address: 0x{:x}", section->sectname,
                             section->addr)
              << std::endl;
    actual_offset += sizeof(section_64);
  }
}

Macho::Macho(std::ifstream& f) : file(f) {
  read_magic(0);
  is_magic_64();
  maybe_swap_bytes();
}
