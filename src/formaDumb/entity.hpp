#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "atom.hpp"
#include "type.hpp"

namespace frma {
enum class FBinaryOp {
  Add,
  Conjunct,
  Disjunct,
  Div,
  Equal,
  Greater,
  GreaterEq,
  Less,
  LessEq,
  Mod,
  Mul,
  NotEqual,
  Sub,
};

enum class FUnaryOp {
  // Dec,
  // Inc,
  LogNot,
  Neg,
  Pos,
};

class _FEntity;

using FEntity = std::shared_ptr<_FEntity>;

class _FEntity {
  const FType *m_type;

  static FAtom kw_toString;

protected:
  _FEntity(const FType &);

  virtual bool tryDispatch(FAtom, FEntity *);

  virtual bool tryDispatch(std::vector<std::pair<FAtom, FEntity>>, FEntity *);

  virtual bool tryDispatch(FBinaryOp, FEntity, FEntity *);

  virtual bool tryRDispatch(FBinaryOp, FEntity, FEntity *);

  virtual bool tryDispatch(FUnaryOp, FEntity *);

public:
  static FEntity dispatch(FBinaryOp, FEntity, FEntity);

  virtual ~_FEntity() {}

  const FType &type() const { return *m_type; }

  FEntity dispatch(FAtom);

  FEntity dispatch(std::vector<std::pair<FAtom, FEntity>>);

  FEntity dispatch(FUnaryOp);

  virtual std::string toString() const;

  virtual bool toBool() const;

  friend class FSendMessage;
};
}
