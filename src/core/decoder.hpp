#ifndef CAESAR_DECODER_H
#define CAESAR_DECODER_H

#include <string>
#include <type_traits>

#include "core/platform.hpp"
#include "expected.hpp"
#include "typedefs.hpp"

template <typename AddrT, int InsSize = 24>
struct Instruction {
  AddrT addr{};
  std::array<u8, InsSize> insn{};
  std::string mnemonic;
  std::string op;
};

using AddrType =
    std::conditional_t<getArchitecture() == Architecture::X86, u32, u64>;
using DefaultInstruction = Instruction<AddrType>;

class Arm64Decoder {
 public:
  static Expected<std::vector<DefaultInstruction>, std::string> decode(
      u32* bytes, size_t size, AddrType addr);
};

#endif  // CAESAR_DECODER_H
