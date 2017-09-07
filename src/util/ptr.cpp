#include "ptr.hpp"

namespace fun {
#if defined(FPTR_DIAGNOSTIC)
FAtomStore<std::string> __fptr_t_ids;
std::unordered_map<std::size_t, FAtomStore<const void *>> __fptr_ids;
std::unordered_map<std::size_t, FAtomStore<const void *>> __fptr_o_ids;
unsigned int __fptr_indent_lvl;

void __fptr_clr_id(std::ostream &os, std::size_t id, bool big) {
  os << "\e[38;5;" << (((big ? id : id * 8) % 179) + 52) << "m";
}

void __fptr_log_id(std::ostream &os, std::size_t id, bool big) {
  __fptr_clr_id(os, id, big);
  os << std::setw(7) << std::right << std::setfill(' ') << id << "\e[39m";
}
#endif
}
