#ifndef CAESAR_PLATFORM_H
#define CAESAR_PLATFORM_H

#include <typedefs.hpp>

enum class PlatformType : u8 { MACH, LINUX, WIN, UNSUPPORTED };

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

#endif
