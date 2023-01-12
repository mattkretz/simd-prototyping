/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "../prototype_simd_index/stdx.h"
#include "iterator.h"
#include "simd2.h"
#include <algorithm>

auto test(std::vector<float>& data)
{
  std::span<float, 4> sub(data.begin(), 4);
  simd2 v = sub;
  v += 1;
  return v.to_array();
}

auto foo (stdx::native_simd<int> v)
{
  std::array<int, v.size()> r;
  std::ranges::copy(v + 1 | std::views::transform([](auto x) {
                              return x + 1;
                            }), r.begin());
  return r;
}
