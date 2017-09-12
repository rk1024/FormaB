#pragma once

#include "util/autoref.hpp"

#include "depends.hpp"
#include "depsGraph.hpp"

namespace fps {
template <typename T>
class FTransferAction : public FDepsGraphAction {
  T                      m_data;
  fun::FPtr<FDepends<T>> m_depends;

public:
  FTransferAction(fun::autoref_t<T> data, fun::FPtr<FDepends<T>> depends)
      : m_data(data), m_depends(depends) {}

  virtual void run() override { m_depends->accept(m_data); }
};

template <typename T, typename U>
fun::FPtr<FTransferAction<T>> transfer(T &&data, U &&depends) {
  return fnew<FTransferAction<T>>(
      data, std::forward<fun::FPtr<FDepends<T>>>(depends));
}

template <typename T, typename U>
fun::FPtr<FTransferAction<T *>> transfer(T *data, U &&depends) {
  return fnew<FTransferAction<T *>>(
      std::forward<fun::autoref_t<T *>>(data),
      std::forward<fun::FPtr<FDepends<T *>>>(depends));
}
}
