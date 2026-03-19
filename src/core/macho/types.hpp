#ifndef CAESAR_MACHO_TYPES_H
#define CAESAR_MACHO_TYPES_H

#include "typedefs.hpp"

struct X86ThreadState64T {
  u64 rax;
  u64 rbx;
  u64 rcx;
  u64 rdx;
  u64 rdi;
  u64 rsi;
  u64 rbp;
  u64 rsp;
  u64 r8;
  u64 r9;
  u64 r10;
  u64 r11;
  u64 r12;
  u64 r13;
  u64 r14;
  u64 r15;
  u64 rip;
  u64 rflags;
  u64 cs;
  u64 fs;
  u64 gs;
};

struct ArmThreadState64T {
  std::array<u64, 29> x;
  u64 fp;
  u64 lr;
  u64 sp;
  u64 pc;
  u32 cpsr;
  u32 pad;
};

#endif
