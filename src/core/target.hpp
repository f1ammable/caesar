#ifndef CAESAR_TARGET_H
#define CAESAR_TARGET_H

#include <atomic>
#include <cstddef>
#include <fstream>
#include <map>
#include <memory>
#include <thread>

#include "typedefs.hpp"
#include "util.hpp"

enum class TargetState : u8 { STOPPED, RUNNING, EXITED };
enum class BinaryType : u8 { MACHO, ELF, PE };
enum class TargetError : u8 { FORK_FAIL };
enum class ResumeType : u8 { RESUME };
enum class Arm64Reg : u8 {
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
  X29,
  X30,
  SP,
  PC,
  LR,
  CSPR,
  COUNT
};

struct Breakpoint {
  u32 orig_ins;
  bool enabled;
};

class Target {
 private:
  static consteval u32 byteArrayToInt(const MagicBytes& bytes);

 protected:
  std::ifstream m_file;
  std::string m_file_path;
  std::jthread m_waiter;
  std::map<u64, Breakpoint> m_breakpoints;
  i32 m_pid = 0;
  std::atomic<TargetState> m_state = TargetState::STOPPED;
  bool m_is_64 = false;

  explicit Target(std::ifstream f, std::string filePath)
      : m_file(std::move(f)), m_file_path(std::move(filePath)) {}

  virtual void readMagic() = 0;
  virtual void is64() = 0;

 public:
  bool m_started = false;

  virtual ~Target() = default;
  Target() = delete;

  virtual void dumpHeader(int offset) = 0;
  virtual i32 attach() = 0;
  virtual i32 setBreakpoint(u64 addr) = 0;
  virtual i32 disableBreakpoint(u64 addr, bool remove) = 0;
  virtual i32 launch(detail::CStringArray& argList) = 0;
  virtual void detach() = 0;
  virtual void eventLoop() = 0;
  virtual void resume(ResumeType cond) = 0;

  void setTargetState(TargetState s) { m_state = s; }
  std::atomic<TargetState>& getTargetState() { return m_state; }
  i32 pid() const { return m_pid; }
  void startEventLoop();
  std::map<u64, Breakpoint>& getRegisteredBreakpoints();
  std::string getInfo();
  std::string getRegisterInfo();

  static bool isFileValid(const std::string& filePath);
  static std::unique_ptr<Target> create(const std::string& path);
};

#endif
