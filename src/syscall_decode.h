#ifndef CAESAR_SYSCALL_DECODE_H
#define CAESAR_SYSCALL_DECODE_H

#include <string>
#include <sys/user.h>

class SyscallDecoder {
private:
  // autogen this with m4, read from unistd_64.h for x86_64
  // static constexpr std::map<int, std::string_view> syscall_names =
  // std::map<int, std::string_view>();
public:
  // provide human-readable syscall errors
  static std::string syscall_err(const struct user_regs_struct &regs);
  static constexpr std::string_view
  syscall_name(const struct user_regs_struct &regs);
};

#endif // !CAESAR_SYSCALL_DECODE_H
