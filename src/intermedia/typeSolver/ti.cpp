/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (ti.cpp)
 * Copyright (C) 2017-2018 Ryan Schroeder, Colin Unger
 *
 * FormaB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * FormaB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with FormaB.  If not, see <https://www.gnu.org/licenses/>.
 *
 ************************************************************************/

#include "ti.hpp"

namespace fie {
bool WAcceptsMessageSet::validate(const fun::FRef<const w::TypeBase> &t) const {
  return true; // TODO: actually validate this?
  return false;
}

bool WAcceptsMessageSet::includes(const w::TypeVars &v) const { return true; }

std::string WAcceptsMessageSet::to_string() const {
  return "Accepts(" + m_msg.name() + ")";
}

bool WAcceptsMessageSet::operator==(const WAcceptsMessageSet &rhs) const {
  return m_msg == rhs.m_msg;
}

WAcceptsMessage::UnifyResult WAcceptsMessage::mgu(
    const CustomType &                  self,
    const fun::FPtr<const w::TypeBase> &rhs,
    w::TIBase &                         t) const {
  // TODO: Checking structs by name is super sketch
  if (auto strukt = rhs.as<const WTypeMessageStruct>();
      strukt && strukt->base()->name() == "fun" &&
      strukt->base()->params() == fun::cons(m_msg) &&
      strukt->params().size() == self.m_params.size()) {

    auto s = m_receiver->mgu(rhs, t);

    for (int i = 0; i < self.m_params.size(); ++i) {
      auto s2 = w::sub(s, self.m_params[i])
                    ->mgu(w::sub(s, strukt->params()[i]), t);
      s = w::composeSubst(s2, s);
    }

    return UnifyResult(true, s);
  }
  else if (auto accepts = rhs.as<const WAcceptsMessageType>();
           accepts && m_msg == accepts->m_base->m_msg) {
    auto s = m_receiver->mgu(accepts->m_base->m_receiver, t);

    for (int i = 0; i < self.m_params.size(); ++i) {
      auto s2 = w::sub(s, self.m_params[i])
                    ->mgu(w::sub(s, accepts->m_params[i]), t);
      s = w::composeSubst(s2, s);
    }

    return UnifyResult(true, s);
  }
  else
    return UnifyResult(false, {});
}

std::string WAcceptsMessage::to_string() const {
  return "Accepts(" +
#if defined(DEBUG)
         m_receiver->to_string() + ", " +
#endif
         m_msg.name() + ")";
}

bool WAcceptsMessage::operator==(const WAcceptsMessage &rhs) const {
  return m_msg == rhs.m_msg;
}
} // namespace fie

namespace std {
size_t hash<fie::WAcceptsMessageSet>::operator()(
    const fie::WAcceptsMessageSet &set) const {
  return msgHash(set.m_msg);
}

size_t hash<fie::WAcceptsMessage>::operator()(
    const fie::WAcceptsMessage &accept) const {
  return msgHash(accept.m_msg);
}
} // namespace std

namespace w {
TypeVars Types<fie::WAcceptsMessageSet>::__ftv(
    const fie::WAcceptsMessageSet &set) {
  return {};
}

fie::WAcceptsMessageSet Types<fie::WAcceptsMessageSet>::__sub(
    const Subst &s, const fie::WAcceptsMessageSet &set) {
  return set;
}
} // namespace w
