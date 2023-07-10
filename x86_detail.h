/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_X86_DETAIL_H_
#define PROTOTYPE_X86_DETAIL_H_

#include "detail.h"

#if _GLIBCXX_SIMD_HAVE_SSE
namespace std
{
  namespace __detail
  {
    int __movmsk(__vec_builtin_sizeof<8, 16> auto __x) noexcept
    { return __builtin_ia32_movmskpd(reinterpret_cast<__vdouble128>(__x)); }

    int __movmsk(__vec_builtin_sizeof<8, 32> auto __x) noexcept
    { return __builtin_ia32_movmskpd256(reinterpret_cast<__vdouble256>(__x)); }

    int __movmsk(__vec_builtin_sizeof<4, 16> auto __x) noexcept
    { return __builtin_ia32_movmskps(reinterpret_cast<__vfloat128>(__x)); }

    int __movmsk(__vec_builtin_sizeof<4, 32> auto __x) noexcept
    { return __builtin_ia32_movmskps256(reinterpret_cast<__vfloat256>(__x)); }

    int __movmsk(__vec_builtin auto __x) noexcept
    {
      if constexpr (sizeof(__x) == 16)
        return __builtin_ia32_pmovmskb128(__x);
      else
        return __builtin_ia32_pmovmskb256(__x);
    }
  }
}
#endif  // _GLIBCXX_SIMD_HAVE_SSE
        //
#endif  // PROTOTYPE_X86_DETAIL_H_
