#ifndef CAESAR_DECODER_TRAITS_H
#define CAESAR_DECODER_TRAITS_H

#include <capstone/capstone.h>

#include <core/platform.hpp>

template <Architecture A, Platform P>
struct DecoderTraits;

template <>
struct DecoderTraits<Architecture::ARM64, Platform::MACH> {
  static constexpr cs_arch CS_ARCH = CS_ARCH_ARM64;
  static constexpr cs_mode CS_MODE = CS_MODE_ARM;
};

using CurrentDecoderTraits = DecoderTraits<getArchitecture(), getPlatform()>;

#endif
