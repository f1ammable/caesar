#ifndef CAESAR_TARGET_H
#define CAESAR_TARGET_H

#include <atomic>
#include <cstddef>
#include <fstream>
#include <memory>
#include <thread>

#include "typedefs.hpp"
#include "util.hpp"

enum class TargetState : std::uint8_t { STOPPED, RUNNING, EXITED };
enum class BinaryType : std::uint8_t { MACHO, ELF, PE };
enum class TargetError : std::uint8_t { FORK_FAIL };
enum class ResumeType : std::uint8_t { RESUME };

struct BreakpointInfo {
  u32 addr;
  bool enabled;
};

class Target {
 private:
  static consteval u32 byteArrayToInt(const MagicBytes& bytes);

 protected:
  std::ifstream m_file;
  std::atomic<TargetState> m_state = TargetState::STOPPED;
  i32 m_pid = 0;
  std::string m_file_path;
  std::jthread m_waiter;

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
  virtual i32 setBreakpoint(u32 addr) = 0;
  virtual i32 launch(CStringArray& argList) = 0;
  virtual void detach() = 0;
  virtual void eventLoop() = 0;
  virtual void resume(ResumeType cond) = 0;

  void setTargetState(TargetState s) { m_state = s; }
  std::atomic<TargetState>& getTargetState() { return m_state; }
  i32 pid() const { return m_pid; }
  void startEventLoop();

  static bool isFileValid(const std::string& filePath);
  static std::unique_ptr<Target> create(const std::string& path);
};

#endif
