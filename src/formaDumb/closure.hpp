#pragma once

#include <memory>
#include <unordered_map>

#include "atom.hpp"
#include "entity.hpp"

namespace frma {
class _FClosure;

using FClosure  = std::shared_ptr<_FClosure>;
using FWClosure = std::weak_ptr<_FClosure>;

class _FClosure {
  using Entry = std::pair<FEntity, bool>;

  FClosure m_parent;

  std::unordered_map<FAtom, Entry> m_items;

  Entry *tryGet(FAtom);

  const Entry *tryGet(FAtom) const;

public:
  _FClosure(FClosure);

  _FClosure();

  FEntity bind(FAtom, FEntity, bool);

  inline FEntity get(FAtom name) const { return this->operator[](name); }

  void set(FAtom, FEntity);

  FEntity operator[](FAtom) const;
};
}
