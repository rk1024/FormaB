#include "scope.hpp"

#include <sstream>

#include "function.hpp"

namespace fie {
namespace pc {
  std::string ScopeClosure::assembleName(const std::string &name,
                                         unsigned int       phi,
                                         unsigned int       count) {
    assert(m_id != ID_NONE);

    std::ostringstream oss;
    oss << m_id << "@" << name;
    if (count != COUNT_CONST) {
      if (phi) oss << "^" << phi;
      oss << "`" << count;
    }
    return oss.str();
  }

  std::uint32_t ScopeClosure::recordName(const std::string &name,
                                         unsigned int       phi,
                                         unsigned int       count) {
    auto func = m_func.lock();

    if (m_id == ID_NONE) {
      m_id = func->m_nextScopeId;
      ++func->m_nextScopeId;

      if (m_id > ID_MAX) func->error("scope ID limit exceeded");
    }

    return func->m_body->vars.intern(assembleName(name, phi, count));
  }

  std::uint32_t ScopeClosure::recordVar(fun::FWeakPtr<ScopeClosure> scope_,
                                        const std::string &         name,
                                        unsigned int                phi,
                                        unsigned int                count,
                                        bool                        mut) {
    auto func  = m_func.lock();
    auto scope = scope_.lock();

    if (count > COUNT_MAX && (mut && count == COUNT_CONST))
      func->error("store limit exceeded for variable '" + name + "'");

    m_vars[name] = fun::cons(phi, count);

    if (scope.get() != this) m_borrowed[name] = scope_;

    return recordName(name, phi, count);
  }

  template <bool          required>
  fun::FPtr<ScopeClosure> ScopeClosure::holderOf(const std::string &name) {
    if (m_vars.find(name) != m_vars.end()) return fun::wrap(this);
    if (m_parent) return m_parent->holderOf<required>(name);
    if (required)
      m_func.lock()->error("variable '" + name + "' not declared");
    else
      return nullptr;
  }

  template fun::FPtr<ScopeClosure> ScopeClosure::holderOf<false>(
      const std::string &name);

  template fun::FPtr<ScopeClosure> ScopeClosure::holderOf<true>(
      const std::string &name);

  ScopeClosure::ScopeClosure(fun::FWeakPtr<FuncClosure> func_,
                             fun::FPtr<ScopeClosure>    parent)
      : m_func(func_), m_parent(parent), m_id(ID_NONE) {}

  std::uint32_t ScopeClosure::bind(const std::string &name, bool mut) {
    return recordVar(fun::weak(this), name, 0, mut ? 0 : COUNT_CONST, mut);
  }

  std::uint32_t ScopeClosure::get(const std::string &name) {
    auto holder = holderOf<true>(name);
    auto info   = holder->m_vars.at(name);

    return holder->recordName(name, info.get<0>(), info.get<1>());
  }

  std::uint32_t ScopeClosure::set(const std::string &name) {
    auto holder = holderOf<true>(name);
    auto info   = holder->m_vars.at(name);

    if (info.get<1>() == COUNT_CONST)
      m_func.lock()->error("variable '" + name + "' is immutable");

    if (holder.get() != this)
      return recordVar(fun::weak(holder), name, 0, 0, true);

    return recordVar(
        fun::weak(holder), name, info.get<0>(), info.get<1>() + 1, true);
  }

  std::uint32_t ScopeClosure::phi(fun::FPtr<ScopeClosure> holder,
                                  const std::string &     name) {
    assert(holder->m_vars.find(name) != holder->m_vars.end());

    if (holder.get() != this)
      return recordVar(fun::weak(holder), name, 0, 0, true);

    auto info = holder->m_vars.at(name);

    assert(info.get<1>() != COUNT_CONST);

    return recordVar(fun::weak(this), name, info.get<0>() + 1, 0, true);
  }

  std::vector<ScopeClosure::VarInfo> ScopeClosure::getModified() {
    std::vector<VarInfo> modified;

    for (auto pair : m_vars) {
      auto it = m_borrowed.find(pair.first);

      if (it != m_borrowed.end())
        modified.emplace_back(
            it->second, pair.first, pair.second.get<0>(), pair.second.get<1>());
    }

    return modified;
  }

  std::vector<ScopeClosure::OwnVarInfo> ScopeClosure::getOwned() {
    std::vector<OwnVarInfo> owned;

    for (auto pair : m_vars) {
      if (m_borrowed.find(pair.first) == m_borrowed.end())
        owned.emplace_back(
            pair.first, pair.second.get<0>(), pair.second.get<1>());
    }

    return owned;
  }
}
}
