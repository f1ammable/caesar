#ifndef CAESAR_TARGET_H
#define CAESAR_TARGET_H

#include <array>
#include <cstddef>
#include <fstream>
#include <memory>
#include <unordered_map>

#include "context.hpp"

class Target {
 protected:
  explicit Target(std::ifstream f)
      : m_file(std::make_unique<std::ifstream>(std::move(f))) {}
  virtual void readMagic() = 0;
  virtual void is64() = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  std::unique_ptr<std::ifstream> m_file;

  using MagicBytes = std::array<std::byte, 4>;
  using u32 = std::uint32_t;

  static consteval u32 byteArrayToInt(const MagicBytes& bytes) {
    return (std::to_integer<u32>(bytes[3]) << 24) |
           (std::to_integer<u32>(bytes[2]) << 16) |
           (std::to_integer<u32>(bytes[1]) << 8) |
           (std::to_integer<u32>(bytes[0]));
  }

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

    return magics.at(magicRead) == Context::getPlatform();
  }
};

#endif
