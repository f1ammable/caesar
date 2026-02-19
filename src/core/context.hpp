#ifndef CAESAR_CONTEXT_H
#define CAESAR_CONTEXT_H

#include <cassert>
#include <memory>

#include "target.hpp"

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Context {
 private:
  Context() = default;
  inline static std::unique_ptr<Target> mTarget;

 public:
  Context(const Context&) = delete;
  Context(Context&&) = delete;
  Context& operator=(const Context&) = delete;
  Context& operator=(Context&&) = delete;

  static Context& getInstance() {
    static Context instance;
    return instance;
  }

  static std::unique_ptr<Target>& getTarget() { return mTarget; }
  static void setTarget(std::unique_ptr<Target> ptr) { mTarget = std::move(ptr); }
};

#endif
