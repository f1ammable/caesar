#ifndef CAESAR_CONTEXT_H
#define CAESAR_CONTEXT_H

#include <string>

enum class ProgramState : std::uint8_t { STOPPED, RUNNING };
enum class BinaryType : std::uint8_t { PE, ELF, MACHO };
enum class PlatformType : std::uint8_t { MACH, LINUX, WIN };
enum class FileError : std::uint8_t { FILE_NOT_FOUND, FILE_OK };

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Context {
  // TODO: Handle endianness differences between platform and binary
 public:
  consteval static PlatformType getPlatform() {
    PlatformType t;  // NOLINT(cppcoreguidelines-init-variables)
#if defined(__APPLE__)
    t = PlatformType::MACH;
#elif defined(__linux__)
    t = PlatformType::LINUX;
#elif defined(_WIN32)
    t = PlatformType::LINUX;
#endif
    return t;
  }

  Context() = default;
  Context(const Context&) = delete;
  Context(Context&&) = delete;
  Context& operator=(const Context&) = delete;
  Context& operator=(Context&&) = delete;

  static Context& getInstance() {
    static Context instance;
    return instance;
  }

  std::string m_loaded_file;
  bool m_is_running = false;
  ProgramState m_state = ProgramState::STOPPED;
};

#endif
