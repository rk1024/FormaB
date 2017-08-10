#include "closure.hpp"

#include <stdexcept>

namespace frma {
_FClosure::Entry *_FClosure::tryGet(FAtom name) {
  if (m_items.count(name)) return &m_items.at(name);

  if (m_parent) return m_parent->tryGet(name);

  return nullptr;
}

const _FClosure::Entry *_FClosure::tryGet(FAtom name) const {
  if (m_items.count(name)) return &m_items.at(name);

  if (m_parent) return m_parent->tryGet(name);

  return nullptr;
}

_FClosure::_FClosure(FClosure parent) : m_parent(parent) {}

_FClosure::_FClosure() : _FClosure(nullptr) {}

FEntity _FClosure::bind(FAtom name, FEntity ent, bool mut) {
  if (m_items.count(name))
    throw std::runtime_error("Name '" + name.toString() + "' already bound");

  m_items[name] = std::make_pair(ent, mut);

  return m_items.at(name).first;
}

void _FClosure::set(FAtom name, FEntity ent) {
  if (auto pair = tryGet(name)) {
    if (!pair->second)
      throw std::runtime_error(
          "Name '" + name.toString() +
          "' is bound to a constant and cannot be modified.");
    pair->first = ent;
  } else
    throw std::runtime_error("Undeclared closure name '" + name.toString() +
                             "'");
}

FEntity _FClosure::operator[](FAtom name) const {
  if (auto pair = tryGet(name)) {
    if (pair->first) return pair->first;
  }

  throw std::runtime_error("Unknown closure name '" + name.toString() + "'");
}
}
