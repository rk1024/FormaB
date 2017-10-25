#pragma once

#include <functional>
#include <iomanip>
#include <iostream>

namespace fun {
template <typename T>
class _dumpHex {
  T m_num;

public:
  template <typename T2>
  _dumpHex(T2 num) : m_num(static_cast<T>(num)) {}

  template <typename T2>
  friend std::ostream &operator<<(std::ostream &, const _dumpHex<T2> &&);
};

template <typename T>
std::ostream &operator<<(std::ostream &os, const _dumpHex<T> &&dh) {
  auto f = os.flags();
  os << std::hex << std::setfill('0')
     << std::setw(sizeof(T) / sizeof(std::int8_t) * 2);
  os.flags(f);

  constexpr bool is_char =
      std::is_same<std::make_signed<T>, signed char>::value;

  if (is_char)
    os << static_cast<int>(dh.m_num);
  else
    os << dh.m_num;

  return os;
}

template <typename T>
_dumpHex<T> dumpHex(T num) {
  return _dumpHex<T>(num);
}
}
