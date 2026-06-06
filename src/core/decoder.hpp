#ifndef CAESAR_DECODER_H
#define CAESAR_DECODER_H

#include <string>

#include "expected.hpp"
#include "typedefs.hpp"

template <int InsSize = 24>
struct Instruction {
  u64 addr{};
  std::array<u8, InsSize> insn{};
  std::string mnemonic;
  std::string op;
};

using DefaultInstruction = Instruction<>;

class Arm64Decoder {
 public:
  static Expected<std::vector<DefaultInstruction>, std::string> decode(
      u32* bytes, size_t size, u64 addr);
};

#endif  // CAESAR_DECODER_H
