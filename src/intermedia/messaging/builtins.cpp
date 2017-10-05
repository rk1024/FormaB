#include "builtins.hpp"

namespace fie {
namespace builtins {
  FIMessage FIAdd(2, "o@op:+:"), FISub(2, "o@op:-:"), FIMul(2, "o@op:*:"),
      FIDiv(2, "o@op:/:"), FIMod(2, "o@op:%:"),

      FINeg(1, "o@-"), FIPos(1, "o@+"),

      FICeq(2, "o@op:==:"), FICgt(2, "o@op:>:"), FIClt(2, "o@op:<:"),

      FICon(2, "o@op:&&:"), FIDis(2, "o@op:||:"),

      FIInv(1, "o@!"),

      FICast(2, "o@op:as:"),

      FICurry(3, "o`c@?"), FICoerce(1, "o`c@!");
}

static std::vector<FIMessage> builtin_vec;

const std::vector<FIMessage> &fiBuiltinMsgs() {
  if (builtin_vec.empty()) {
    builtin_vec.push_back(builtins::FIAdd);
    builtin_vec.push_back(builtins::FISub);
    builtin_vec.push_back(builtins::FIMul);
    builtin_vec.push_back(builtins::FIDiv);
    builtin_vec.push_back(builtins::FIMod);

    builtin_vec.push_back(builtins::FINeg);
    builtin_vec.push_back(builtins::FIPos);

    builtin_vec.push_back(builtins::FICeq);
    builtin_vec.push_back(builtins::FICgt);
    builtin_vec.push_back(builtins::FIClt);

    builtin_vec.push_back(builtins::FICon);
    builtin_vec.push_back(builtins::FIDis);

    builtin_vec.push_back(builtins::FIInv);

    builtin_vec.push_back(builtins::FICast);

    builtin_vec.push_back(builtins::FICurry);
    builtin_vec.push_back(builtins::FICoerce);
  }

  return builtin_vec;
}
}
