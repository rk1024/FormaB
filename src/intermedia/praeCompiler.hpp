#pragma once

#include <string.h>

#include "util/atom.hpp"
#include "util/enableRefCount.hpp"

#include "function.hpp"
#include "opcode.hpp"

#include "ast.hpp"

namespace fie {
class _FIPraeCompiler {
  class _AssemblyClosure {
    fun::FAtomStore<FIFunction, std::uint16_t> m_funcs;

  public:
    auto funcs() -> decltype(m_funcs) & { return m_funcs; }
    auto funcs() const -> const decltype(m_funcs) & { return m_funcs; }
  };

  using AssemblyClosure = fun::EnableRefCount<_AssemblyClosure>;

  class FuncClosure {
    AssemblyClosure m_assem;
    FIBytecode *    m_body;

  public:
    inline AssemblyClosure assem() const { return m_assem; }
    inline FIBytecode *    body() const { return m_body; }

    FuncClosure(const AssemblyClosure &assem, FIBytecode &body)
        : m_assem(assem), m_body(&body) {}

    inline FuncClosure &emit(FIInstruction ins) {
      m_body->instructions.push_back(ins);
      return *this;
    }

    inline FuncClosure &emit(FIOpcode op) { return emit(FIInstruction(op)); }

    template <typename T>
    inline
        typename std::enable_if<std::is_integral<T>::value, FuncClosure>::type &
        emit(FIOpcode op, T arg) {
      return emit(FIInstruction(op, arg));
    }

    inline auto beginLabel() {
      m_body->labels.emplace_back(static_cast<std::uint16_t>(-1),
                                  "l" + std::to_string(m_body->labels.size()));

      assert(static_cast<std::uint16_t>(m_body->labels.size()) != 0);

      return static_cast<std::uint16_t>(m_body->labels.size() - 1);
    }

    inline void label(std::uint16_t id) {
      m_body->labels.at(id).pos =
          static_cast<std::uint16_t>(m_body->instructions.size());
    }
  };

  fun::FAtomStore<AssemblyClosure> m_assems;

  std::uint32_t emitLoadExprs(FuncClosure &,
                              const frma::FPExprs *,
                              bool = true);
  bool emitLoadExpr(FuncClosure &, const frma::FPExpr *);

  bool emitLoadLBoolean(FuncClosure &, const frma::FPLBoolean *);
  bool emitLoadLNumeric(FuncClosure &, const frma::FPLNumeric *);
  bool emitLoadXBlock(FuncClosure &, const frma::FPXBlock *);
  bool emitLoadXControl(FuncClosure &, const frma::FPXControl *);
  bool emitLoadXFunc(FuncClosure &, const frma::FPXFunc *);
  bool emitLoadXInfix(FuncClosure &, const frma::FPXInfix *);
  bool emitLoadXMember(FuncClosure &, const frma::FPXMember *);
  bool emitLoadXParen(FuncClosure &, const frma::FPXParen *, bool = false);
  bool emitLoadXPrim(FuncClosure &, const frma::FPXPrim *);
  bool emitLoadXUnary(FuncClosure &, const frma::FPXUnary *);

  void emitStmts(FuncClosure &, const frma::FPStmts *);
  void emitStmt(FuncClosure &, const frma::FPStmt *);

  void emitSBind(FuncClosure &, const frma::FPSBind *);
  void emitSControl(FuncClosure &, const frma::FPSControl *);

  void emitBindings(FuncClosure &, const frma::FPBindings *, bool);
  void emitBinding(FuncClosure &, const frma::FPBinding *, bool);

  inline void _compileStatements(FuncClosure &c, const frma::FPExpr *e) {
    emitLoadExpr(c, e);
  }
  inline void _compileStatements(FuncClosure &c, const frma::FPLBoolean *b) {
    emitLoadLBoolean(c, b);
  }
  inline void _compileStatements(FuncClosure &c, const frma::FPLNumeric *n) {
    emitLoadLNumeric(c, n);
  }
  inline void _compileStatements(FuncClosure &c, const frma::FPXControl *t) {
    emitLoadXControl(c, t);
  }
  inline void _compileStatements(FuncClosure &c, const frma::FPXInfix *i) {
    emitLoadXInfix(c, i);
  }
  inline void _compileStatements(FuncClosure &c, const frma::FPXMember *m) {
    emitLoadXMember(c, m);
  }
  inline void _compileStatements(FuncClosure &c, const frma::FPXPrim *p) {
    emitLoadXPrim(c, p);
  }
  inline void _compileStatements(FuncClosure &c, const frma::FPXUnary *u) {
    emitLoadXUnary(c, u);
  }
  inline void _compileStatements(FuncClosure &c, const frma::FPStmts *s) {
    emitStmts(c, s);
  }
  inline void _compileStatements(FuncClosure &c, const frma::FPStmt *s) {
    emitStmt(c, s);
  }

public:
  inline auto registerAssembly() { return m_assems.emplace(); }

  template <typename T>
  std::uint16_t compileEntryPoint(decltype(m_assems.emplace()) assem,
                                  const T *                    val) {
    FIBytecode  body;
    FuncClosure closure(m_assems.value(assem), body);

    _compileStatements(closure, val);

    if (closure.body()->instructions.empty() ||
        closure.body()
                ->instructions.at(closure.body()->instructions.size() - 1)
                .op != FIOpcode::Ret)
      closure.emit(FIOpcode::Ret);

    return m_assems.value(assem)->funcs().emplace(body);
  }

  std::vector<std::pair<const frma::FPBlock *, std::uint16_t>> compileBlocks(
      std::size_t assem, const frma::FPrims *);

  void dump(std::ostream &os) const;
};

using FIPraeCompiler  = fun::EnableRefCount<_FIPraeCompiler>;
using WFIPraeCompiler = fun::EnableWeakRefCount<_FIPraeCompiler>;
}
