#include "atom.hpp"

namespace frma {
std::unordered_map<std::string, std::intptr_t> FAtom::s_ids;

std::vector<std::string> FAtom::s_strings;

FAtom::FAtom(std::intptr_t id) : m_id(id) {}

FAtom FAtom::intern(const std::string &name) {
  if (s_ids.count(name))
    return s_ids.at(name);
  else {
    intptr_t id = s_strings.size();
    s_ids.emplace(name, id);
    s_strings.emplace_back(name);
    return id;
  }
}

const std::string &FAtom::toString() const { return s_strings.at(m_id); }

bool FAtom::operator==(const FAtom &rhs) const { return m_id == rhs.m_id; }
bool FAtom::operator!=(const FAtom &rhs) const { return m_id != rhs.m_id; }
bool FAtom::operator!() const { return m_id == -1; }
}
