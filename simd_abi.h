/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_ABI_H_
#define PROTOTYPE_SIMD_ABI_H_

#include <experimental/simd>

namespace std
{
  namespace simd_abi
  {
    using scalar = std::experimental::parallelism_v2::simd_abi::scalar;

    template <typename _Tp>
      using native = std::experimental::parallelism_v2::simd_abi::native<_Tp>;

    template <typename _Tp, size_t _Np>
      using fixed_size
        = typename std::experimental::parallelism_v2::simd_abi::deduce<_Tp, _Np>::type;

    template <typename _Tp>
      inline constexpr size_t max_fixed_size
        = std::experimental::parallelism_v2::simd_abi::max_fixed_size<_Tp>;
  }
}

#endif  // PROTOTYPE_SIMD_ABI_H_

// vim: et
