#pragma once

#include <string>

namespace frma {
class FNamespace {
  static FNamespace s_global;

  std::string m_name;

public:
  static const FNamespace &global();

  const std::string &name() const { return m_name; }

  FNamespace(const std::string &name);
};
}
