#pragma once

#include <vector>

#include "ast.hpp"
#include "builtins.hpp"
#include "closure.hpp"
#include "entity.hpp"

namespace frma {
class FDumbInterpreter {
  const FPrims *m_prims;

  FEntity run(const FPrims *, FClosure);
  FEntity run(const FPrim *, FClosure);
  FEntity run(const FMBlock *, FClosure);
  FEntity run(const FMStmts *, FClosure);
  FEntity run(const FMStmt *, FClosure);
  FEntity run(const FMExpr *, FClosure);
  FEntity run(const FMXControl *, FClosure);
  void    run(const FMSBind *, FClosure);
  void run(const FMBindings *, FClosure, bool mut);
  void run(const FMBinding *, FClosure, bool mut);
  FEntity run(const FMSAssign *, FClosure);
  FEntity run(const FMAssignValue *, FClosure);
  FEntity run(const FMXInfix *, FClosure);
  FEntity run(const FMXUnary *, FClosure);
  FEntity run(const FMXMember *, FClosure);
  FEntity run(const FMXPrim *, FClosure);
  FEntity run(const FMXMsg *, FClosure);
  FEntity run(const FMXParen *, FClosure);

  FEntity runScopedWhere(const FMXParen *, FClosure);

  void getSelectors(const FMMsgSelectors *,
                    std::vector<const FMMsgSelector *> &);

  void getKeywords(const FMMsgKeywords *,
                   std::vector<std::pair<FAtom, FEntity>> &,
                   FClosure closure);

  FBool getBoolean(const FMXBoolean *boolean) const;

  void assignLval(const FMXMember *, FEntity, FClosure);

public:
  FDumbInterpreter(const FPrims *);

  void run();

  friend class _FFunction;
};
}
