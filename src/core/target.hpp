#ifndef CAESAR_TARGET_H
#define CAESAR_TARGET_H

#include <atomic>
#include <cstddef>
#include <fstream>
#include <map>
#include <memory>
#include <thread>

#include "core/decoder.hpp"
#include "core/platform.hpp"
#include "typedefs.hpp"
#include "util.hpp"

enum class TargetState : u8 { STOPPED, RUNNING, EXITED };
enum class BinaryType : u8 { MACHO, ELF, PE };
enum class TargetError : u8 { FORK_FAIL };
enum class ResumeType : u8 { RESUME };

struct Breakpoint {
  u32 orig_ins;
  bool enabled;
};

class Target {
 private:
  static consteval u32 byteArrayToInt(const MagicBytes& bytes);

 protected:
  u64 m_aslr_slide = 0;
  std::unique_ptr<DebugInfo> m_debug_info;
  std::jthread m_waiter;
  std::string m_file_path;
  std::map<u64, Breakpoint> m_breakpoints;
  std::ifstream m_file;
  i32 m_pid = 0;
  std::atomic<TargetState> m_state = TargetState::STOPPED;
  bool m_is_64 = false;

  explicit Target(std::ifstream f, std::string filePath)
      : m_file_path(std::move(filePath)), m_file(std::move(f)) {}

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
  virtual void setThreadState(ThreadState* state) = 0;
  virtual ThreadState& getLastKnownThreadState() = 0;
  virtual u64 writeRegValue(const RegEntryT& regEntry, u64 val) = 0;
  virtual Expected<std::vector<DefaultInstruction>, std::string>
  decodeInstructionRange(AddrType start, AddrType end) = 0;

  void setTargetState(TargetState s) { m_state = s; }
  std::atomic<TargetState>& getTargetState() { return m_state; }
  i32 pid() const { return m_pid; }
  void startEventLoop();
  std::map<u64, Breakpoint>& getRegisteredBreakpoints();
  std::string getInfo();
  std::string formatRegisterOutput(ThreadState* threadState) const;
  std::string formatDisasmOutput(
      const std::vector<DefaultInstruction>& insns) const;
  const std::string& getFilePath() const;
  const std::unique_ptr<DebugInfo>& getDebugInfo() { return m_debug_info; }
  void setDebugInfo(std::unique_ptr<DebugInfo> info) {
    m_debug_info = std::move(info);
  }
  static bool isFileValid(const std::string& filePath);
  static std::unique_ptr<Target> create(const std::string& path);
  std::string formatSymbolLookupOutput(const std::unique_ptr<DebugInfo>& symbols);
};

#endif
