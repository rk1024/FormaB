#pragma once

#include <cstdint>
#include <functional>

#include "hashing.hpp"

namespace fun {
template <typename...>
struct cons_cell;

template <std::size_t, typename, typename...>
struct cons_get;

template <>
struct cons_cell<> {
  constexpr bool operator==(const cons_cell &) const { return true; }
  constexpr bool operator!=(const cons_cell &) const { return false; }
  constexpr bool operator<(const cons_cell &) const { return false; }
  constexpr bool operator>(const cons_cell &) const { return false; }
  constexpr bool operator<=(const cons_cell &) const { return false; }
  constexpr bool operator>=(const cons_cell &) const { return false; }
};

template <typename TCar, typename... TCdr>
struct cons_cell<TCar, TCdr...> {
  template <std::size_t index>
  using select_t = cons_get<index, TCar, TCdr...>;

  TCar               car;
  cons_cell<TCdr...> cdr;

  cons_cell() = default;

  cons_cell(const TCar &_car, const TCdr &... _cdr) : car(_car), cdr(_cdr...) {}

  template <std::size_t idx>
  constexpr auto &      get() {
    return select_t<idx>::get(*this);
  }

  template <std::size_t idx>
  constexpr const auto &get() const {
    return select_t<idx>::get(*this);
  }

#define CONS_COMPARE_A(op, inv)                                                \
  bool operator op(const cons_cell &rhs) const {                               \
    if (car inv rhs.car) return false;                                         \
    return true;                                                               \
  }

#define CONS_COMPARE_E(op)                                                     \
  bool operator op(const cons_cell &rhs) const {                               \
    if (car op rhs.car) return true;                                           \
    return false;                                                              \
  }

  CONS_COMPARE_A(==, !=)
  CONS_COMPARE_A(!=, ==)

  CONS_COMPARE_E(<)
  CONS_COMPARE_E(>)
  CONS_COMPARE_E(<=)
  CONS_COMPARE_E(>=)

#undef CONS_COMPARE_A
#undef CONS_COMPARE_E
};

template <std::size_t index, typename TCar, typename... TCdr>
struct cons_get {
  using next_t = std::enable_if_t<(index > 0), cons_get<index - 1, TCdr...>>;

  using item_t = typename next_t::item_t;

  static constexpr item_t &get(cons_cell<TCar, TCdr...> &cell) {
    return next_t::get(cell.cdr);
  }

  static constexpr const item_t &get(const cons_cell<TCar, TCdr...> &cell) {
    return next_t::get(cell.cdr);
  }
};

template <typename TCar, typename... TCdr>
struct cons_get<0, TCar, TCdr...> {
  using item_t = TCar;

  static constexpr item_t &get(cons_cell<TCar, TCdr...> &cell) {
    return cell.car;
  }

  static constexpr const item_t &get(const cons_cell<TCar, TCdr...> &cell) {
    return cell.car;
  }
};

template <typename... TArgs>
cons_cell<TArgs...> cons(TArgs... args) {
  return cons_cell<TArgs...>(std::forward<TArgs>(args)...);
}

template <typename... TArgs>
const cons_cell<TArgs...> cons_const(TArgs... args) {
  return cons_cell<TArgs...>(std::forward<TArgs>(args)...);
}

template <typename... TItems>
constexpr void _combineConsHashes(std::size_t &, const cons_cell<TItems...> &);

template <>
constexpr void _combineConsHashes(std::size_t &, const cons_cell<> &) {}

template <typename TCar, typename... TCdr>
constexpr void _combineConsHashes(std::size_t &seed,
                                  const cons_cell<TCar, TCdr...> &cell) {
  combineHashes(seed, cell.car);
  _combineConsHashes<TCdr...>(seed, cell.cdr);
}

template <typename... TItems>
inline std::size_t consHash(const cons_cell<TItems...> &cell) {
  auto ret = MULTIHASH_EMPTY;
  _combineConsHashes<TItems...>(ret, cell);
  return ret;
}
}

namespace std {
template <typename... TItems>
struct hash<fun::cons_cell<TItems...>> {
  std::size_t operator()(const fun::cons_cell<TItems...> &cell) const {
    return fun::consHash(cell);
  }
};
}
