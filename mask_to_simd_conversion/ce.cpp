/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
// Copyright © 2023–2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

#include "../simd_mask.h"

auto test(std::simd_mask<int> x, std::simd<int> v)
{
  return -x + v;
}
