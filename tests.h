/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "interleave.h"
#include "permute.h"

namespace test01
{
  using namespace std::__detail;

  static_assert(std::same_as<__nopromot_common_type_t<short, signed char>, short>);
  static_assert(std::same_as<__nopromot_common_type_t<short, unsigned char>, short>);
  static_assert(std::same_as<__nopromot_common_type_t<short, unsigned short>, unsigned short>);
  static_assert(std::same_as<__nopromot_common_type_t<short, char>, short>);

  static_assert(    __non_narrowing_constexpr_conversion<_Ic< 1>, float>);
  static_assert(    __non_narrowing_constexpr_conversion<_Ic< 1>, unsigned short>);
  static_assert(not __non_narrowing_constexpr_conversion<_Ic<-1>, unsigned short>);
  static_assert(    __non_narrowing_constexpr_conversion<_Ic<1.f>, unsigned short>);
  static_assert(    __non_narrowing_constexpr_conversion<_Ic<1.>, float>);
  static_assert(not __non_narrowing_constexpr_conversion<_Ic<1.1>, float>);
  static_assert(    __non_narrowing_constexpr_conversion<_Ic<1.1f>, double>);


  static_assert(    __broadcast_constructible<_Ic<1>, float>);
  static_assert(    __broadcast_constructible<_Ic<1.1f>, double>);
  static_assert(not __broadcast_constructible<_Ic<1.1>, float>);

  static_assert(__value_preserving_convertible_to<bool, bool>);
  static_assert(__broadcast_constructible<bool, bool>);
  static_assert(__simd_broadcast_invokable<decltype([] (int) { return true; }), bool, 4>);
}

template <auto X>
  using Ic = std::__detail::_Ic<X>;

static_assert(    std::convertible_to<Ic<1>, std::simd<float>>);
static_assert(not std::convertible_to<Ic<1.1>, std::simd<float>>);
static_assert(not std::convertible_to<std::simd<int, 4>, std::simd<float, 4>>);
static_assert(not std::convertible_to<std::simd<float, 4>, std::simd<int, 4>>);
static_assert(    std::convertible_to<std::simd<int, 4>, std::simd<double, 4>>);

static_assert(
  all_of(std::simd_mask<float, 4>([] (int) { return true; }) == std::simd_mask<float, 4>(true)));
static_assert(
  all_of(std::simd_mask<float, 4>([] (int) { return false; }) == std::simd_mask<float, 4>(false)));

static_assert(
  all_of(std::get<0>(std::interleave(std::iota_v<std::simd<int>>))
	   == std::iota_v<std::simd<int>>));

static_assert(
  all_of(std::get<0>(std::interleave(std::simd<int>(0), std::simd<int>(1)))
	   == (std::iota_v<std::simd<int>> & 1)));

static_assert(
  all_of(std::get<1>(std::interleave(std::simd<int>(0), std::simd<int>(1)))
	   == (std::iota_v<std::simd<int>> & 1)));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int>>, std::simd_permutations::duplicate_even)
	   == std::iota_v<std::simd<int>> / 2 * 2));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int>>, std::simd_permutations::duplicate_odd)
	   == std::iota_v<std::simd<int>> / 2 * 2 + 1));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int>>, std::simd_permutations::swap_neighbors<1>)
	   == std::simd<int>([](int i) { return i ^ 1; })));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int, 8>>,
		      std::simd_permutations::swap_neighbors<2>)
	   == std::simd<int, 8>(std::array{2, 3, 0, 1, 6, 7, 4, 5}.begin())));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int, 12>>,
		      std::simd_permutations::swap_neighbors<3>)
	   == std::simd<int, 12>(
		std::array{3, 4, 5, 0, 1, 2, 9, 10, 11, 6, 7, 8}.begin())));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int>>, std::simd_permutations::broadcast<1>)
	   == std::simd<int>(1)));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int>>, std::simd_permutations::broadcast_first)
	   == std::simd<int>(0)));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int>>, std::simd_permutations::broadcast_last)
	   == std::simd<int>(int(std::simd_size_v<int> - 1))));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int>>, std::simd_permutations::reverse)
	   == std::simd<int>([](int i) { return int(std::simd_size_v<int>) - 1 - i; })));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int>>, std::simd_permutations::rotate<1>)
	   == (std::iota_v<std::simd<int>> + 1) % int(std::simd_size_v<int>)));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int>>, std::simd_permutations::rotate<2>)
	   == (std::iota_v<std::simd<int>> + 2) % int(std::simd_size_v<int>)));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int, 7>>, std::simd_permutations::rotate<2>)
           == std::simd<int, 7>(std::array {2, 3, 4, 5, 6, 0, 1}.begin())));

static_assert(
  all_of(std::permute(std::iota_v<std::simd<int, 7>>, std::simd_permutations::rotate<-2>)
           == std::simd<int, 7>(std::array {5, 6, 0, 1, 2, 3, 4}.begin())));

static_assert(std::simd_flags<>() == std::simd_flag_default);

static_assert(std::simd_flag_aligned != std::simd_flag_default);

static_assert(std::simd_flag_default != std::simd_flag_aligned);

static_assert((std::simd_flag_default | std::simd_flag_default) == std::simd_flag_default);

static_assert((std::simd_flag_aligned | std::simd_flag_default) == std::simd_flag_aligned);

static_assert((std::simd_flag_aligned | std::simd_flag_aligned) == std::simd_flag_aligned);

static_assert((std::simd_flag_aligned | std::simd_flag_convert)
                == (std::simd_flag_convert | std::simd_flag_aligned));

static_assert(((std::simd_flag_aligned | std::simd_flag_convert) & std::simd_flag_aligned)
                != (std::simd_flag_convert | std::simd_flag_aligned));

static_assert(((std::simd_flag_aligned | std::simd_flag_convert) & std::simd_flag_aligned)
                == std::simd_flag_aligned);

static_assert(std::simd_flag_aligned.test(std::simd_flag_aligned));

static_assert(std::simd_flag_aligned.test(std::simd_flag_default));

static_assert(not std::simd_flag_default.test(std::simd_flag_aligned));
