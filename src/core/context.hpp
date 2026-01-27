#ifndef CAESAR_CONTEXT_H
#define CAESAR_CONTEXT_H

#include <cassert>
#include <memory>

#include "macho.hpp"
#include "target.hpp"
#include "util.hpp"

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Context {
 private:
  Context() = default;
  std::unique_ptr<Target> m_target = nullptr;

  // TODO: Handle endianness differences between platform and binary
 public:
  Context(const Context&) = delete;
  Context(Context&&) = delete;
  Context& operator=(const Context&) = delete;
  Context& operator=(Context&&) = delete;

  static Context& getInstance() {
    static Context instance;
    return instance;
  }

  void initTarget(std::string path) {
    // TODO: Add other platforms here
    switch (getPlatform()) {
      case PlatformType::MACH:
        m_target = std::make_unique<Macho>(std::move(std::ifstream(path)),
                                           std::move(path));
    }
    assert(m_target != nullptr && "Target is not supported yet!");
  }

  std::unique_ptr<Target>& getTarget() { return m_target; }
};

#endif
