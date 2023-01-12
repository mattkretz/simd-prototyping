/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "../simd_mask2.h"

auto test(simd_mask2<stdx::native_simd<int>> x,
    stdx::native_simd<int> v)
{
  return x + 1;
}
