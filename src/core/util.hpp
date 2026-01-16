#ifndef CAESAR_UTIL_HPP
#define CAESAR_UTIL_HPP

#include <mach-o/loader.h>

#include <array>
#include <cstddef>
#include <cstdint>

template <typename T>
struct SwapDescriptor {
  static void swap(T* ptr) {
    static_assert(sizeof(T) % sizeof(uint32_t) == 0,
                  "Struct size must be multiple of 4 bytes");

    auto* fields = reinterpret_cast<uint32_t*>(ptr);
    constexpr size_t numFields = sizeof(T) / sizeof(uint32_t);

    // TODO: Compiler builtin, make this more platform agnostic
    for (size_t i = 0; i < numFields; i++) {
      fields[i] = __builtin_bswap32(fields[i]);
    }
  }
};

struct FieldInfo {
  size_t offset;
  size_t size;
  bool swap;
};

template <>
struct SwapDescriptor<segment_command_64> {
  static void swap(segment_command_64* segment) {
    char* base = reinterpret_cast<char*>(segment);

    constexpr std::array<FieldInfo, 11> fields = {
        {{.offset = offsetof(segment_command_64, cmd), .size = 4, .swap = true},
         {.offset = offsetof(segment_command_64, cmdsize),
          .size = 4,
          .swap = true},
         {.offset = offsetof(segment_command_64, segname),
          .size = 16,
          .swap = false},  // Skip char array
         {.offset = offsetof(segment_command_64, vmaddr),
          .size = 8,
          .swap = true},
         {.offset = offsetof(segment_command_64, vmsize),
          .size = 8,
          .swap = true},
         {.offset = offsetof(segment_command_64, fileoff),
          .size = 8,
          .swap = true},
         {.offset = offsetof(segment_command_64, filesize),
          .size = 8,
          .swap = true},
         {.offset = offsetof(segment_command_64, maxprot),
          .size = 4,
          .swap = true},
         {.offset = offsetof(segment_command_64, initprot),
          .size = 4,
          .swap = true},
         {.offset = offsetof(segment_command_64, nsects),
          .size = 4,
          .swap = true},
         {.offset = offsetof(segment_command_64, flags),
          .size = 4,
          .swap = true}}};

    for (const auto& f : fields) {
      if (f.swap) {
        if (f.size == 4) {
          auto* ptr = reinterpret_cast<uint32_t*>(base + f.offset);
          *ptr = __builtin_bswap32(*ptr);
        } else if (f.size == 8) {
          auto* ptr = reinterpret_cast<uint64_t*>(base + f.offset);
          *ptr = __builtin_bswap64(*ptr);
        }
      }
    }
  }
};

template <>
struct SwapDescriptor<section_64> {
  static void swap(section_64* section) {
    char* base = reinterpret_cast<char*>(section);

    constexpr std::array<FieldInfo, 12> fields = {
        {{.offset = offsetof(section_64, sectname), .size = 16, .swap = false},
         {.offset = offsetof(section_64, segname), .size = 16, .swap = false},
         {.offset = offsetof(section_64, addr), .size = 8, .swap = true},
         {.offset = offsetof(section_64, size), .size = 8, .swap = true},
         {.offset = offsetof(section_64, offset), .size = 4, .swap = true},
         {.offset = offsetof(section_64, align), .size = 4, .swap = true},
         {.offset = offsetof(section_64, reloff), .size = 4, .swap = true},
         {.offset = offsetof(section_64, nreloc), .size = 4, .swap = true},
         {.offset = offsetof(section_64, flags), .size = 4, .swap = true},
         {.offset = offsetof(section_64, reserved1), .size = 4, .swap = true},
         {.offset = offsetof(section_64, reserved2), .size = 4, .swap = true},
         {.offset = offsetof(section_64, reserved3), .size = 4, .swap = true}}};

    for (const auto& f : fields) {
      if (f.swap) {
        if (f.size == 4) {
          auto* ptr = reinterpret_cast<uint32_t*>(base + f.offset);
          *ptr = __builtin_bswap32(*ptr);
        } else if (f.size == 8) {
          auto* ptr = reinterpret_cast<uint64_t*>(base + f.offset);
          *ptr = __builtin_bswap64(*ptr);
        }
      }
    }
  }
};

#endif
