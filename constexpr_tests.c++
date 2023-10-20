/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "interleave.h"
#include "permute.h"
#include "simd_split.h"
#include "mask_reductions.h"

namespace test01
{
  using namespace std::__detail;

  template <typename T, int N>
    class MyArray
    {
      T data[N];

    public:
      static constexpr std::integral_constant<int, N> size = {};
    };

  static_assert(__static_range_size<MyArray<int, 4>> == 4);
  static_assert(__static_range_size<std::array<int, 4>> == 4);
  static_assert(__static_range_size<int[4]> == 4);
  static_assert(__static_range_size<std::span<int, 4>> == 4);
  static_assert(__static_range_size<std::span<int>> == std::dynamic_extent);
  static_assert(__static_range_size<std::vector<int>> == std::dynamic_extent);

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

  static_assert(std::is_trivially_copyable_v<std::__detail::_SimdTuple<float, _NativeAbi<float>>>);
  static_assert(std::is_trivially_copyable_v<_AbiCombine<63, _NativeAbi<float>>::__traits<float>::_SimdMember>);
  static_assert(std::is_trivially_copyable_v<_AbiCombine<63, _NativeAbi<float>>::__traits<float>::_SimdBase>);
}

template <auto X>
  using Ic = std::__detail::_Ic<X>;

static_assert(    std::convertible_to<Ic<1>, std::simd<float>>);
static_assert(not std::convertible_to<Ic<1.1>, std::simd<float>>);
static_assert(not std::convertible_to<std::simd<int, 4>, std::simd<float, 4>>);
static_assert(not std::convertible_to<std::simd<float, 4>, std::simd<int, 4>>);
static_assert(    std::convertible_to<std::simd<int, 4>, std::simd<double, 4>>);

template <typename T>
concept usable_simd = std::is_nothrow_move_constructible_v<T>
                        and std::is_nothrow_move_assignable_v<T>
                        and std::is_nothrow_default_constructible_v<T>
                        and std::is_trivially_copyable_v<T>
                        and std::is_standard_layout_v<T>;

static_assert(
  usable_simd<std::__pv2::_SimdTraits<int, std::__detail::__deduce_t<int, 6>>::_SimdBase>);
static_assert(
  usable_simd<std::__pv2::_SimdTraits<int, std::__detail::__deduce_t<int, 6>>::_SimdMember>);

template <typename T>
  struct test_usable_simd
  {
    static_assert(usable_simd<std::simd<T, 1>>);
    static_assert(usable_simd<std::simd<T, 2>>);
    static_assert(usable_simd<std::simd<T, 3>>);
    static_assert(usable_simd<std::simd<T, 4>>);
    static_assert(usable_simd<std::simd<T, 8>>);
    static_assert(usable_simd<std::simd<T, 16>>);
    static_assert(usable_simd<std::simd<T, 32>>);
    static_assert(usable_simd<std::simd<T, 63>>);
    static_assert(usable_simd<std::simd<T, 64>>);

    static_assert(usable_simd<std::simd_mask<T, 1>>);
    static_assert(usable_simd<std::simd_mask<T, 2>>);
    static_assert(usable_simd<std::simd_mask<T, 3>>);
    static_assert(usable_simd<std::simd_mask<T, 4>>);
    static_assert(usable_simd<std::simd_mask<T, 8>>);
    static_assert(usable_simd<std::simd_mask<T, 16>>);
    static_assert(usable_simd<std::simd_mask<T, 32>>);
    static_assert(usable_simd<std::simd_mask<T, 63>>);
    static_assert(usable_simd<std::simd_mask<T, 64>>);
  };

template <template <typename> class Tpl>
  struct instantiate_all_vectorizable
  {
    Tpl<float> a;
    Tpl<double> b;
    Tpl<char> c;
    Tpl<char16_t> d;
    Tpl<char32_t> e;
    Tpl<wchar_t> f;
    Tpl<signed char> g;
    Tpl<unsigned char> h;
    Tpl<short> i;
    Tpl<unsigned short> j;
    Tpl<int> k;
    Tpl<unsigned int> l;
    Tpl<long> m;
    Tpl<unsigned long> n;
    Tpl<long long> o;
    Tpl<unsigned long long> p;
  };

template struct instantiate_all_vectorizable<test_usable_simd>;

static_assert(
  all_of(std::simd_mask<float, 4>([](int) { return true; }) == std::simd_mask<float, 4>(true)));
static_assert(
  all_of(std::simd_mask<float, 4>([](int) { return false; }) == std::simd_mask<float, 4>(false)));
static_assert(
  all_of(std::simd_mask<float, 4>([](int i) { return i < 2; })
           == std::simd_mask<float, 4>(std::array{true, true, false, false}.begin())));

static_assert([] constexpr {
  constexpr std::simd_mask<float, 7> a([](int i) -> bool { return i < 3; });
  constexpr std::basic_simd b = -a;
  static_assert(b[0] == -(0 < 3));
  static_assert(b[1] == -(1 < 3));
  static_assert(b[2] == -(2 < 3));
  static_assert(b[3] == -(3 < 3));
  return all_of(b == std::simd<int, 7>([](int i) { return -int(i < 3); }));
}());

static_assert([] constexpr {
  constexpr std::simd_mask<float, 4> a([](int i) -> bool { return i < 2; });
  constexpr std::basic_simd b = ~a;
  static_assert(b[0] == ~int(0 < 2));
  static_assert(b[1] == ~int(1 < 2));
  static_assert(b[2] == ~int(2 < 2));
  static_assert(b[3] == ~int(3 < 2));
  return all_of(b == std::simd<int, 4>([](int i) { return ~int(i < 2); }));
}());


// simd reductions ///////////////////

static_assert(reduce(std::simd<int, 7>(1)) == 7);
static_assert(reduce(std::simd<int, 7>(2), std::multiplies<>()) == 128);
static_assert(reduce(std::simd<int, 8>(2), std::bit_and<>()) == 2);
static_assert(reduce(std::simd<int, 8>(2), std::bit_or<>()) == 2);
static_assert(reduce(std::simd<int, 8>(2), std::bit_xor<>()) == 0);
static_assert(reduce(std::simd<int, 3>(2), std::bit_and<>()) == 2);
static_assert(reduce(std::simd<int, 6>(2), std::bit_and<>()) == 2);
static_assert(reduce(std::simd<int, 7>(2), std::bit_and<>()) == 2);
static_assert(reduce(std::simd<int, 7>(2), std::bit_or<>()) == 2);
static_assert(reduce(std::simd<int, 7>(2), std::bit_xor<>()) == 2);

// mask reductions ///////////////////

static_assert(all_of(std::simd<float>() == std::simd<float>()));
static_assert(any_of(std::simd<float>() == std::simd<float>()));
static_assert(not none_of(std::simd<float>() == std::simd<float>()));
static_assert(reduce_count(std::simd<float>() == std::simd<float>()) == std::simd<float>::size);
static_assert(reduce_min_index(std::simd<float>() == std::simd<float>()) == 0);
static_assert(reduce_max_index(std::simd<float>() == std::simd<float>()) == std::simd<float>::size - 1);

// simd_split ////////////////////////

static_assert([] {
  constexpr std::simd<int, 8> a([] (int i) { return i; });
  auto a4 = simd_split<std::simd<int, 4>>(a);
  auto a3 = simd_split<std::simd<int, 3>>(a);
  return a4.size() == 2 and std::same_as<decltype(a4), std::array<std::simd<int, 4>, 2>>
           and std::tuple_size_v<decltype(a3)> == 3
           and all_of(std::get<0>(a3) == std::simd<int, 3>([] (int i) { return i; }))
           and all_of(std::get<1>(a3) == std::simd<int, 3>([] (int i) { return i + 3; }))
           and all_of(std::get<2>(a3) == std::simd<int, 2>([] (int i) { return i + 6; }));
}());

static_assert([] {
  constexpr std::simd_mask<int, 8> a([] (int i) -> bool { return i & 1; });
  auto a4 = simd_split<std::simd_mask<int, 4>>(a);
  auto a3 = simd_split<std::simd_mask<int, 3>>(a);
  return a4.size() == 2 and std::same_as<decltype(a4), std::array<std::simd_mask<int, 4>, 2>>
           and std::tuple_size_v<decltype(a3)> == 3
           and all_of(std::get<0>(a3) == std::simd_mask<int, 3>(
                                           [] (int i) -> bool { return i & 1; }))
           and all_of(std::get<1>(a3) == std::simd_mask<int, 3>(
                                           [] (int i) -> bool { return (i + 3) & 1; }))
           and all_of(std::get<2>(a3) == std::simd_mask<int, 2>(
                                           [] (int i) -> bool { return (i + 6) & 1; }));
}());

// simd_cat ///////////////////////////

static_assert(all_of(std::simd_cat(std::iota_v<std::simd<int, 3>>, std::simd<int, 1>(3))
                       == std::iota_v<std::simd<int, 4>>));

static_assert(all_of(std::simd_cat(std::iota_v<std::simd<int, 4>>,
                                   std::iota_v<std::simd<int, 4>> + 4)
                       == std::iota_v<std::simd<int, 8>>));

static_assert(all_of(std::simd_cat(std::iota_v<std::simd<double, 4>>,
                                   std::iota_v<std::simd<double, 2>> + 4)
                       == std::iota_v<std::simd<double, 6>>));

static_assert(all_of(std::simd_cat(std::iota_v<std::simd<double, 4>>,
                                   std::iota_v<std::simd<double, 4>> + 4)
                       == std::iota_v<std::simd<double, 8>>));

// simd_select ////////////////////////

static_assert(all_of(std::simd<long long, 8>(std::array{0, 0, 0, 0, 4, 4, 4, 4}.begin())
                       == simd_select(std::iota_v<std::simd<double, 8>> < 4, 0ll, 4ll)));

static_assert(all_of(std::simd<int, 8>(std::array{0, 0, 0, 0, 4, 4, 4, 4}.begin())
                       == simd_select(std::iota_v<std::simd<float, 8>> < 4.f, 0, 4)));

// interleave /////////////////////

static_assert(
  all_of(std::get<0>(std::interleave(std::iota_v<std::simd<int>>))
	   == std::iota_v<std::simd<int>>));

static_assert(
  all_of(std::get<0>(std::interleave(std::simd<int>(0), std::simd<int>(1)))
	   == (std::iota_v<std::simd<int>> & 1)));

static_assert(
  all_of(std::get<1>(std::interleave(std::simd<int>(0), std::simd<int>(1)))
	   == (std::iota_v<std::simd<int>> & 1)));

// simd_permute ////////////////////////

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int>>, std::simd_permutations::duplicate_even)
	   == std::iota_v<std::simd<int>> / 2 * 2));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int>>, std::simd_permutations::duplicate_odd)
	   == std::iota_v<std::simd<int>> / 2 * 2 + 1));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int>>, std::simd_permutations::swap_neighbors<1>)
	   == std::simd<int>([](int i) { return i ^ 1; })));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int, 8>>,
		      std::simd_permutations::swap_neighbors<2>)
	   == std::simd<int, 8>(std::array{2, 3, 0, 1, 6, 7, 4, 5}.begin())));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int, 12>>,
		      std::simd_permutations::swap_neighbors<3>)
	   == std::simd<int, 12>(
		std::array{3, 4, 5, 0, 1, 2, 9, 10, 11, 6, 7, 8}.begin())));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int>>, std::simd_permutations::broadcast<1>)
	   == std::simd<int>(1)));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int>>, std::simd_permutations::broadcast_first)
	   == std::simd<int>(0)));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int>>, std::simd_permutations::broadcast_last)
	   == std::simd<int>(int(std::simd_size_v<int> - 1))));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int>>, std::simd_permutations::reverse)
	   == std::simd<int>([](int i) { return int(std::simd_size_v<int>) - 1 - i; })));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int>>, std::simd_permutations::rotate<1>)
	   == (std::iota_v<std::simd<int>> + 1) % int(std::simd_size_v<int>)));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int>>, std::simd_permutations::rotate<2>)
	   == (std::iota_v<std::simd<int>> + 2) % int(std::simd_size_v<int>)));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int, 7>>, std::simd_permutations::rotate<2>)
           == std::simd<int, 7>(std::array {2, 3, 4, 5, 6, 0, 1}.begin())));

static_assert(
  all_of(std::simd_permute(std::iota_v<std::simd<int, 7>>, std::simd_permutations::rotate<-2>)
           == std::simd<int, 7>(std::array {5, 6, 0, 1, 2, 3, 4}.begin())));

// simd_flags ////////////////////////

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
