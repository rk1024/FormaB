#include "praeCompilerClosures.hpp"

#include <sstream>
#include <string>

using namespace frma;

namespace fie {
namespace pc {
  PositionTracker::PositionTracker(const FormaAST *curr)
      : m_rootNode(fnew<PositionNode>(this, curr)) {
    m_node = fun::weak(m_rootNode);
  }

  fun::FPtr<PositionNode> PositionTracker::move(const frma::FormaAST *to) {
    return m_node.lock()->push(to);
  }

  PositionNode::PositionNode(PositionTracker *     parent,
                             const frma::FormaAST *curr)
      : m_parent(parent), m_curr(curr) {}

  fun::FPtr<PositionNode> PositionNode::push(const frma::FormaAST *node) {
    auto next    = fnew<PositionNode>(m_parent, node);
    next->m_prev = fun::weak(this);

    if (!m_next) m_parent->m_node = fun::weak(next);

    m_next = fun::weak(next);

    return next;
  }

  PositionNode::~PositionNode() {
    if (m_prev) {
      auto prev = m_prev.lock();
      assert(prev->m_next.peek() == this);
      prev->m_next = m_next;
    }
    if (m_next) {
      auto next = m_next.lock();
      assert(next->m_prev.peek() == this);
      next->m_prev = m_prev;
    } else
      m_parent->m_node = m_prev;
  }

  FuncClosure::FuncClosure(fun::FPtr<AssemblyClosure> assem,
                           FIBytecode &               body,
                           const frma::FormaAST *     curr)
      : PositionTracker(curr), m_assem(assem), m_body(&body) {
    pushScope();
  }

  FuncClosure &FuncClosure::emit(FIInstruction ins) {
    m_body->instructions.push_back(ins);
    return *this;
  }

  std::uint16_t FuncClosure::beginLabel() {
    m_body->labels.emplace_back(static_cast<std::uint16_t>(-1),
                                "l" + std::to_string(m_body->labels.size()));

    assert(static_cast<std::uint16_t>(m_body->labels.size()) != 0);

    return static_cast<std::uint16_t>(m_body->labels.size() - 1);
  }

  void FuncClosure::label(std::uint16_t id) {
    m_body->labels.at(id).pos =
        static_cast<std::uint16_t>(m_body->instructions.size());
  }

  void FuncClosure::pushScope() {
    m_scope = fnew<ScopeClosure>(fun::weak(this), m_scope);
  }

  void FuncClosure::dropScope() { m_scope = m_scope->parent(); }

  fun::FPtr<ScopeClosure> FuncClosure::popScope() {
    auto ret = std::move(m_scope);
    m_scope  = ret->parent();
    return ret;
  }

  void FuncClosure::applyScope() {
    auto scope = popScope();

    for (auto var : scope->getModified()) {
      emit(FIOpcode::Ldvar, scope->get(var.get<1>()));
      emit(FIOpcode::Stvar, m_scope->phi(var.get<0>().lock(), var.get<1>()));
    }
  }

  void FuncClosure::applyScopeWithIds(VarIds &ids, bool add) {
    auto scope = popScope();

    for (auto var : scope->getModified()) {
      auto          cell = fun::cons(var.get<0>(), var.get<1>());
      auto          it   = ids.find(cell);
      std::uint32_t phi;
      if (it == ids.end()) {
        phi                = m_scope->phi(var.get<0>().lock(), var.get<1>());
        if (add) ids[cell] = phi;
      } else
        phi = it->second;

      emit(FIOpcode::Ldvar, scope->get(var.get<1>()));
      emit(FIOpcode::Stvar, phi);
    }
  }

  FuncClosure::VarIds FuncClosure::applyScopeWithIds() {
    VarIds ids;
    applyScopeWithIds(ids, true);
    return ids;
  }

  void FuncClosure::error(std::string &&desc) {
    auto loc = curr()->loc();

    std::ostringstream os;

    os << "\x1b[1m";

    if (loc.begin.filename)
      os << *loc.begin.filename;
    else
      os << "???";


    os << ":" << loc.begin.line << ":" << loc.begin.column;

    if (loc.end != loc.begin) os << "-";

    if (loc.end.filename != loc.begin.filename) {
      if (loc.end.filename)
        os << *loc.end.filename;
      else
        os << "???";

      os << ":";

      goto diffLine;
    } else if (loc.end.line != loc.begin.line) {
    diffLine:
      os << loc.end.line << ":";

      goto diffCol;
    } else if (loc.end.column != loc.begin.column) {
    diffCol:
      os << loc.end.column;
    }

    os << ": \x1b[38;5;9merror:\x1b[0m " << desc << std::endl;

    std::cerr << os.str();

    throw std::runtime_error(os.str());
  }

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
