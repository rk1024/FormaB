#pragma once

#include <string>
#include <vector>

#include "closure.hpp"
#include "entity.hpp"

namespace frma {
class FPExpr;
class FPXFunc;
class FPFuncParams;
class FPFuncParam;
class FDumbInterpreter;

class _FNumber;
class _FBool;
class _FString;
class _FFunction;
class _FCout;

using FNumber   = std::shared_ptr<_FNumber>;
using FBool     = std::shared_ptr<_FBool>;
using FString   = std::shared_ptr<_FString>;
using FFunction = std::shared_ptr<_FFunction>;
using FCout     = std::shared_ptr<_FCout>;

class _FNumber : public _FEntity {
  static FAtom kw_exp;

  double m_num;

  virtual bool tryDispatch(std::vector<std::pair<FAtom, FEntity>>,
                           FEntity *) override;

  virtual bool tryDispatch(FBinaryOp, FEntity, FEntity *) override;

  virtual bool tryRDispatch(FBinaryOp, FEntity, FEntity *) override;

  virtual bool tryDispatch(FUnaryOp, FEntity *) override;

public:
  const FType type = FType("Number", 0x99d50a022df94b80, 0xb07ed08d57f30c5d);

  _FNumber(double);

  virtual std::string toString() const override;
};

class _FBool : public _FEntity {
  static FBool s_true, s_false;

  bool m_bool;

  _FBool(bool);

public:
  const FType type = FType("Boolean", 0x0d726c41db134e45, 0x9dd9698a7efe7b5f);

  static const FBool True();

  static const FBool False();

  static const FBool get(bool);

  virtual std::string toString() const override;

  virtual bool toBool() const override;
};

class _FString : public _FEntity {
  static FAtom kw_replace, kw_with, kw_toUpper, kw_toLower;

  std::string m_str;

  virtual bool tryDispatch(FAtom, FEntity *) override;

  virtual bool tryDispatch(std::vector<std::pair<FAtom, FEntity>>,
                           FEntity *) override;

  virtual bool tryDispatch(FBinaryOp, FEntity, FEntity *) override;

  virtual bool tryRDispatch(FBinaryOp, FEntity, FEntity *) override;

public:
  const FType type = FType("String", 0x0ce87f3bde3a4244, 0x9645837d8b04913c);

  _FString(const std::string &);

  virtual std::string toString() const override;
};

class _FFunction : public _FEntity {
  static FAtom kw_call;

  FDumbInterpreter * m_interp;
  std::vector<FAtom> m_params;
  const FPExpr *     m_expr;
  FWClosure          m_closure;

  virtual bool tryDispatch(FAtom, FEntity *) override;

  virtual bool tryDispatch(std::vector<std::pair<FAtom, FEntity>>,
                           FEntity *) override;

  void getParams(const FPFuncParams *);

  void getParams(const FPFuncParam *);

public:
  const FType type = FType("Function", 0x6aa332f726ac48ea, 0xafd3c7ab8758a4d4);

  _FFunction(FDumbInterpreter *, const FPXFunc *, FClosure);
};

class _FCout : public _FEntity {
  static FCout s_inst;

  static FAtom kw_print, kw_test;

  _FCout();

  virtual bool tryDispatch(std::vector<std::pair<FAtom, FEntity>>,
                           FEntity *) override;

public:
  const FType type = FType("CoutType", 0x5134f880d7a24a93, 0xb5185a8ba129c446);

  static const FCout instance();
};
}
