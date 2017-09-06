#include "function.hpp"

#include "scope.hpp"

namespace fie {
namespace pc {
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
}
}
