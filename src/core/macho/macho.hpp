#ifndef CAESAR_MACHO_HPP
#define CAESAR_MACHO_HPP

#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <mach/arm/kern_return.h>
#include <mach/exception_types.h>
#include <mach/mach_types.h>
#include <mach/machine.h>
#include <mach/message.h>
#include <mach/thread_status.h>

#include <fstream>
#include <string>

#include "core/target.hpp"
#include "core/util.hpp"

struct CpuTypeNames {
  cpu_type_t cpu_type;
  const char* cpu_name;
};

class Macho final : public Target {
 public:
  explicit Macho(std::ifstream f, std::string filePath);

  void dumpHeader(int offset) override;
  i32 attach() override;
  i32 setBreakpoint(u64 addr) override;
  i32 disableBreakpoint(u64 addr, bool remove) override;
  i32 launch(CStringArray& argList) override;
  void detach() override;
  void eventLoop() override;
  void resume(ResumeType cond) override;

  static std::string exceptionReason(exception_type_t exc,
                                     mach_msg_type_number_t codeCnt,
                                     mach_exception_data_t code);
  void setThreadPort(mach_port_t thread) { m_thread_port = thread; }
  mach_port_t& getThreadPort() { return m_thread_port; }
  void readAslrSlide();
  u64& getAslrSlide();
  i32 restorePrevIns(u64 k);

 private:
  task_t m_task = 0;
  u64 m_aslr_slide = 0;
  u32 m_magic = 0;
  mach_port_t m_exc_port = 0;
  mach_port_t m_thread_port = 0;
  bool m_is_swap = false;
  static constexpr std::array<CpuTypeNames, 4> CPU_TYPE_NAMES = {
      {{.cpu_type = CPU_TYPE_I386, .cpu_name = "i386"},
       {.cpu_type = CPU_TYPE_X86_64, .cpu_name = "x86_64"},
       {.cpu_type = CPU_TYPE_ARM, .cpu_name = "arm"},
       {.cpu_type = CPU_TYPE_ARM64, .cpu_name = "arm64"}}};

  void readMagic() override;
  void is64() override;

  void maybeSwapBytes();

  template <typename T>
  T loadBytesAndMaybeSwap(u32 offset) {
    T buf;
    m_file.seekg(offset, std::ios::beg);
    m_file.read(std::bit_cast<char*>(&buf), sizeof(T));
    if (m_is_swap) SwapDescriptor<T>::swap(&buf);
    return buf;
  }

  void dumpSegmentCommands(int offset, u32 ncmds);
  static std::string cpuTypeName(cpu_type_t cpuType);
  void dumpSections(u32 offset, u32 end);
  i32 setupExceptionPorts();
  void readAslrSlideFromRegions();
  mach_port_t threadSelect() const;  // TODO: doesn't work (yet)
};

#endif  // CAESAR_MACHO_HPP
