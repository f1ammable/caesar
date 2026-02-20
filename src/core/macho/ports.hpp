#ifndef CAESAR_PORTS_HPP
#define CAESAR_PORTS_HPP

#include <mach/exception_types.h>
#include <mach/message.h>
#include <mach/thread_status.h>

#include <array>

struct MachExcPorts {
  mach_msg_type_number_t excp_type_count{};
  std::array<exception_mask_t, EXC_TYPES_COUNT> masks{};
  std::array<mach_port_t, EXC_TYPES_COUNT> ports{};
  std::array<exception_behavior_t, EXC_TYPES_COUNT> behaviours{};
  std::array<thread_state_flavor_t, EXC_TYPES_COUNT> flavours{};
};

#endif
