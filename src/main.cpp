#include "process.h"
#include "syscall_decode.h"
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp) {
  // if (argc < 2) {
  //   std::cout << fmt::format("Usage: $ {} <executable> \n", argv[0]);
  //   return -1;
  // }

  auto p = std::make_unique<Process>(fork());
  auto pid = p->pid();
  auto status = p->status;

  if (pid == -1) {
    std::cerr << fmt::format("Error forking {}\n", strerror(errno));
    return -1;
  }

  // child
  if (pid == 0) {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0)) {
      std::cerr << fmt::format("Error setting TRACEME {}\n", strerror(errno));
      return -1;
    }
    // use /bin/ls for now
    char *args[] = {(char *)"ls", NULL};
    execve("/bin/ls", args, envp);
  }

  // parent
  else {
    waitpid(pid, &status, 0);
    p->wait_status();
    // this works but not well
    while (WIFSTOPPED(status)) {
      struct user_regs_struct regs = p->regs();
      long long rax_dec = (long long)regs.rax;
      long ins = ptrace(PTRACE_PEEKTEXT, pid, regs.rip, NULL);
      std::cout << fmt::format("rip: {:#x} rax: {}\n", regs.rip,
                               (rax_dec < 0 && rax_dec > -4096)
                                   ? SyscallDecoder::syscall_err(regs)
                                   : std::to_string(regs.rax));
      std::cout << "Bytes: ";
      for (int i = 0; i < 8; i++) {
        std::cout << fmt::format("{:02x} ", (ins >> (i * 8)) & 0xFF);
      }
      std::cout << "\n";
      p->status = p->sstep();
    }
  }
}
