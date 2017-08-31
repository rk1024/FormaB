#include "praeCompilerClosures.hpp"

#include <sstream>
#include <string>

using namespace frma;

namespace fie {
namespace pc {
  PositionTracker::PositionTracker(const FormaAST *curr)
      : m_rootNode(fnew<PositionNode>(this, curr)) {
    m_node = m_rootNode;
  }

  fun::FPtr<PositionNode> PositionTracker::move(const frma::FormaAST *to) {
    return m_node.lock()->push(to);
  }

  PositionNode::PositionNode(PositionTracker *     parent,
                             const frma::FormaAST *curr)
      : m_parent(parent), m_curr(curr) {}

  fun::FPtr<PositionNode> PositionNode::push(const frma::FormaAST *node) {
    auto next    = fnew<PositionNode>(m_parent, node);
    next->m_prev = fun::wrap(this);

    if (!m_next) m_parent->m_node = next;

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
      m_parent->m_node = m_prev.lock();
  }

  FuncClosure::FuncClosure(fun::FPtr<AssemblyClosure> assem,
                           FIBytecode &               body,
                           const frma::FormaAST *     curr)
      : PositionTracker(curr),
        m_assem(assem),
        m_scope(fnew<ScopeClosure>(this, nullptr)),
        m_body(&body) {}

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

  void FuncClosure::beginScope() {}

  void FuncClosure::endScope() {}

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

  inline std::string assembleName(const std::string &base, int count) {
    std::ostringstream oss;
    oss << base;
    if (count >= 0) oss << "`" << count;
    return oss.str();
  }

  std::uint32_t ScopeClosure::bind(const std::string &name, bool mut) {
    if (m_counts.find(name) != m_counts.end()) goto declared;

    m_counts[name] = mut ? 0 : -1;

    {
      std::uint32_t id;
      if (!m_func->m_body->vars.intern(assembleName(name, m_counts[name]), &id))
        goto declared;

      return id;
    }
  declared:
    m_func->error("variable '" + name + "' already declared");
  }

  std::uint32_t ScopeClosure::get(const std::string &name, bool set) {
    auto it = m_counts.find(name);
    if (it == m_counts.end()) goto notDeclared;

    {
      int count = (*it).second;

      if (set) {
        if (count < 0) m_func->error("variable '" + name + "' is not mutable");

        m_counts[name] = ++count;
      }

      std::uint32_t id;

      if (!m_func->m_body->vars.intern(assembleName(name, count), &id) && set)
        goto declared;

      return id;
    declared:
      m_func->error("variable '" + assembleName(name, count) +
                    "' already registered. (this shouldn't happen)");
    }

    assert(false);
  notDeclared:
    m_func->error("variable '" + name + "' not declared");
  }
}
}
