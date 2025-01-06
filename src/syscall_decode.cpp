#include "syscall_decode.h"
#include <fmt/format.h>

std::string SyscallDecoder::syscall_err(const struct user_regs_struct &regs) {
  int err = -(int)regs.rax;
  return fmt::format("{} (Syscall failed with {})", err, strerror(err));
}

std::string_view
SyscallDecoder::syscall_name(const struct user_regs_struct &regs) {
  return "Not implemented yet";
}
