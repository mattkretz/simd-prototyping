/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_ARM_DETAIL_H_
#define PROTOTYPE_ARM_DETAIL_H_

#include "simd_config.h"

#if _GLIBCXX_SIMD_HAVE_NEON or _GLIBCXX_SIMD_HAVE_SVE
namespace std::__detail
{
  struct _MachineFlags
  {
    uint64_t _M_have_neon : 1 = _GLIBCXX_SIMD_HAVE_NEON;

    uint64_t _M_have_neon_a32 : 1 = _GLIBCXX_SIMD_HAVE_NEON_A32;

    uint64_t _M_have_neon_a64 : 1 = _GLIBCXX_SIMD_HAVE_NEON_A64;

    uint64_t _M_have_sve : 1 = _GLIBCXX_SIMD_HAVE_SVE;

    //uint64_t _M_have_sve2 : 1 = __pv2::__have_sve2;

    uint64_t _M_padding = 0;
  };

  static_assert(sizeof(_MachineFlags) == sizeof(uint64_t) * 2);
}
#endif

#endif // PROTOTYPE_ARM_DETAIL_H_
