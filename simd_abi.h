/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_ABI_H_
#define PROTOTYPE_SIMD_ABI_H_

#include <experimental/simd>

namespace std
{
  namespace __detail
  {
    using _SimdSizeType = int;

    template <typename _Tp, _SimdSizeType _Np>
      using __deduce_t
        = typename std::experimental::parallelism_v2::simd_abi::deduce<_Tp, _Np>::type;
  }
}

#endif  // PROTOTYPE_SIMD_ABI_H_

// vim: et
