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
  FEntity run(const FPBlock *, FClosure);
  FEntity run(const FPStmts *, FClosure);
  FEntity run(const FPStmt *, FClosure);
  FEntity run(const FPExpr *, FClosure);
  FEntity run(const FPXControl *, FClosure);
  void    run(const FPSBind *, FClosure);
  void run(const FPBindings *, FClosure, bool mut);
  void run(const FPBinding *, FClosure, bool mut);
  FEntity run(const FPSAssign *, FClosure);
  FEntity run(const FPAssignValue *, FClosure);
  FEntity run(const FPXInfix *, FClosure);
  FEntity run(const FPXUnary *, FClosure);
  FEntity run(const FPXMember *, FClosure);
  FEntity run(const FPXPrim *, FClosure);
  FEntity run(const FPXMsg *, FClosure);
  FEntity run(const FPXParen *, FClosure);

  FEntity runScopedWhere(const FPXParen *, FClosure);

  // void getSelectors(const FPMsgSelectors *,
  //                   std::vector<const FPMsgSelector *> &);

  void getKeywords(const FPMsgKeywords *,
                   std::vector<std::pair<FAtom, FEntity>> &,
                   FClosure closure);

  FBool getBoolean(const FPLBoolean *boolean) const;

  void assignLval(const FPXMember *, FEntity, FClosure);

public:
  FDumbInterpreter(const FPrims *);

  void run();

  friend class _FFunction;
};
}
