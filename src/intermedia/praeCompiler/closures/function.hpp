#pragma once

#include <unordered_map>

#include "util/cons.hpp"

#include "intermedia/bytecode.hpp"

#include "assembly.hpp"
#include "position.hpp"

namespace fie {
namespace pc {
  class ScopeClosure;

  class FuncClosure : public PositionTracker {
  public:
    using VarIds = std::unordered_map<
        fun::cons_cell<fun::FWeakPtr<ScopeClosure>, std::string>,
        std::uint32_t>;

  private:
    fun::FWeakPtr<AssemblyClosure> m_assem;
    fun::FPtr<ScopeClosure>        m_scope;
    unsigned int                   m_nextScopeId = 0;
    FIBytecode *                   m_body;

  public:
    inline fun::FPtr<AssemblyClosure> assem() const { return m_assem.lock(); }
    inline fun::FPtr<ScopeClosure>    scope() const { return m_scope; }
    inline FIBytecode *               body() const { return m_body; }

    FuncClosure(fun::FPtr<AssemblyClosure>,
                FIBytecode &,
                const frma::FormaAST *);

    FuncClosure &emit(FIInstruction);

    inline FuncClosure &emit(FIOpcode op) { return emit(FIInstruction(op)); }

    template <typename T>
    inline std::enable_if_t<std::is_integral<T>::value ||
                                std::is_floating_point<T>::value,
                            FuncClosure>
        &emit(FIOpcode op, T arg) {
      return emit(FIInstruction(op, arg));
    }

    std::uint16_t beginLabel();

    void label(std::uint16_t);

    void                    pushScope();
    void                    dropScope();
    fun::FPtr<ScopeClosure> popScope();
    void                    applyScope();
    void applyScopeWithIds(VarIds &, bool add);
    VarIds applyScopeWithIds();

    [[noreturn]] void error(std::string &&desc);

    friend class ScopeClosure;
  };
}
}
