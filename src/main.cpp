#include "syscall_decode.h"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  // if (argc < 2) {
  //   std::cout << fmt::format("Usage: $ {} <executable> \n", argv[0]);
  //   std::exit(-1);
  // }

  // use /bin/ls for now
  pid_t child = fork();

  if (child == 0) {
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    execl("/bin/ls", "ls", NULL);
  } else {
    long ins;
    int status;
    int insyscall = 0;
    int start = 0;
    long long rax_dec = 0;
    struct user_regs_struct regs;
    while (1) {
      wait(&status);
      if (WIFEXITED(status))
        break;
      ptrace(PTRACE_GETREGS, child, NULL, &regs);
      rax_dec = (long long)regs.rax;
      if (start == 1) {
        ins = ptrace(PTRACE_PEEKTEXT, child, regs.rip, NULL);
        std::cout << fmt::format("rip: {:#x} rax: {}\n", regs.rip,
                                 (rax_dec < 0 && rax_dec > -4096)
                                     ? SyscallDecoder::syscall_err(regs)
                                     : std::to_string(regs.rax));
        std::cout << "Bytes: ";
        for (int i = 0; i < 8; i++) {
          std::cout << fmt::format("{:02x} ", (ins >> (i * 8)) & 0xFF);
        }
        std::cout << "\n";
      }
      if (regs.orig_rax == SYS_write) {
        start = 1;
        ptrace(PTRACE_SINGLESTEP, child, NULL, NULL);
      } else {
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
      }
    }
  }
}
