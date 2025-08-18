#ifndef CAESAR_PROCESS_H
#define CAESAR_PROCESS_H

#include <sys/user.h>
#include <unistd.h>
class Process {
private:
  pid_t _pid;
  struct user_regs_struct _regs;

public:
  int status;
  explicit Process(pid_t pid);
  ~Process();

  [[nodiscard]] const user_regs_struct &regs();
  [[nodiscard]] const pid_t &pid();
  void wait_status() const;
  [[nodiscard]] int sstep();
};

#endif // !CAESAR_PROCESS_H
