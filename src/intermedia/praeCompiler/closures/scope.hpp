#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "util/cons.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"

namespace fie {
namespace pc {
  class FuncClosure;

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
