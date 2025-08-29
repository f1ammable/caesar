#ifndef CAESAR_UTIL_HPP
#define CAESAR_UTIL_HPP

#include <mach-o/loader.h>

#include <cstddef>
#include <cstdint>

template <typename T>
struct SwapDescriptor {
  static void swap(T* ptr) {
    static_assert(sizeof(T) % sizeof(uint32_t) == 0,
                  "Struct size must be multiple of 4 bytes");

    uint32_t* fields = reinterpret_cast<uint32_t*>(ptr);
    constexpr size_t num_fields = sizeof(T) / sizeof(uint32_t);

    for (size_t i = 0; i < num_fields; i++) {
      fields[i] = __builtin_bswap32(fields[i]);
    }
  }
};

template <>
struct SwapDescriptor<segment_command_64> {
  static void swap(segment_command_64* segment) {
    char* base = reinterpret_cast<char*>(segment);

    struct FieldInfo {
      size_t offset;
      size_t size;
      bool swap;
    };

    constexpr FieldInfo fields[] = {
        {offsetof(segment_command_64, cmd), 4, true},
        {offsetof(segment_command_64, cmdsize), 4, true},
        {offsetof(segment_command_64, segname), 16, false},  // Skip char array
        {offsetof(segment_command_64, vmaddr), 8, true},
        {offsetof(segment_command_64, vmsize), 8, true},
        {offsetof(segment_command_64, fileoff), 8, true},
        {offsetof(segment_command_64, filesize), 8, true},
        {offsetof(segment_command_64, maxprot), 4, true},
        {offsetof(segment_command_64, initprot), 4, true},
        {offsetof(segment_command_64, nsects), 4, true},
        {offsetof(segment_command_64, flags), 4, true}};

    for (const auto& f : fields) {
      if (f.swap) {
        if (f.size == 4) {
          uint32_t* ptr = reinterpret_cast<uint32_t*>(base + f.offset);
          *ptr = __builtin_bswap32(*ptr);
        } else if (f.size == 8) {
          uint64_t* ptr = reinterpret_cast<uint64_t*>(base + f.offset);
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

    struct FieldInfo {
      size_t offset;
      size_t size;
      bool swap;
    };

    constexpr FieldInfo fields[] = {
        {offsetof(section_64, sectname), 16, false},
        {offsetof(section_64, segname), 16, false},
        {offsetof(section_64, addr), 8, true},
        {offsetof(section_64, size), 8, true},
        {offsetof(section_64, offset), 4, true},
        {offsetof(section_64, align), 4, true},
        {offsetof(section_64, reloff), 4, true},
        {offsetof(section_64, nreloc), 4, true},
        {offsetof(section_64, flags), 4, true},
        {offsetof(section_64, reserved1), 4, true},
        {offsetof(section_64, reserved2), 4, true},
        {offsetof(section_64, reserved3), 4, true}
    };

    for (const auto& f : fields) {
      if (f.swap) {
        if (f.size == 4) {
          uint32_t* ptr = reinterpret_cast<uint32_t*>(base + f.offset);
          *ptr = __builtin_bswap32(*ptr);
        } else if (f.size == 8) {
          uint64_t* ptr = reinterpret_cast<uint64_t*>(base + f.offset);
          *ptr = __builtin_bswap64(*ptr);
        }
      }
    }
  }
};

#endif
