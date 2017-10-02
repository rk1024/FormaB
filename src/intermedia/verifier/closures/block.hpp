#pragma once

#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "intermedia/assembly.hpp"
#include "intermedia/function.hpp"

namespace fie {
namespace vc {
  class BlockClosure : public fun::FObject {
    fun::FPtr<FIAssembly>                m_assem;
    fun::FPtr<const FIFunction>          m_func;
    std::queue<fun::FPtr<BlockClosure>> *m_q;
    std::size_t                          m_pc = 0;
    std::stack<std::uint32_t>            m_stack;
    std::unordered_map<std::uint32_t, std::uint32_t> m_vars;

    bool assertArity(std::uint32_t, const char *);

    void handlePHOp(std::uint32_t, fun::FPtr<FIStruct>, const char *);

    void handleBoolOp(std::uint32_t, const char *);

    bool handlePopBool(const char *);

  public:
    BlockClosure(fun::FPtr<FIAssembly>,
                 fun::FPtr<const FIFunction>,
                 std::queue<fun::FPtr<BlockClosure>> *);

    BlockClosure(fun::FPtr<BlockClosure>, std::size_t);

    void iterate();
  };
}
}
