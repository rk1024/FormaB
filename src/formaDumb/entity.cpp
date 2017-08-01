#include "entity.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

#include "builtins.hpp"

namespace frma {
FAtom _FEntity::kw_toString;

_FEntity::_FEntity(const FType &type) : m_type(&type) {}

bool _FEntity::tryDispatch(FAtom keyword, FEntity *ent) {
  if (!kw_toString) kw_toString = FAtom::intern("toString");

  if (keyword == kw_toString) {
    *ent = std::make_shared<_FString>(toString());
    return true;
  }

  return false;
}

bool _FEntity::tryDispatch(std::vector<std::pair<FAtom, FEntity>>, FEntity *) {
  return false;
}

bool _FEntity::tryDispatch(FBinaryOp, FEntity, FEntity *) { return false; }

bool _FEntity::tryRDispatch(FBinaryOp, FEntity, FEntity *) { return false; }

bool _FEntity::tryDispatch(FUnaryOp, FEntity *) { return false; }

FEntity _FEntity::dispatch(FBinaryOp op, FEntity lhs, FEntity rhs) {
  FEntity ent;

  if (lhs && lhs->tryDispatch(op, rhs, &ent) ||
      rhs && rhs->tryRDispatch(op, lhs, &ent))
    return ent;

  throw std::runtime_error("Binary operator not found.");
}

FEntity _FEntity::dispatch(FAtom keyword) {
  FEntity ent;

  if (tryDispatch(keyword, &ent)) return ent;

  throw std::runtime_error("'" + keyword.toString() +
                           "': unary method not found.");
}

FEntity _FEntity::dispatch(std::vector<std::pair<FAtom, FEntity>> keywords) {
  FEntity ent;

  if (tryDispatch(keywords, &ent)) return ent;

  throw std::runtime_error("'" + keywords.at(0).first.toString() +
                           "': keyword method not found.");
}

FEntity _FEntity::dispatch(FUnaryOp op) {
  FEntity ent;

  if (tryDispatch(op, &ent)) return ent;

  throw std::runtime_error("Unary operator not found.");
}

std::string _FEntity::toString() const {
  std::ostringstream oss;
  oss << "<" << m_type->name() << " " << static_cast<const void *>(this) << ">";
  return oss.str();
}

bool _FEntity::toBool() const {
  throw std::runtime_error("Boolean operator not found.");
}
}
