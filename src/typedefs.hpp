#ifndef CAESAR_TYPEDEFS_H
#define CAESAR_TYPEDEFS_H

#include <array>
#include <cstdint>
#include <cmd/object.hpp>

using u32 = std::uint32_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;
using MagicBytes = std::array<std::byte, 4>;
using FnPtr = Object (*)(const std::vector<std::string>&);

#endif
