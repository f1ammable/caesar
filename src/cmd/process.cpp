#include "process.h"

#include <fmt/format.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

Process::Process(pid_t pid) : _pid(pid) {}
Process::~Process() {
  std::cout << "Detaching\n";
  ptrace(PTRACE_DETACH, this->_pid, 0, 0);
}

void Process::wait_status() const {
  if (WIFSTOPPED(this->status))
    std::cout << fmt::format("Child stopped {}\n", WSTOPSIG(this->status));
  if (WIFEXITED(this->status))
    std::cout << fmt::format("Child exited {}\n", WEXITSTATUS(this->status));
  if (WIFSIGNALED(this->status))
    std::cout << fmt::format("Child signaled {}\n", WTERMSIG(this->status));
  if (WCOREDUMP(this->status)) std::cout << fmt::format("Child core dumped\n");
}

[[nodiscard]] const pid_t& Process::pid() { return this->_pid; }
[[nodiscard]] const user_regs_struct& Process::regs() {
  if (ptrace(PTRACE_GETREGS, this->_pid, NULL, &this->_regs)) {
    std::cerr << fmt::format("Error fetching registers from child {}\n",
                             strerror(errno));
    std::exit(-1);
  }
  return this->_regs;
}

[[nodiscard]] int Process::sstep() {
  int retval = ptrace(PTRACE_SINGLESTEP, this->_pid, 0, 0);
  if (retval) return retval;
  waitpid(this->_pid, &this->status, 0);
  return this->status;
}
