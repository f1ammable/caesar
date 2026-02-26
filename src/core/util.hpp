#ifndef CAESAR_UTIL_HPP
#define CAESAR_UTIL_HPP

#ifdef __APPLE__

#include <mach-o/loader.h>

#endif

#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <vector>

#include "expected.hpp"
#include "typedefs.hpp"

template <typename T>
struct SwapDescriptor {
  static void swap(T* ptr) {
    static_assert(sizeof(T) % sizeof(u32) == 0,
                  "Struct size must be multiple of 4 bytes");

    auto* fields = reinterpret_cast<u32*>(ptr);
    constexpr size_t numFields = sizeof(T) / sizeof(u32);

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

#ifdef __APPLE__

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
          auto* ptr = reinterpret_cast<u32*>(base + f.offset);
          *ptr = __builtin_bswap32(*ptr);
        } else if (f.size == 8) {
          auto* ptr = reinterpret_cast<u64*>(base + f.offset);
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
          auto* ptr = reinterpret_cast<u32*>(base + f.offset);
          *ptr = __builtin_bswap32(*ptr);
        } else if (f.size == 8) {
          auto* ptr = reinterpret_cast<u64*>(base + f.offset);
          *ptr = __builtin_bswap64(*ptr);
        }
      }
    }
  }
};

#endif

enum class PlatformType : std::uint8_t { MACH, LINUX, WIN, UNSUPPORTED };
constexpr static PlatformType getPlatform() {
  PlatformType t = PlatformType::UNSUPPORTED;
#ifdef __APPLE__
  t = PlatformType::MACH;
#elif defined(__linux__)
  t = PlatformType::LINUX;
#elif defined(_WIN32)
  t = PlatformType::LINUX;
#endif
  return t;
}

struct CStringArray {
  std::vector<std::vector<char>> storage;
  std::vector<char*> ptrs;

  char** data() { return ptrs.data(); }

  void prepend(const std::string& s) {
    storage.insert(storage.begin(), std::vector<char>(s.begin(), s.end()));
    storage.front().push_back('\0');
    rebuildPtrs();
  }

 private:
  void rebuildPtrs() {
    ptrs.clear();
    ptrs.reserve(storage.size() + 1);
    for (auto& s : storage) {
      ptrs.push_back(s.data());
    }
    ptrs.push_back(nullptr);
  }
};

inline std::string toHex(u64 addr) { return std::format("{:#018x}", addr); }

inline Expected<u64, std::string> strToAddr(const std::string& addr) {
  u64 res = 0;
  try {
    res = static_cast<u64>(std::stoull(addr, nullptr, 0));
    return res;
  } catch (std::invalid_argument& e) {
    return Unexpected{
        std::format("Could not convert from {} to address!", addr)};
  } catch (std::out_of_range& e) {
    return Unexpected{"Address provided is out of range!"};
  }
}

constexpr std::string_view getTargetType() {
  constexpr PlatformType p = getPlatform();
  switch (p) {
    case PlatformType::MACH:
      return "Mach-O";
    case PlatformType::LINUX:
      return "ELF";
    case PlatformType::WIN:
      return "PE";
  }
}

#endif
