#include "target.hpp"

#include <memory>

#include "platform.hpp"

#ifdef __APPLE__
#include "macho/macho.hpp"
#endif

consteval u32 Target::byteArrayToInt(const MagicBytes& bytes) {
  return (std::to_integer<u32>(bytes[3]) << 24) |
         (std::to_integer<u32>(bytes[2]) << 16) |
         (std::to_integer<u32>(bytes[1]) << 8) |
         (std::to_integer<u32>(bytes[0]));
}

std::unique_ptr<Target> Target::create(const std::string& path) {
#ifdef __APPLE__
  return std::make_unique<Macho>(std::ifstream(path), path);
#endif
  return nullptr;
}

bool Target::isFileValid(const std::string& filePath) {
  const std::unordered_map<u32, Platform> magics{
      {{byteArrayToInt(MagicBytes{std::byte{0xCF}, std::byte{0xFA},
                                  std::byte{0xED}, std::byte{0xFE}}),
        Platform::MACH},

       {byteArrayToInt(MagicBytes{std::byte{0x7F}, std::byte{'E'},
                                  std::byte{'L'}, std::byte{'F'}}),
        Platform::LINUX},

       {byteArrayToInt(MagicBytes{std::byte{'M'}, std::byte{'Z'}, std::byte{},
                                  std::byte{}}),
        Platform::WIN}}};

  const u32 magicRead = 0;
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

std::string Target::getInfo() {
  std::string res{};

  res += std::format("Target: {} \n", m_file_path);
  res += std::format("Type: {}, Architecure: {}", getTargetType(),
                     m_is_64 ? "64-bit" : "32-bit");
  return res;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::string Target::formatRegisterOutput(ThreadState* threadState) const {
  std::string res{};
  constexpr int regsPerRow = 4;
  for (int i = 0; i < 29; i++) {
    res += std::format(
        " x{:<2}: {}", i,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        detail::toHex(threadState->x[i]));
    if ((i + 1) % regsPerRow == 0)
      res += '\n';
    else
      res += "  ";
  }

  res += std::format("\n fp: {} lr: {}\n", detail::toHex(threadState->fp),
                     detail::toHex(threadState->lr));
  res += std::format(" sp: {} pc: {}\n", detail::toHex(threadState->sp),
                     detail::toHex(threadState->pc));
  res += std::format(" cpsr: {}\n", detail::toHex(threadState->cpsr));
  return res;
}
