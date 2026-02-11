#include "target.hpp"
#include "macho/macho.hpp"

#include <memory>

#include "core/util.hpp"

std::unique_ptr<Target> Target::create(const std::string& path) {
  switch (getPlatform()) {
    case PlatformType::MACH:
      return std::make_unique<Macho>(std::ifstream(path), path);
  }
}
