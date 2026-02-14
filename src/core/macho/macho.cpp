#include "macho.hpp"

#include <architecture/byte_order.h>
#include <libproc.h>
#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <mach/arm/kern_return.h>
#include <mach/arm/thread_status.h>
#include <mach/exception_types.h>
#include <mach/kern_return.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/mach_traps.h>
#include <mach/mach_types.h>
#include <mach/machine.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#include <mach/port.h>
#include <mach/task.h>
#include <mach/thread_act.h>
#include <mach/thread_special_ports.h>
#include <mach/vm_map.h>
#include <mach/vm_types.h>
#include <spawn.h>
#include <sys/proc_info.h>
#include <sys/ptrace.h>
#include <sys/signal.h>
#include <sys/spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <core/context.hpp>
#include <core/util.hpp>
#include <csignal>
#include <cstring>
#include <error.hpp>
#include <iostream>
#include <macho/ports.hpp>
#include <stdexcept>

#include "target.hpp"
extern "C" {
#include "mach_exc.h"
#include "mach_excServer.h"
}

// Adapted from https://lowlevelbits.org/parsing-mach-o-files/

void Macho::dump() { dumpMachHeader(0); }

void Macho::readMagic() {
  uint32_t magic = 0;
  m_file.seekg(0, std::ios::beg);
  m_file.read(std::bit_cast<char*>(&magic), sizeof(uint32_t));
  m_magic = magic;
}

void Macho::is64() {
  if (m_magic == MH_MAGIC_64 || m_magic == MH_CIGAM_64) m_is_64 = true;
}

void Macho::maybeSwapBytes() {
  if (m_magic == MH_CIGAM || m_magic == MH_CIGAM_64) m_is_swap = true;
}

void Macho::dumpMachHeader(int offset) {
  uint32_t ncmds = 0;
  int loadCmdsOffset = offset;

  if (m_is_64) {
    constexpr size_t headerSize = sizeof(struct mach_header_64);
    auto header = loadBytesAndMaybeSwap<mach_header_64>(offset);
    ncmds = header.ncmds;
    loadCmdsOffset += headerSize;
    std::cout << Macho::cpuTypeName(header.cputype) << '\n';
  } else {
    int headerSize = sizeof(struct mach_header);
    auto header = loadBytesAndMaybeSwap<mach_header>(offset);
  }

  dumpSegmentCommands(loadCmdsOffset, ncmds);
}

void Macho::dumpSegmentCommands(int offset, uint32_t ncmds) {
  uint32_t actualOffset = offset;
  for (int i = 0; i < ncmds; i++) {
    auto cmd = loadBytesAndMaybeSwap<load_command>(actualOffset);
    if (cmd.cmd == LC_SEGMENT_64) {
      auto segment = loadBytesAndMaybeSwap<segment_command_64>(actualOffset);
      std::cout << std::format(
                       "segname: {:<25} offset: 0x{:<12x} vmaddr: 0x{:<18x} "
                       "vmsize: 0x{:x}",
                       segment.segname, segment.fileoff, segment.vmaddr,
                       segment.vmsize)
                << '\n';
      dumpSections(actualOffset + sizeof(segment_command_64),
                   actualOffset + cmd.cmdsize);
    }
    actualOffset += cmd.cmdsize;
  }
}

std::string Macho::cpuTypeName(cpu_type_t cpuType) {
  for (const auto& x : CPU_TYPE_NAMES) {
    if (cpuType == x.cpu_type) return x.cpu_name;
  }
  return "unknown";
}

void Macho::dumpSections(uint32_t offset, uint32_t end) {
  uint32_t actualOffset = offset;
  while (actualOffset != end) {
    auto section = loadBytesAndMaybeSwap<section_64>(actualOffset);
    std::cout << std::format("Section: {}; Address: 0x{:x}", section.sectname,
                             section.addr)
              << '\n';
    actualOffset += sizeof(section_64);
  }
}

Macho::Macho(std::ifstream f, std::string filePath)
    : Target(std::move(f), std::move(filePath)) {
  readMagic();
  is64();
  maybeSwapBytes();
}

// TODO: Add appropriate error messages
i32 Macho::launch(CStringArray& argList) {
  pid_t pid = 0;
  int status = 0;
  posix_spawnattr_t attr = nullptr;

  status = posix_spawnattr_init(&attr);
  if (status != 0) return -1;

  status = posix_spawnattr_setflags(&attr, POSIX_SPAWN_START_SUSPENDED);
  if (status != 0) return -1;

  argList.prepend(m_file_path);

  status = posix_spawn(&pid, m_file_path.c_str(), nullptr, &attr,
                       argList.data(), nullptr);
  posix_spawnattr_destroy(&attr);
  if (status != 0) return -1;
  m_pid = pid;

  // TODO: This only works with sudo even when codesigned, why?
  kern_return_t kr = task_for_pid(mach_task_self(), pid, &this->m_task);
  if (kr != KERN_SUCCESS) {
    CoreError::error(mach_error_string(kr));
  }
  return pid;
}

i32 Macho::attach() {
  // http://uninformed.org/index.cgi?v=4&a=3&p=14
  i32 res = this->setupExceptionPorts();
  ptrace(PT_ATTACHEXC, m_pid, nullptr, 0);

  return res;
};

void Macho::setBreakpoint(u32 addr) {
  throw std::runtime_error("unimplemented");
};

void Macho::detach() { throw std::runtime_error("unimplemented"); }

i32 Macho::setupExceptionPorts() {
  i32 res = 0;
  MachExcPorts savedPorts{};
  task_get_exception_ports(
      m_task,
      EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC |
          EXC_MASK_EMULATION | EXC_MASK_SOFTWARE | EXC_MASK_BREAKPOINT |
          EXC_MASK_SYSCALL | EXC_MASK_MACH_SYSCALL | EXC_MASK_RPC_ALERT |
          EXC_MASK_CRASH,
      savedPorts.masks.data(), &savedPorts.excp_type_count,
      savedPorts.ports.data(), savedPorts.behaviours.data(),
      savedPorts.flavours.data());
  kern_return_t kr = mach_port_allocate(mach_task_self(),
                                        MACH_PORT_RIGHT_RECEIVE, &m_exc_port);

  if (kr != KERN_SUCCESS) {
    CoreError::error(mach_error_string(kr));
    res = -1;
  }

  kr = mach_port_insert_right(mach_task_self(), m_exc_port, m_exc_port,
                              MACH_MSG_TYPE_MAKE_SEND);

  if (kr != KERN_SUCCESS) {
    CoreError::error(mach_error_string(kr));
    res = -1;
  }

  kr = task_set_exception_ports(
      m_task,
      EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC |
          EXC_MASK_EMULATION | EXC_MASK_SOFTWARE | EXC_MASK_BREAKPOINT |
          EXC_MASK_SYSCALL | EXC_MASK_MACH_SYSCALL | EXC_MASK_RPC_ALERT |
          EXC_MASK_CRASH,
      m_exc_port, EXCEPTION_STATE_IDENTITY | MACH_EXCEPTION_CODES,
      ARM_THREAD_STATE64);

  if (kr != KERN_SUCCESS) {
    CoreError::error(mach_error_string(kr));
    res = -1;
  }

  return res;
}

void Macho::eventLoop() {
  mach_msg_return_t ret = 0;
  __RequestUnion__mach_exc_subsystem msgBuf{};
  __ReplyUnion__mach_exc_subsystem rplBuf{};

  auto* msg = reinterpret_cast<mach_msg_header_t*>(&msgBuf);
  auto* rpl = reinterpret_cast<mach_msg_header_t*>(&rplBuf);

  while (m_state == TargetState::RUNNING) {
    ret = mach_msg(msg, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0,
                   sizeof(__RequestUnion__mach_exc_subsystem), m_exc_port, 100,
                   MACH_PORT_NULL);
    if (ret == MACH_RCV_TIMED_OUT) {
      int status = 0;
      if (waitpid(m_pid, &status, WNOHANG) > 0) m_state = TargetState::STOPPED;
      continue;
    }
    assert(ret == MACH_MSG_SUCCESS && "Did not receive mach message");

    mach_exc_server(msg, rpl);

    ret = mach_msg(rpl, MACH_SEND_MSG, rpl->msgh_size, 0, MACH_PORT_NULL, 0,
                   MACH_PORT_NULL);
    assert(ret == MACH_MSG_SUCCESS && "Did not send mach message");
  }
}

extern "C" {

kern_return_t catch_mach_exception_raise(mach_port_t excPort,
                                         mach_port_t threadPort,
                                         mach_port_t taskPort,
                                         exception_type_t excType,
                                         mach_exception_data_t codes,
                                         mach_msg_type_number_t numCodes) {
#pragma unused(excPort)
#pragma unused(threadPort)
#pragma unused(excType)
#pragma unused(numCodes)
#pragma unused(taskPort)
#pragma unused(codes)
  return KERN_FAILURE;
}

kern_return_t catch_mach_exception_raise_state(
    mach_port_t excPort, exception_type_t exc,
    const mach_exception_data_t code,  // NOLINT(misc-misplaced-const)
    mach_msg_type_number_t codeCnt, int* flavour,
    const thread_state_t oldState,  // NOLINT(misc-misplaced-const)
    mach_msg_type_number_t oldStateCnt, thread_state_t newState,
    mach_msg_type_number_t* newStateCnt) {
#pragma unused(excPort)
#pragma unused(exc)
#pragma unused(code)
#pragma unused(codeCnt)
#pragma unused(oldStateCnt)
#pragma unused(newState)
#pragma unused(newStateCnt)

  return KERN_FAILURE;
}

kern_return_t catch_mach_exception_raise_state_identity(
    mach_port_t excPort, mach_port_t thread, mach_port_t task,
    exception_type_t exc, mach_exception_data_t code,
    mach_msg_type_number_t codeCnt, int* flavour, thread_state_t oldState,
    mach_msg_type_number_t oldStateCnt, thread_state_t newState,
    mach_msg_type_number_t* newStateCnt) {
#pragma unused(excPort)
#pragma unused(task)
#pragma unused(code)
#pragma unused(codeCnt)
#pragma unused(newState)
#pragma unused(newStateCnt)
  auto& target = Context::getInstance().getTarget();
  target->setTargetState(TargetState::STOPPED);

  std::cout << Macho::exceptionReason(exc, codeCnt, code);

  auto* macho = dynamic_cast<Macho*>(Context::getInstance().getTarget().get());
  auto* oldArmState = reinterpret_cast<arm_thread_state64_t*>(oldState);
  auto* newArmState = reinterpret_cast<arm_thread_state64_t*>(newState);
  memcpy(newArmState, oldArmState, sizeof(arm_thread_state64_t));

  macho->setThreadPort(thread);

  if (exc == EXC_BAD_INSTRUCTION && *flavour == ARM_THREAD_STATE64) {
    printf("Fault at: %llu\n", oldArmState->__pc);
    newArmState->__pc += 4;
    printf("Resume at: %llu\n", newArmState->__pc);
  }

  *newStateCnt = oldStateCnt;
  return KERN_SUCCESS;
}

}  // extern "C"

mach_port_t Macho::threadSelect() {
  std::cout << "TASK: " << m_task << '\n';
  thread_act_array_t acts{};
  mach_msg_type_number_t numThreads = 0;

  kern_return_t kr = task_threads(m_task, &acts, &numThreads);

  if (kr != KERN_SUCCESS) {
    std::cout << "task_threads fail!\n";
    CoreError::error(mach_error_string(kr));
  }

  thread_act_t mainThread = acts[0];  // Cleanup
  vm_deallocate(mach_task_self(), reinterpret_cast<vm_address_t>(acts),
                numThreads * sizeof(thread_act_t));

  return mainThread;
}

void Macho::resume(ResumeType cond) {
  auto& target = Context::getInstance().getTarget();

  if (!m_started) {
    std::cerr << "Target is not running!\n";
    return;
  }

  switch (cond) {
    case ResumeType::RESUME:
      ptrace(PT_THUPDATE, m_pid,
             reinterpret_cast<caddr_t>(static_cast<uintptr_t>(m_thread_port)),
             0);
      ptrace(PT_CONTINUE, m_pid, reinterpret_cast<caddr_t>(1), 0);
      target->setTargetState(TargetState::RUNNING);
      break;
  }
}

std::string Macho::exceptionReason(exception_type_t exc,
                                   mach_msg_type_number_t codeCnt,
                                   mach_exception_data_t code) {
  const auto signalStr = [] {
    std::map<mach_exception_data_type_t, std::string> res;
#define INSERT_ELEM(p) res.emplace(p, #p);
    INSERT_ELEM(SIGHUP);
    INSERT_ELEM(SIGINT);
    INSERT_ELEM(SIGQUIT);
    INSERT_ELEM(SIGILL);
    INSERT_ELEM(SIGTRAP);
    INSERT_ELEM(SIGABRT);
    INSERT_ELEM(SIGFPE);
    INSERT_ELEM(SIGKILL);
    INSERT_ELEM(SIGBUS);
    INSERT_ELEM(SIGSEGV);
    INSERT_ELEM(SIGSYS);
    INSERT_ELEM(SIGPIPE);
    INSERT_ELEM(SIGALRM);
    INSERT_ELEM(SIGTERM);
    INSERT_ELEM(SIGURG);
    INSERT_ELEM(SIGSTOP);
    INSERT_ELEM(SIGTSTP);
    INSERT_ELEM(SIGCONT);
    INSERT_ELEM(SIGCHLD);
    INSERT_ELEM(SIGTTIN);
    INSERT_ELEM(SIGTTOU);
    INSERT_ELEM(SIGIO);
    INSERT_ELEM(SIGXCPU);
    INSERT_ELEM(SIGXFSZ);
    INSERT_ELEM(SIGVTALRM);
    INSERT_ELEM(SIGPROF);
    INSERT_ELEM(SIGWINCH);
    INSERT_ELEM(SIGINFO);
    INSERT_ELEM(SIGUSR1);
    INSERT_ELEM(SIGUSR2);
#undef INSERT_ELEM
    return res;
  };

  const auto machExcStr = [] {
    std::map<mach_exception_data_type_t, std::string> res{};
#define INSERT_ELEM(p) res.emplace(p, #p);
    INSERT_ELEM(EXC_BAD_ACCESS);
    INSERT_ELEM(EXC_BAD_INSTRUCTION);
    INSERT_ELEM(EXC_ARITHMETIC);
    INSERT_ELEM(EXC_EMULATION);
    INSERT_ELEM(EXC_SOFTWARE);
    INSERT_ELEM(EXC_BREAKPOINT);
    INSERT_ELEM(EXC_SYSCALL);
    INSERT_ELEM(EXC_MACH_SYSCALL);
    INSERT_ELEM(EXC_RPC_ALERT);
    INSERT_ELEM(EXC_CRASH);
    INSERT_ELEM(EXC_RESOURCE);
    INSERT_ELEM(EXC_GUARD);
    INSERT_ELEM(EXC_CORPSE_NOTIFY);
#undef INSERT_ELEM
    return res;
  };

  std::string reason{};

  // Unix signal
  if (exc == EXC_SOFTWARE && codeCnt >= 2 && code[0] == EXC_SOFT_SIGNAL) {
    reason = std::format("signal {}\n", signalStr()[code[1]]);
  } else {
    reason = std::format("reason {}\n", machExcStr()[exc]);
  }

  return reason;
}
