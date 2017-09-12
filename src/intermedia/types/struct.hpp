#pragma once

#include <string>
#include <utility>

#include "util/object/object.hpp"

namespace fie {
class FIStruct : public fun::FObject {
  std::string m_name;

public:
  inline const std::string name() const { return m_name; }

  FIStruct(const std::string &);
};
}
