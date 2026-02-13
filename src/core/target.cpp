#include "target.hpp"

#include <memory>

#include "core/util.hpp"
#include "macho/macho.hpp"

std::unique_ptr<Target> Target::create(const std::string& path) {
  switch (getPlatform()) {
    case PlatformType::MACH:
      auto p = std::make_unique<Macho>(std::ifstream(path), path);
      return p;
  }
}
