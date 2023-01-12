/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "stdx.h"
#include "reference.h"
#include "iterator.h"
#include "simd_index.h"

#include <memory>
#include <span>

auto
f (std::span<float> data)
{
  // optional: assume alignment of span ptr
  data = std::span<float>(std::assume_aligned<64>(data.data()), data.size());

  using V = stdx::native_simd<float>;
  using Idx = simd_index<V>;
  data[Idx(2)] = 2;

  for (Idx i = 0; i < data.size(); ++i)
    data[i] += 1;

  Idx i = 0;
  V acc = data[i];
  for (++i; i > 0 && i < data.size (); ++i)
    acc += data[i];
  acc += i.masked_copy_from(data);
  i.masked_copy_to(data, acc);
  return acc;
}
