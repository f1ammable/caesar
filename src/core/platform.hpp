#ifndef CAESAR_PLATFORM_H
#define CAESAR_PLATFORM_H

#include <core/macho/types.hpp>
#include <cstddef>
#include <typedefs.hpp>
#include <unordered_map>

#include "expected.hpp"

enum class Platform : u8 { MACH, LINUX, WIN };
enum class Architecture : u8 { ARM64, X86_64, X86 };

enum class Arm64Reg : u8 {
  // General purpose
  X0,
  X1,
  X2,
  X3,
  X4,
  X5,
  X6,
  X7,
  X8,
  X9,
  X10,
  X11,
  X12,
  X13,
  X14,
  X15,
  X16,
  X17,
  X18,
  X19,
  X20,
  X21,
  X22,
  X23,
  X24,
  X25,
  X26,
  X27,
  X28,
  // Named aliases
  FP,  // x29
  LR,  // x30
  // Special registers
  SP,
  PC,
  CPSR,
  COUNT
};

enum class X86Reg : u8 {
  // General purpose (64-bit)
  RAX,
  RBX,
  RCX,
  RDX,
  RSI,
  RDI,
  RBP,
  RSP,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
  // Instruction pointer & flags
  RIP,
  RFLAGS,
  // Segment registers
  CS,
  SS,
  DS,
  ES,
  FS,
  GS,
  COUNT
};

constexpr static Platform getPlatform() {
  Platform t{};
#ifdef __APPLE__
  t = Platform::MACH;
#elif defined(__linux__)
  t = PlatformType::LINUX;
#elif defined(_WIN32)
  t = PlatformType::WIN;
#else
  static_asssert(false, "Unsupported platform!");
#endif
  return t;
}

constexpr std::string_view getTargetType() {
  constexpr Platform p = getPlatform();
  switch (p) {
    case Platform::MACH:
      return "Mach-O";
    case Platform::LINUX:
      return "ELF";
    case Platform::WIN:
      return "PE";
  }
}

constexpr Architecture getArchitecture() {
#if defined(__arm64__) || defined(__aarch64__)
  return Architecture::ARM64;
#elif defined(__x86_64__)
  return Architecture::X86_64;
#elif defined(__i386__)
  return Architecture::X86;
#else
  static_assert(false, "Unsupported architecture");
#endif
}

template <typename RegEnum>
struct RegEntry {
  RegEnum reg;
  std::ptrdiff_t offset;
  u8 size;
};

template <Architecture arch, Platform platform>
struct PlatformTraits;

struct StringHash {
  using is_transparent = void;
  [[nodiscard]] size_t operator()(std::string_view sv) const {
    return std::hash<std::string_view>{}(sv);
  }
};

struct StringEqual {
  using is_transparent = void;
  [[nodiscard]] bool operator()(std::string_view lhs,
                                std::string_view rhs) const {
    return lhs == rhs;
  }
};

template <typename RegEnum>
using RegMap = std::unordered_map<std::string_view, RegEntry<RegEnum>,
                                  StringHash, StringEqual>;

template <>
struct PlatformTraits<Architecture::ARM64, Platform::MACH> {
  using ThreadState = ArmThreadState64T;
  using RegEnum = Arm64Reg;
  using Entry = RegEntry<RegEnum>;
  using Map = RegMap<RegEnum>;

  static inline const Map REG_MAP = {
      {"x0",
       {.reg = Arm64Reg::X0,
        .offset = offsetof(ThreadState, x) + (0 * sizeof(u64)),
        .size = 8}},
      {"x1",
       {.reg = Arm64Reg::X1,
        .offset = offsetof(ThreadState, x) + (1 * sizeof(u64)),
        .size = 8}},
      {"x2",
       {.reg = Arm64Reg::X2,
        .offset = offsetof(ThreadState, x) + (2 * sizeof(u64)),
        .size = 8}},
      {"x3",
       {.reg = Arm64Reg::X3,
        .offset = offsetof(ThreadState, x) + (3 * sizeof(u64)),
        .size = 8}},
      {"x4",
       {.reg = Arm64Reg::X4,
        .offset = offsetof(ThreadState, x) + (4 * sizeof(u64)),
        .size = 8}},
      {"x5",
       {.reg = Arm64Reg::X5,
        .offset = offsetof(ThreadState, x) + (5 * sizeof(u64)),
        .size = 8}},
      {"x6",
       {.reg = Arm64Reg::X6,
        .offset = offsetof(ThreadState, x) + (6 * sizeof(u64)),
        .size = 8}},
      {"x7",
       {.reg = Arm64Reg::X7,
        .offset = offsetof(ThreadState, x) + (7 * sizeof(u64)),
        .size = 8}},
      {"x8",
       {.reg = Arm64Reg::X8,
        .offset = offsetof(ThreadState, x) + (8 * sizeof(u64)),
        .size = 8}},
      {"x9",
       {.reg = Arm64Reg::X9,
        .offset = offsetof(ThreadState, x) + (9 * sizeof(u64)),
        .size = 8}},
      {"x10",
       {.reg = Arm64Reg::X10,
        .offset = offsetof(ThreadState, x) + (10 * sizeof(u64)),
        .size = 8}},
      {"x11",
       {.reg = Arm64Reg::X11,
        .offset = offsetof(ThreadState, x) + (11 * sizeof(u64)),
        .size = 8}},
      {"x12",
       {.reg = Arm64Reg::X12,
        .offset = offsetof(ThreadState, x) + (12 * sizeof(u64)),
        .size = 8}},
      {"x13",
       {.reg = Arm64Reg::X13,
        .offset = offsetof(ThreadState, x) + (13 * sizeof(u64)),
        .size = 8}},
      {"x14",
       {.reg = Arm64Reg::X14,
        .offset = offsetof(ThreadState, x) + (14 * sizeof(u64)),
        .size = 8}},
      {"x15",
       {.reg = Arm64Reg::X15,
        .offset = offsetof(ThreadState, x) + (15 * sizeof(u64)),
        .size = 8}},
      {"x16",
       {.reg = Arm64Reg::X16,
        .offset = offsetof(ThreadState, x) + (16 * sizeof(u64)),
        .size = 8}},
      {"x17",
       {.reg = Arm64Reg::X17,
        .offset = offsetof(ThreadState, x) + (17 * sizeof(u64)),
        .size = 8}},
      {"x18",
       {.reg = Arm64Reg::X18,
        .offset = offsetof(ThreadState, x) + (18 * sizeof(u64)),
        .size = 8}},
      {"x19",
       {.reg = Arm64Reg::X19,
        .offset = offsetof(ThreadState, x) + (19 * sizeof(u64)),
        .size = 8}},
      {"x20",
       {.reg = Arm64Reg::X20,
        .offset = offsetof(ThreadState, x) + (20 * sizeof(u64)),
        .size = 8}},
      {"x21",
       {.reg = Arm64Reg::X21,
        .offset = offsetof(ThreadState, x) + (21 * sizeof(u64)),
        .size = 8}},
      {"x22",
       {.reg = Arm64Reg::X22,
        .offset = offsetof(ThreadState, x) + (22 * sizeof(u64)),
        .size = 8}},
      {"x23",
       {.reg = Arm64Reg::X23,
        .offset = offsetof(ThreadState, x) + (23 * sizeof(u64)),
        .size = 8}},
      {"x24",
       {.reg = Arm64Reg::X24,
        .offset = offsetof(ThreadState, x) + (24 * sizeof(u64)),
        .size = 8}},
      {"x25",
       {.reg = Arm64Reg::X25,
        .offset = offsetof(ThreadState, x) + (25 * sizeof(u64)),
        .size = 8}},
      {"x26",
       {.reg = Arm64Reg::X26,
        .offset = offsetof(ThreadState, x) + (26 * sizeof(u64)),
        .size = 8}},
      {"x27",
       {.reg = Arm64Reg::X27,
        .offset = offsetof(ThreadState, x) + (27 * sizeof(u64)),
        .size = 8}},
      {"x28",
       {.reg = Arm64Reg::X28,
        .offset = offsetof(ThreadState, x) + (28 * sizeof(u64)),
        .size = 8}},
      {"fp",
       {.reg = Arm64Reg::FP, .offset = offsetof(ThreadState, fp), .size = 8}},
      {"x29",
       {.reg = Arm64Reg::FP, .offset = offsetof(ThreadState, fp), .size = 8}},
      {"lr",
       {.reg = Arm64Reg::LR, .offset = offsetof(ThreadState, lr), .size = 8}},
      {"x30",
       {.reg = Arm64Reg::LR, .offset = offsetof(ThreadState, lr), .size = 8}},
      {"sp",
       {.reg = Arm64Reg::SP, .offset = offsetof(ThreadState, sp), .size = 8}},
      {"pc",
       {.reg = Arm64Reg::PC, .offset = offsetof(ThreadState, pc), .size = 8}},
      {"cpsr",
       {.reg = Arm64Reg::CPSR,
        .offset = offsetof(ThreadState, cpsr),
        .size = 4}},
  };
};

using CurrentPlatform = PlatformTraits<getArchitecture(), getPlatform()>;
using ThreadState = CurrentPlatform::ThreadState;
using Reg = CurrentPlatform::RegEnum;
using RegEntryT = CurrentPlatform::Entry;
inline constexpr auto& regMap = CurrentPlatform::REG_MAP;

inline u64 readRegValue(const ThreadState& threadState,
                        const RegEntryT& regEntry) {
  const auto* ptr = reinterpret_cast<const u8*>(&threadState) + regEntry.offset;
  return regEntry.size == 8 ? *reinterpret_cast<const u64*>(ptr)
                            : *reinterpret_cast<const u32*>(ptr);
}

inline Expected<const RegEntryT*, std::string> findRegEntry(
    std::string_view name) {
  auto it = regMap.find(name);
  if (it != regMap.end()) return &it->second;
  return Unexpected{std::format("Unknown register: {}", name)};
}

#endif
