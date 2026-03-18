#ifndef CAESAR_REGISTER_H
#define CAESAR_REGISTER_H

#include <array>
#include <typedefs.hpp>

#include "platform.hpp"

enum class Arm64Reg : u8 {
  // General purpose
  X0,
  X1,
  X2,
  X3,
  X4,
  X5,
  X6,
  X7,
  X8,
  X9,
  X10,
  X11,
  X12,
  X13,
  X14,
  X15,
  X16,
  X17,
  X18,
  X19,
  X20,
  X21,
  X22,
  X23,
  X24,
  X25,
  X26,
  X27,
  X28,
  // Named aliases
  FP,  // x29
  LR,  // x30
  // Special registers
  SP,
  PC,
  CPSR,
  COUNT
};

enum class X86Reg : u8 {
  // General purpose (64-bit)
  RAX,
  RBX,
  RCX,
  RDX,
  RSI,
  RDI,
  RBP,
  RSP,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
  // Instruction pointer & flags
  RIP,
  RFLAGS,
  // Segment registers
  CS,
  SS,
  DS,
  ES,
  FS,
  GS,
  COUNT
};

using Reg = std::conditional_t<getArchitecture() == Architecture::ARM64,
                               Arm64Reg, X86Reg>;

struct RegEntry {
  std::string_view name;
  Reg reg;
};

inline constexpr auto REG_MAP =
#ifdef __arm64__
    std::array<RegEntry, 36>{{
        {.name = "x0", .reg = Arm64Reg::X0},
        {.name = "x1", .reg = Arm64Reg::X1},
        {.name = "x2", .reg = Arm64Reg::X2},
        {.name = "x3", .reg = Arm64Reg::X3},
        {.name = "x4", .reg = Arm64Reg::X4},
        {.name = "x5", .reg = Arm64Reg::X5},
        {.name = "x6", .reg = Arm64Reg::X6},
        {.name = "x7", .reg = Arm64Reg::X7},
        {.name = "x8", .reg = Arm64Reg::X8},
        {.name = "x9", .reg = Arm64Reg::X9},
        {.name = "x10", .reg = Arm64Reg::X10},
        {.name = "x11", .reg = Arm64Reg::X11},
        {.name = "x12", .reg = Arm64Reg::X12},
        {.name = "x13", .reg = Arm64Reg::X13},
        {.name = "x14", .reg = Arm64Reg::X14},
        {.name = "x15", .reg = Arm64Reg::X15},
        {.name = "x16", .reg = Arm64Reg::X16},
        {.name = "x17", .reg = Arm64Reg::X17},
        {.name = "x18", .reg = Arm64Reg::X18},
        {.name = "x19", .reg = Arm64Reg::X19},
        {.name = "x20", .reg = Arm64Reg::X20},
        {.name = "x21", .reg = Arm64Reg::X21},
        {.name = "x22", .reg = Arm64Reg::X22},
        {.name = "x23", .reg = Arm64Reg::X23},
        {.name = "x24", .reg = Arm64Reg::X24},
        {.name = "x25", .reg = Arm64Reg::X25},
        {.name = "x26", .reg = Arm64Reg::X26},
        {.name = "x27", .reg = Arm64Reg::X27},
        {.name = "x28", .reg = Arm64Reg::X28},
        {.name = "fp", .reg = Arm64Reg::FP},
        {.name = "x29", .reg = Arm64Reg::FP},
        {.name = "lr", .reg = Arm64Reg::LR},
        {.name = "x30", .reg = Arm64Reg::LR},
        {.name = "sp", .reg = Arm64Reg::SP},
        {.name = "pc", .reg = Arm64Reg::PC},
        {.name = "cpsr", .reg = Arm64Reg::CPSR},
    }};
#endif

#ifdef __i386__
std::array<RegEntry, 24>{{
    {.name = "rax", .reg = X86Reg::RAX},
    {.name = "rbx", .reg = X86Reg::RBX},
    {.name = "rcx", .reg = X86Reg::RCX},
    {.name = "rdx", .reg = X86Reg::RDX},
    {.name = "rsi", .reg = X86Reg::RSI},
    {.name = "rdi", .reg = X86Reg::RDI},
    {.name = "rbp", .reg = X86Reg::RBP},
    {.name = "rsp", .reg = X86Reg::RSP},
    {.name = "r8", .reg = X86Reg::R8},
    {.name = "r9", .reg = X86Reg::R9},
    {.name = "r10", .reg = X86Reg::R10},
    {.name = "r11", .reg = X86Reg::R11},
    {.name = "r12", .reg = X86Reg::R12},
    {.name = "r13", .reg = X86Reg::R13},
    {.name = "r14", .reg = X86Reg::R14},
    {.name = "r15", .reg = X86Reg::R15},
    {.name = "rip", .reg = X86Reg::RIP},
    {.name = "rflags", .reg = X86Reg::RFLAGS},
    {.name = "cs", .reg = X86Reg::CS},
    {.name = "ss", .reg = X86Reg::SS},
    {.name = "ds", .reg = X86Reg::DS},
    {.name = "es", .reg = X86Reg::ES},
    {.name = "fs", .reg = X86Reg::FS},
    {.name = "gs", .reg = X86Reg::GS},
}};
#endif

#endif
