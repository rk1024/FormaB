#pragma once

#include <cassert>
#include <cstdint>
#include <unordered_map>

#include "ast/astBase.hpp"

#include "util/atom.hpp"

#include "function.hpp"
#include "opcode.hpp"

namespace fie {
namespace pc {
  class AssemblyClosure;
  class _FuncClosure;
  class ScopeClosure;

  class AssemblyClosure : public fun::FObject {
    fun::FAtomStore<fun::FPtr<FIFunction>, std::uint16_t> m_funcs;

  public:
    auto funcs() -> decltype(m_funcs) & { return m_funcs; }
    auto funcs() const -> const decltype(m_funcs) & { return m_funcs; }
  };

  class PositionTracker;

  class PositionNode : public fun::FObject {
    PositionTracker *           m_parent;
    const frma::FormaAST *      m_curr;
    fun::FPtr<PositionNode>     m_prev;
    fun::FWeakPtr<PositionNode> m_next;

  public:
    const frma::FormaAST *ast() const { return m_curr; }

    PositionNode(PositionTracker *, const frma::FormaAST *);

    fun::FPtr<PositionNode> push(const frma::FormaAST *);

    virtual ~PositionNode() override;

    friend class PositionTracker;
  };

  class PositionTracker : public fun::FObject {
    fun::FPtr<PositionNode> m_node;

  public:
    const frma::FormaAST *curr() const { return m_node->m_curr; }

    PositionTracker(const frma::FormaAST *);

    fun::FPtr<PositionNode> move(const frma::FormaAST *);

    friend class PositionNode;
  };

  class _FuncClosure : public PositionTracker {
    fun::FWeakPtr<AssemblyClosure> m_assem;
    fun::FPtr<ScopeClosure>        m_scope;
    FIBytecode *                   m_body;

  public:
    inline fun::FPtr<AssemblyClosure> assem() const { return m_assem.lock(); }
    inline fun::FPtr<ScopeClosure>    scope() const { return m_scope; }
    inline FIBytecode *               body() const { return m_body; }

    _FuncClosure(fun::FPtr<AssemblyClosure>,
                 FIBytecode &,
                 const frma::FormaAST *);

    _FuncClosure &emit(FIInstruction);

    inline _FuncClosure &emit(FIOpcode op) { return emit(FIInstruction(op)); }

    template <typename T>
    inline
        typename std::enable_if<std::is_integral<T>::value, _FuncClosure>::type
            &
            emit(FIOpcode op, T arg) {
      return emit(FIInstruction(op, arg));
    }

    std::uint16_t beginLabel();

    void label(std::uint16_t);

    void beginScope();
    void endScope();

    [[noreturn]] void error(std::string &&desc);

    friend class ScopeClosure;
  };

  class ScopeClosure : public fun::FObject {
    _FuncClosure *              m_func;
    fun::FWeakPtr<ScopeClosure> m_parent;

    std::unordered_map<std::string, int> m_counts;

  public:
    inline fun::FPtr<ScopeClosure> parent() const { return m_parent.lock(); }

    ScopeClosure(_FuncClosure *func, fun::FPtr<ScopeClosure> parent)
        : m_func(func), m_parent(parent) {}

    std::uint32_t bind(const std::string &, bool);
    std::uint32_t get(const std::string &, bool);
  };
}
}
