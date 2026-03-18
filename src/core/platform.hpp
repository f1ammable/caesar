#ifndef CAESAR_PLATFORM_H
#define CAESAR_PLATFORM_H

#include <typedefs.hpp>

enum class PlatformType : u8 { MACH, LINUX, WIN, UNSUPPORTED };
enum class Architecture : u8 { ARM64, X86_64 };

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

constexpr Architecture getArchitecture() {
#ifdef __arm64__
  return Architecture::ARM64;
#endif

// TODO: Check if this def is 32-bit only or not
#ifdef __i386__
  return Architecture::X86_64;
#endif
}

#endif
