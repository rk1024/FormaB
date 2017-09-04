#pragma once

#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include "ast/astBase.hpp"

#include "util/atom.hpp"
#include "util/cons.hpp"

#include "bytecode.hpp"
#include "function.hpp"

namespace fie {
namespace pc {
#define INITFLAGS(type)                                                        \
  namespace type {                                                             \
    enum Flags : std::uint16_t;                                                \
                                                                               \
    inline constexpr Flags operator|(Flags a, Flags b) {                       \
      return static_cast<Flags>(static_cast<std::uint16_t>(a) |                \
                                static_cast<std::uint16_t>(b));                \
    }                                                                          \
                                                                               \
    inline constexpr Flags operator&(Flags a, Flags b) {                       \
      return static_cast<Flags>(static_cast<std::uint16_t>(a) &                \
                                static_cast<std::uint16_t>(b));                \
    }                                                                          \
                                                                               \
    inline constexpr Flags operator~(Flags a) {                                \
      return static_cast<Flags>(~static_cast<std::uint16_t>(a));               \
    }                                                                          \
  }

  INITFLAGS(ParenFlags)

  namespace ParenFlags {
    enum Flags : std::uint16_t {
      Bind  = 1,
      Eval  = 2,
      Scope = 4,

      Default = Bind | Eval | Scope,

      NoBind  = Default & ~Bind,
      NoEval  = Default & ~Eval,
      NoScope = Default & ~Scope,
    };
  }

  class AssemblyClosure;
  class FuncClosure;
  class ScopeClosure;

  class AssemblyClosure : public fun::FObject {
    fun::FAtomStore<fun::FPtr<FIFunction>, std::uint16_t> m_funcs;
    fun::FAtomStore<std::string, std::uint32_t>           m_msgs, m_strings;

  public:
    auto funcs() -> decltype(m_funcs) & { return m_funcs; }
    auto funcs() const -> const decltype(m_funcs) & { return m_funcs; }

    auto msgs() -> decltype(m_msgs) & { return m_msgs; }
    auto strings() -> decltype(m_strings) & { return m_strings; }
  };

  class PositionTracker;

  class PositionNode : public fun::FObject {
    PositionTracker *           m_parent;
    const frma::FormaAST *      m_curr;
    fun::FWeakPtr<PositionNode> m_prev, m_next;

  public:
    const frma::FormaAST *ast() const { return m_curr; }

    PositionNode(PositionTracker *, const frma::FormaAST *);

    fun::FPtr<PositionNode> push(const frma::FormaAST *);

    virtual ~PositionNode() override;

    friend class PositionTracker;
  };

  class PositionTracker : public fun::FObject {
    fun::FWeakPtr<PositionNode> m_node;
    fun::FPtr<PositionNode>     m_rootNode;

  public:
    const frma::FormaAST *curr() const { return m_node.lock()->m_curr; }

    PositionTracker(const frma::FormaAST *);

    fun::FPtr<PositionNode> move(const frma::FormaAST *);

    friend class PositionNode;
  };

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

  class ScopeClosure : public fun::FObject {
  public:
    using VarInfo = fun::cons_cell<fun::FWeakPtr<ScopeClosure>,
                                   std::string,
                                   unsigned int,
                                   unsigned int>;

    using OwnVarInfo = fun::cons_cell<std::string, unsigned int, unsigned int>;

  private:
    static const unsigned int ID_NONE = 0xffffffff, ID_MAX = 0xfffffffe;
    static const unsigned int COUNT_CONST = 0xffffffff, COUNT_MAX = 0xfffffffe;

    fun::FWeakPtr<FuncClosure> m_func;
    fun::FPtr<ScopeClosure>    m_parent;
    unsigned int               m_id;
    std::unordered_map<std::string, fun::cons_cell<unsigned int, unsigned int>>
        m_vars;
    std::unordered_map<std::string, fun::FWeakPtr<ScopeClosure>> m_borrowed;

    std::string assembleName(const std::string &, unsigned int, unsigned int);

    template <bool>
    fun::FPtr<ScopeClosure> holderOf(const std::string &);

    std::uint32_t recordName(const std::string &, unsigned int, unsigned int);

    std::uint32_t recordVar(fun::FWeakPtr<ScopeClosure>,
                            const std::string &,
                            unsigned int,
                            unsigned int,
                            bool);

  public:
    inline fun::FPtr<ScopeClosure> parent() const { return m_parent; }

    ScopeClosure(fun::FWeakPtr<FuncClosure>, fun::FPtr<ScopeClosure>);

    std::uint32_t bind(const std::string &, bool mut);
    std::uint32_t get(const std::string &);
    std::uint32_t set(const std::string &);
    std::uint32_t phi(fun::FPtr<ScopeClosure>, const std::string &);

    std::vector<VarInfo>    getModified();
    std::vector<OwnVarInfo> getOwned();
  };
}
}
