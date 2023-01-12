/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "../simd.h"

static_assert( std::convertible_to<     int, std::simd<short>>);
static_assert(!std::convertible_to<unsigned, std::simd<short>>);
static_assert( std::convertible_to<     int, std::simd<unsigned short>>);
static_assert( std::convertible_to<unsigned, std::simd<unsigned short>>);

static_assert( std::convertible_to<std::rebind_simd_t<char, std::simd<short>>, std::simd<short>>);
static_assert(!std::convertible_to<std::simd<short>, std::simd<char>>);
static_assert( std::convertible_to<std::simd<long>, std::simd<long long>>);
static_assert(!std::convertible_to<std::simd<long long>, std::simd<long>>);

void f(std::common_type_t<std::rebind_simd_t<char, std::simd<short>>, std::simd<signed short>>)
{

}
