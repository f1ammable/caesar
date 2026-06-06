#include "decoder.hpp"

#include <capstone/capstone.h>

#include <algorithm>
#include <core/decoder_traits.hpp>
#include <core/util.hpp>

#include "expected.hpp"

Expected<std::vector<DefaultInstruction>, std::string> Arm64Decoder::decode(
    u32* bytes, size_t size, AddrType addr) {
  csh handle{};
  cs_insn* insn{};
  size_t count{};

  if (cs_open(CurrentDecoderTraits::CS_ARCH, CurrentDecoderTraits::CS_MODE,
              &handle) != CS_ERR_OK) {
    return Unexpected{"Could not open handle to capstone!"};
  }

  count = cs_disasm(handle, reinterpret_cast<const uint8_t*>(bytes), size, addr,
                    0, &insn);

  if (count <= 0)
    return Unexpected{
        std::format("Failed to disassemble code at {}!", detail::toHex(addr))};

  // TODO: Optimise so only certain instructions get replaced instead of
  // re-disassembling upon disassembling within the same area
  std::vector<DefaultInstruction> res{};
  res.reserve(count);
  for (int i = 0; i < count; i++) {
    DefaultInstruction ins{.addr = static_cast<AddrType>(insn[i].address),
                           .mnemonic = insn[i].mnemonic,
                           .op = insn[i].op_str};
    std::ranges::copy(insn[i].bytes, ins.insn.begin());
    res.push_back(std::move(ins));
  }
  return res;
}
