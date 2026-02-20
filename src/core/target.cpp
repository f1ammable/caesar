#include "target.hpp"
#include <mach/arm/vm_types.h>

#include <memory>

#include "core/util.hpp"
#include "macho/macho.hpp"

consteval u32 Target::byteArrayToInt(const MagicBytes& bytes) {
  return (std::to_integer<u32>(bytes[3]) << 24) |
         (std::to_integer<u32>(bytes[2]) << 16) |
         (std::to_integer<u32>(bytes[1]) << 8) |
         (std::to_integer<u32>(bytes[0]));
}

std::unique_ptr<Target> Target::create(const std::string& path) {
  switch (getPlatform()) {
    case PlatformType::MACH:
      auto p = std::make_unique<Macho>(std::ifstream(path), path);
      return p;
  }
}

bool Target::isFileValid(const std::string& filePath) {
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

  // Unsupported file
  if (!magics.contains(magicRead)) return false;

  return magics.at(magicRead) == getPlatform();
}

void Target::startEventLoop() {
  m_waiter = std::jthread(&Target::eventLoop, this);
  while (m_state == TargetState::RUNNING)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  if (m_waiter.joinable()) m_waiter.join();
}

std::map<u64, Breakpoint>& Target::getRegisteredBreakpoints() {
  return m_breakpoints;
}
