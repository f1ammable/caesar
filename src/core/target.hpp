#ifndef CAESAR_TARGET_H
#define CAESAR_TARGET_H

#include <array>
#include <atomic>
#include <cstddef>
#include <fstream>
#include <memory>
#include <thread>

#include "typedefs.hpp"
#include "util.hpp"

enum class TargetState : std::uint8_t { STOPPED, RUNNING };
enum class BinaryType : std::uint8_t { MACHO, ELF, PE };
enum class TargetError : std::uint8_t { FORK_FAIL };
enum class ResumeType : std::uint8_t { RESUME };

class Target {
 private:
  using MagicBytes = std::array<std::byte, 4>;
  static consteval u32 byteArrayToInt(const MagicBytes& bytes) {
    return (std::to_integer<u32>(bytes[3]) << 24) |
           (std::to_integer<u32>(bytes[2]) << 16) |
           (std::to_integer<u32>(bytes[1]) << 8) |
           (std::to_integer<u32>(bytes[0]));
  }

 protected:
  explicit Target(std::ifstream f, std::string filePath)
      : m_file(std::move(f)), m_file_path(std::move(filePath)) {}
  virtual void readMagic() = 0;
  virtual void is64() = 0;

  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  std::ifstream m_file;
  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  std::atomic<TargetState> m_state = TargetState::STOPPED;
  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  int32_t m_pid = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  std::string m_file_path;

 public:
  virtual ~Target() = default;
  Target() = delete;
  virtual void dump() = 0;
  static bool isFileValid(const std::string& filePath) {
    const std::unordered_map<u32, PlatformType> magics{
        {{byteArrayToInt(MagicBytes{std::byte{0xCF}, std::byte{0xFA},
                                    std::byte{0xED}, std::byte{0xFE}}),
          PlatformType::MACH},

         {byteArrayToInt(MagicBytes{std::byte{0x7F}, std::byte{'E'},
                                    std::byte{'L'}, std::byte{'F'}}),
          PlatformType::LINUX},

         {byteArrayToInt(MagicBytes{std::byte{'M'}, std::byte{'Z'}, std::byte{},
                                    std::byte{}}),
          PlatformType::WIN}}};

    u32 magicRead = 0;
    std::ifstream f{filePath};
    f.seekg(0, std::ios::beg);
    f.read(std::bit_cast<char*>(&magicRead), sizeof(u32));

    // TODO: Return some sort of error message

    // Unsupported file
    if (!magics.contains(magicRead)) return false;

    return magics.at(magicRead) == getPlatform();
  }

  static std::unique_ptr<Target> create(const std::string& path);

  void setTargetState(TargetState s) { m_state = s; }
  std::atomic<TargetState>& getTargetState() { return m_state; }
  i32 pid() const { return m_pid; }
  virtual i32 attach() = 0;
  virtual void setBreakpoint(u32 addr) = 0;
  virtual i32 launch(CStringArray& argList) = 0;
  virtual void detach() = 0;
  virtual void eventLoop() = 0;
  virtual void resume(ResumeType cond) = 0;
  void startEventLoop() {
    m_waiter = std::jthread(&Target::eventLoop, this);
    while (m_state == TargetState::RUNNING)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  std::jthread m_waiter;
  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  bool m_started = false;
};

#endif
