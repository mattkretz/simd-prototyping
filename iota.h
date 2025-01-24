/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_IOTA_H_
#define PROTOTYPE_IOTA_H_

#include "simd.h"

#if not SIMD_IS_A_RANGE
#error "simd_iota is implemented such that it requires simd to be a range"
#endif

namespace std
{
  template <typename _Tp>
    concept __simd_iota_constructible
      = regular<_Tp> and __detail::__vectorizable<_Tp> and (_Tp(1uz) == _Tp() + _Tp(1uz));

  template <typename _Tp>
    constexpr _Tp simd_iota;

  template <__simd_iota_constructible _Tp>
    constexpr _Tp simd_iota<_Tp> = _Tp();

  template <__simd_iota_constructible _Tp, typename _Abi>
    constexpr basic_simd<_Tp, _Abi> simd_iota<basic_simd<_Tp, _Abi>>
      = basic_simd<_Tp, _Abi>([](_Tp __i) -> _Tp { return __i; });
}

#endif  // PROTOTYPE_IOTA_H_
