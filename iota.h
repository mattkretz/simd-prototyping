/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_IOTA_H_
#define PROTOTYPE_IOTA_H_

#include "simd.h"
#include <array>

namespace std
{
  namespace __detail
  {
    template <typename _Tp, typename>
      struct __iota_array;

    template <typename _Tp, std::size_t... _Is>
      struct __iota_array<_Tp, std::index_sequence<_Is...>>
      {
        static constexpr _Tp __data[sizeof...(_Is)] = {static_cast<_Tp>(_Is)...};
      };
  }

  template <typename _Tp>
    inline constexpr _Tp
    iota_v;

  template <typename _Tp>
    requires(std::is_arithmetic_v<_Tp>)
    inline constexpr _Tp
    iota_v<_Tp> = _Tp();

  template <__detail::__simd_type _Tp>
    inline constexpr _Tp
    iota_v<_Tp> = _Tp([](int __i) { return static_cast<typename _Tp::value_type>(__i); });

  template <typename _Tp, std::size_t _Np>
    inline constexpr std::array<_Tp, _Np>
    iota_v<std::array<_Tp, _Np>> = []<std::size_t... _Is>(std::index_sequence<_Is...>) {
                                   return std::array<_Tp, _Np>{static_cast<_Tp>(_Is)...};
                                 }(std::make_index_sequence<_Np>());

  template <typename _Tp, std::size_t _Np>
    inline constexpr auto&
    iota_v<_Tp[_Np]>
      = __detail::__iota_array<_Tp, decltype(std::make_index_sequence<_Np>())>::__data;
}

#endif  // PROTOTYPE_IOTA_H_
