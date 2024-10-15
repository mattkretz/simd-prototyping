/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "simd"

namespace simd = SIMD_NSPC;

namespace test01
{
  using namespace simd::__detail;

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
  static_assert(__simd_generator_invokable<decltype([] (int) { return true; }), bool, 4>);

  static_assert(std::is_trivially_copyable_v<simd::__detail::_SimdTuple<float, _NativeAbi<float>>>);
  static_assert(std::is_trivially_copyable_v<simd::_AbiCombine<63, _NativeAbi<float>>::__traits<float>::_SimdMember>);

#if defined __SSE__ and not defined __AVX__
  static_assert(std::same_as<__deduce_t<float, 7>, simd::_AbiCombine<7, simd::_VecAbi<4>>>);
  static_assert(simd::_VecAbi<7>::_S_size == 7);
  static_assert(simd::_VecAbi<7>::_S_full_size == 8);
  static_assert(simd::_VecAbi<7>::_IsValid<float>::value == false);
  static_assert(simd::_VecAbi<std::__bit_ceil(7) / 2>::_S_is_partial == false);
  static_assert(simd::_VecAbi<std::__bit_ceil(7) / 2>::_IsValid<float>::value == true);
  static_assert(std::same_as<_AllNativeAbis::_BestPartialAbi<float, 7>, simd::_VecAbi<4>>);
  static_assert(std::same_as<__fixed_size_storage_t<float, 7>,
                             _SimdTuple<float, simd::_VecAbi<4>, simd::_VecAbi<3>>>);

  static_assert(simd::vec<float>::size > 1);
  static_assert(alignof(simd::vec<float>) > alignof(float));
  static_assert(alignof(simd::vec<float, 4>) > alignof(float));
  static_assert(alignof(simd::vec<float, 3>) > alignof(float));
  static_assert(alignof(simd::vec<float, 7>) > alignof(float));
#endif
#if defined __AVX__ and not defined __AVX512F__
  static_assert(std::same_as<__deduce_t<float, 8>, simd::_VecAbi<8>>);
  static_assert(std::same_as<__deduce_t<float, 16>, simd::_AbiArray<simd::_VecAbi<8>, 2>>);
  static_assert(std::same_as<__deduce_t<float, 16>::_SimdMember<float>,
                             std::array<__vec_builtin_type<float, 8>, 2>>);
  static_assert(std::same_as<__deduce_t<float, 16>::_MaskMember<int>,
                             std::array<__vec_builtin_type<int, 8>, 2>>);
  static_assert(std::same_as<simd::mask<float, 16>::abi_type, __deduce_t<float, 16>>);
  static_assert(std::same_as<_SimdMaskTraits<4, __deduce_t<float, 16>>::_MaskMember,
                             std::array<__vec_builtin_type<int, 8>, 2>>);
#endif
}

#if defined __AVX__ and not defined __AVX2__
static_assert(alignof(simd::mask<int, 8>) == 16);
static_assert(alignof(simd::mask<float, 8>) == 32);
static_assert(alignof(simd::mask<int, 16>) == 16);
static_assert(alignof(simd::mask<float, 16>) == 32);
static_assert(alignof(simd::mask<long long, 4>) == 16);
static_assert(alignof(simd::mask<double, 4>) == 32);
static_assert(alignof(simd::mask<long long, 8>) == 16);
static_assert(alignof(simd::mask<double, 8>) == 32);
static_assert(std::same_as<decltype(+simd::mask<float, 8>()), simd::vec<int, 8>>);
#endif

template <auto X>
  using Ic = simd::__detail::_Ic<X>;

static_assert(    std::convertible_to<Ic<1>, simd::vec<float>>);
static_assert(not std::convertible_to<Ic<1.1>, simd::vec<float>>);
static_assert(not std::convertible_to<simd::vec<int, 4>, simd::vec<float, 4>>);
static_assert(not std::convertible_to<simd::vec<float, 4>, simd::vec<int, 4>>);
static_assert(    std::convertible_to<simd::vec<int, 4>, simd::vec<double, 4>>);

template <typename V>
  concept has_static_size = requires {
    { V::size } -> std::convertible_to<int>;
    { V::size() } -> std::signed_integral;
    { auto(V::size.value) } -> std::signed_integral;
  };

template <typename V, typename T = typename V::value_type>
  concept usable_simd_or_mask
    = std::is_nothrow_move_constructible_v<V>
        and std::is_nothrow_move_assignable_v<V>
        and std::is_nothrow_default_constructible_v<V>
        and std::is_trivially_copyable_v<V>
        and std::is_standard_layout_v<V>
        and std::ranges::random_access_range<V&>
        and not std::ranges::output_range<V&, T>
        and std::constructible_from<V, V> // broadcast
        and simd::integral<V> == std::integral<T>
        and simd::floating_point<V> == std::floating_point<T>
        and simd::regular<V>
        and simd::equality_comparable<V>
        and has_static_size<V>
      ;

template <typename V, typename T = typename V::value_type>
  concept usable_simd
    = usable_simd_or_mask<V, T>
        and std::convertible_to<V, std::array<T, V::size()>>
        and std::convertible_to<std::array<T, V::size()>, V>
      // Not for masks because no implicit conversion from bool -> mask
        and simd::equality_comparable_with<V, T>
        and simd::equality_comparable_with<T, V>
      ;

template <typename T>
  struct test_usable_simd
  {
    static_assert(not usable_simd<simd::vec<T, 0>>);
#if SIMD_DISABLED_HAS_API
    static_assert(has_static_size<simd::vec<T, 0>>);
    static_assert(simd::vec<T, 0>::size() == 0);
#else
    static_assert(not has_static_size<simd::vec<T, 0>>);
#endif
    static_assert(usable_simd<simd::vec<T, 1>>);
    static_assert(usable_simd<simd::vec<T, 2>>);
    static_assert(usable_simd<simd::vec<T, 3>>);
    static_assert(usable_simd<simd::vec<T, 4>>);
    static_assert(usable_simd<simd::vec<T, 7>>);
    static_assert(usable_simd<simd::vec<T, 8>>);
    static_assert(usable_simd<simd::vec<T, 16>>);
    static_assert(usable_simd<simd::vec<T, 32>>);
    static_assert(usable_simd<simd::vec<T, 63>>);
    static_assert(usable_simd<simd::vec<T, 64>>);

#if SIMD_DISABLED_HAS_API
    static_assert(has_static_size<simd::mask<T, 0>>);
    static_assert(simd::mask<T, 0>::size() == 0);
#else
    static_assert(not has_static_size<simd::mask<T, 0>>);
#endif
    static_assert(usable_simd_or_mask<simd::mask<T, 1>>);
    static_assert(usable_simd_or_mask<simd::mask<T, 2>>);
    static_assert(usable_simd_or_mask<simd::mask<T, 3>>);
    static_assert(usable_simd_or_mask<simd::mask<T, 4>>);
    static_assert(usable_simd_or_mask<simd::mask<T, 7>>);
    static_assert(usable_simd_or_mask<simd::mask<T, 8>>);
    static_assert(usable_simd_or_mask<simd::mask<T, 16>>);
    static_assert(usable_simd_or_mask<simd::mask<T, 32>>);
    static_assert(usable_simd_or_mask<simd::mask<T, 63>>);
    static_assert(usable_simd_or_mask<simd::mask<T, 64>>);
  };

template <template <typename> class Tpl>
  struct instantiate_all_vectorizable
  {
    Tpl<float> a;
    Tpl<double> b;
    Tpl<char> c;
    Tpl<char8_t> c8;
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
#ifdef __STDCPP_FLOAT16_T__
    //Tpl<std::float16_t> q;
#endif
#ifdef __STDCPP_FLOAT32_T__
    Tpl<std::float32_t> r;
#endif
#ifdef __STDCPP_FLOAT64_T__
    Tpl<std::float64_t> s;
#endif
  };

template struct instantiate_all_vectorizable<test_usable_simd>;

// simd generator ctor ///////////////

namespace test_generator
{
  struct udt_convertible_to_float
  { operator float() const; };

  static_assert(    std::constructible_from<simd::vec<float>, float (&)(int)>);
  static_assert(not std::convertible_to<float (&)(int), simd::vec<float>>);
  static_assert(not std::constructible_from<simd::vec<float>, int (&)(int)>);
  static_assert(not std::constructible_from<simd::vec<float>, double (&)(int)>);
  static_assert(    std::constructible_from<simd::vec<float>, short (&)(int)>);
  // should be invalid with wording update:
  static_assert(    std::constructible_from<simd::vec<float>, long double (&)(int)>);
  static_assert(    std::constructible_from<simd::vec<float>,
                                            udt_convertible_to_float (&)(int)>);
}

// mask generator ctor ///////////////

namespace dp = std::simd_generic;

static_assert(
  all_of(dp::mask<float, 4>([](int) { return true; }) == dp::mask<float, 4>(true)));
static_assert(
  all_of(dp::mask<float, 4>([](int) { return false; }) == dp::mask<float, 4>(false)));
static_assert(
  all_of(dp::mask<float, 4>([](int i) { return i < 2; })
           == dp::mask<float, 4>(std::array{true, true, false, false}.begin())));

static_assert(all_of((dp::generate<dp::vec<int, 4>>([](int i) { return i << 10; }) >> 10)
                == dp::iota_v<dp::vec<int, 4>>));

// mask to vec ///////////////////////

static_assert([] constexpr {
  constexpr dp::mask<float, 7> a([](int i) -> bool { return i < 3; });
  constexpr dp::basic_vec b = -a;
  static_assert(b[0] == -(0 < 3));
  static_assert(b[1] == -(1 < 3));
  static_assert(b[2] == -(2 < 3));
  static_assert(b[3] == -(3 < 3));
  return all_of(b == dp::generate<dp::vec<int, 7>>([](int i) { return -int(i < 3); }));
}());

static_assert([] constexpr {
  constexpr dp::mask<float, 7> a([](int i) -> bool { return i < 3; });
  constexpr dp::basic_vec b = ~a;
  static_assert(b[0] == ~int(0 < 3));
  static_assert(b[1] == ~int(1 < 3));
  static_assert(b[2] == ~int(2 < 3));
  static_assert(b[3] == ~int(3 < 3));
  return all_of(b == dp::generate<dp::vec<int, 7>>([](int i) { return ~int(i < 3); }));
}());

static_assert([] constexpr {
  constexpr dp::mask<float, 4> a([](int i) -> bool { return i < 2; });
  constexpr dp::basic_vec b = a;
  static_assert(b[0] == 1);
  static_assert(b[1] == 1);
  static_assert(b[2] == 0);
  return b[3] == 0;
}());

static_assert([] constexpr {
  // Corner case on AVX w/o AVX2 systems. <float, 5> is an AVX register;
  // <int, 5> is deduced as SSE + scalar.
  constexpr dp::mask<float, 5> a([](int i) -> bool { return i >= 2; });
  constexpr dp::basic_vec b = a;
  static_assert(b[0] == 0);
  static_assert(b[1] == 0);
  static_assert(b[2] == 1);
  static_assert(b[3] == 1);
  static_assert(b[4] == 1);
  static_assert(all_of((b == 1) == a));
  constexpr dp::mask<float, 8> a8([](int i) -> bool { return i <= 4; });
  constexpr dp::basic_vec b8 = a8;
  static_assert(b8[0] == 1);
  static_assert(b8[1] == 1);
  static_assert(b8[2] == 1);
  static_assert(b8[3] == 1);
  static_assert(b8[4] == 1);
  static_assert(b8[5] == 0);
  static_assert(b8[6] == 0);
  static_assert(b8[7] == 0);
  static_assert(all_of((b8 == 1) == a8));
  constexpr dp::mask<float, 15> a15([](int i) -> bool { return i <= 4; });
  constexpr dp::basic_vec b15 = a15;
  static_assert(b15[0] == 1);
  static_assert(b15[4] == 1);
  static_assert(b15[5] == 0);
  static_assert(b15[8] == 0);
  static_assert(b15[14] == 0);
  static_assert(all_of((b15 == 1) == a15));
  return true;
}());

static_assert([] constexpr {
  constexpr dp::mask<float, 4> a([](int i) -> bool { return i < 2; });
  constexpr dp::basic_vec b = ~a;
  constexpr dp::basic_vec c = a;
  static_assert(c[0] == int(a[0]));
  static_assert(c[1] == int(a[1]));
  static_assert(c[2] == int(a[2]));
  static_assert(c[3] == int(a[3]));
  static_assert(b[0] == ~int(0 < 2));
  static_assert(b[1] == ~int(1 < 2));
  static_assert(b[2] == ~int(2 < 2));
  static_assert(b[3] == ~int(3 < 2));
  return all_of(b == dp::generate<dp::vec<int, 4>>([](int i) { return ~int(i < 2); }));
}());

// simd reductions ///////////////////

static_assert(reduce(dp::vec<int, 7>(1)) == 7);
static_assert(reduce(dp::vec<int, 7>(2), std::multiplies<>()) == 128);
static_assert(reduce(dp::vec<int, 8>(2), std::bit_and<>()) == 2);
static_assert(reduce(dp::vec<int, 8>(2), std::bit_or<>()) == 2);
static_assert(reduce(dp::vec<int, 8>(2), std::bit_xor<>()) == 0);
static_assert(reduce(dp::vec<int, 3>(2), std::bit_and<>()) == 2);
static_assert(reduce(dp::vec<int, 6>(2), std::bit_and<>()) == 2);
static_assert(reduce(dp::vec<int, 7>(2), std::bit_and<>()) == 2);
static_assert(reduce(dp::vec<int, 7>(2), std::bit_or<>()) == 2);
static_assert(reduce(dp::vec<int, 7>(2), std::bit_xor<>()) == 2);

// mask reductions ///////////////////

static_assert(all_of(dp::vec<float>() == dp::vec<float>()));
static_assert(any_of(dp::vec<float>() == dp::vec<float>()));
static_assert(not none_of(dp::vec<float>() == dp::vec<float>()));
static_assert(reduce_count(dp::vec<float>() == dp::vec<float>()) == dp::vec<float>::size);
static_assert(reduce_min_index(dp::vec<float>() == dp::vec<float>()) == 0);
static_assert(reduce_max_index(dp::vec<float>() == dp::vec<float>()) == dp::vec<float>::size - 1);

// split ////////////////////////

static_assert([] {
  constexpr auto a = dp::generate<dp::vec<int, 8>>([] (int i) { return i; });
  auto a4 = split<dp::vec<int, 4>>(a);
  auto a3 = split<dp::vec<int, 3>>(a);
  return a4.size() == 2 and std::same_as<decltype(a4), std::array<dp::vec<int, 4>, 2>>
           and std::tuple_size_v<decltype(a3)> == 3
           and all_of(std::get<0>(a3) == dp::generate<dp::vec<int, 3>>([] (int i) { return i; }))
           and all_of(std::get<1>(a3) == dp::generate<dp::vec<int, 3>>([] (int i) { return i + 3; }))
           and all_of(std::get<2>(a3) == dp::generate<dp::vec<int, 2>>([] (int i) { return i + 6; }));
}());

static_assert([] {
  constexpr dp::mask<int, 8> a([] (int i) -> bool { return i & 1; });
  auto a4 = split<dp::mask<int, 4>>(a);
  auto a3 = split<dp::mask<int, 3>>(a);
  return a4.size() == 2 and std::same_as<decltype(a4), std::array<dp::mask<int, 4>, 2>>
           and std::tuple_size_v<decltype(a3)> == 3
           and all_of(std::get<0>(a3) == dp::mask<int, 3>(
                                           [] (int i) -> bool { return i & 1; }))
           and all_of(std::get<1>(a3) == dp::mask<int, 3>(
                                           [] (int i) -> bool { return (i + 3) & 1; }))
           and all_of(std::get<2>(a3) == dp::mask<int, 2>(
                                           [] (int i) -> bool { return (i + 6) & 1; }));
}());

// cat ///////////////////////////

static_assert(all_of(simd::cat(simd::iota_v<simd::vec<int, 3>>, simd::vec<int, 1>(3))
                       == simd::iota_v<simd::vec<int, 4>>));

static_assert(all_of(simd::cat(simd::iota_v<simd::vec<int, 4>>,
                                   simd::iota_v<simd::vec<int, 4>> + 4)
                       == simd::iota_v<simd::vec<int, 8>>));

static_assert(all_of(simd::cat(simd::iota_v<simd::vec<double, 4>>,
                                   simd::iota_v<simd::vec<double, 2>> + 4)
                       == simd::iota_v<simd::vec<double, 6>>));

static_assert(all_of(simd::cat(simd::iota_v<simd::vec<double, 4>>,
                                   simd::iota_v<simd::vec<double, 4>> + 4)
                       == simd::iota_v<simd::vec<double, 8>>));

// select ////////////////////////

static_assert(all_of(simd::vec<long long, 8>(std::array{0, 0, 0, 0, 4, 4, 4, 4})
                       == select(simd::iota_v<simd::vec<double, 8>> < 4, 0ll, 4ll)));

static_assert(all_of(simd::vec<int, 8>(std::array{0, 0, 0, 0, 4, 4, 4, 4})
                       == select(simd::iota_v<simd::vec<float, 8>> < 4.f, 0, 4)));

// interleave /////////////////////

static_assert(
  all_of(std::get<0>(simd::interleave(simd::iota_v<simd::vec<int>>))
	   == simd::iota_v<simd::vec<int>>));

static_assert(
  all_of(std::get<0>(simd::interleave(simd::vec<int>(0), simd::vec<int>(1)))
	   == (simd::iota_v<simd::vec<int>> & 1)));

static_assert(
  all_of(std::get<1>(simd::interleave(simd::vec<int>(0), simd::vec<int>(1)))
	   == (simd::iota_v<simd::vec<int>> & 1)));

// permute ////////////////////////

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int>>, simd::permutations::duplicate_even)
	   == simd::iota_v<simd::vec<int>> / 2 * 2));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int>>, simd::permutations::duplicate_odd)
	   == simd::iota_v<simd::vec<int>> / 2 * 2 + 1));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int>>, simd::permutations::swap_neighbors<1>)
           == dp::generate<simd::vec<int>>([](int i) { return i ^ 1; })));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int, 8>>,
		      simd::permutations::swap_neighbors<2>)
	   == simd::vec<int, 8>(std::array{2, 3, 0, 1, 6, 7, 4, 5})));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int, 12>>,
		      simd::permutations::swap_neighbors<3>)
	   == simd::vec<int, 12>(
		std::array{3, 4, 5, 0, 1, 2, 9, 10, 11, 6, 7, 8})));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int>>, simd::permutations::broadcast<1>)
	   == simd::vec<int>(1)));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int>>, simd::permutations::broadcast_first)
	   == simd::vec<int>(0)));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int>>, simd::permutations::broadcast_last)
           == simd::vec<int>(int(simd::vec<int>::size() - 1))));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int>>, simd::permutations::reverse)
           == dp::generate<simd::vec<int>>([](int i) { return int(simd::vec<int>::size()) - 1 - i; })));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int>>, simd::permutations::rotate<1>)
           == (simd::iota_v<simd::vec<int>> + 1) % int(simd::vec<int>::size())));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int>>, simd::permutations::rotate<2>)
           == (simd::iota_v<simd::vec<int>> + 2) % int(simd::vec<int>::size())));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int, 7>>, simd::permutations::rotate<2>)
           == simd::vec<int, 7>(std::array {2, 3, 4, 5, 6, 0, 1})));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::vec<int, 7>>, simd::permutations::rotate<-2>)
           == simd::vec<int, 7>(std::array {5, 6, 0, 1, 2, 3, 4})));

// simd_flags ////////////////////////

static_assert(simd::flags<>()._M_is_equal(simd::flag_default));

static_assert(not simd::flag_aligned._M_is_equal(simd::flag_default));

static_assert(not simd::flag_default._M_is_equal(simd::flag_aligned));

static_assert((simd::flag_default | simd::flag_default)
                ._M_is_equal(simd::flag_default));

static_assert((simd::flag_aligned | simd::flag_default)
                ._M_is_equal(simd::flag_aligned));

static_assert((simd::flag_aligned | simd::flag_aligned)
                ._M_is_equal(simd::flag_aligned));

static_assert((simd::flag_aligned | simd::flag_convert)
                ._M_is_equal(simd::flag_convert | simd::flag_aligned));

static_assert(not ((simd::flag_aligned | simd::flag_convert)
                     ._M_and(simd::flag_aligned))
                ._M_is_equal(simd::flag_convert | simd::flag_aligned));

static_assert(((simd::flag_aligned | simd::flag_convert)
                 ._M_and(simd::flag_aligned))
                ._M_is_equal(simd::flag_aligned));

static_assert(simd::flag_aligned._M_test(simd::flag_aligned));

static_assert(simd::flag_aligned._M_test(simd::flag_default));

static_assert(not simd::flag_default._M_test(simd::flag_aligned));

// simd concepts ///////////////////////////////////

static_assert(std::simd_generic::integral<int>);
static_assert(std::simd_generic::integral<simd::vec<int>>);
static_assert(std::simd_generic::integral<simd::mask<int>>);

static_assert(simd::regular<int>);
static_assert(simd::regular<simd::vec<int>>);
static_assert(simd::regular<simd::mask<int>>);

// simd.math ///////////////////////////////////////

namespace math_tests
{
  using namespace vir::literals;

  constexpr simd::vec<float, 1>
    operator""_f1(long double x)
  { return float(x); }

  constexpr simd::vec<float, 4>
    operator""_f4(long double x)
  { return float(x); }

  template <typename... Ts>
    concept hypot_invocable = requires(Ts... xs) {
      simd::hypot(xs...);
    };

  template <typename R, typename... Ts>
    concept hypot_invocable_r = requires(Ts... xs) {
      { simd::hypot(xs...) } -> std::same_as<R>;
    };

  template <typename T>
    struct holder
    {
      T value;

      constexpr
      operator const T&() const
      { return value; }
    };

  static_assert(simd::floor(1.1_f1)[0] == std::floor(1.1f));
  static_assert(simd::floor(std::array{1.1f, 1.2f, 2.f, 3.f})[0] == std::floor(1.1f));
  static_assert(simd::hypot(1.1_f1, 1.2_f1)[0] == std::hypot(1.1f, 1.2f));
  static_assert(simd::hypot(1.1_f1, 1.2f)[0] == std::hypot(1.1f, 1.2f));
  // the next doesn't work with the P1928 spec, but it can be made to work
  static_assert(simd::hypot(std::array{1.1f}, 1.2f)[0] == std::hypot(1.1f, 1.2f));
  static_assert(simd::hypot(1.1f, 1.2_f1)[0] == std::hypot(1.1f, 1.2f));
  static_assert(simd::hypot(1_cw, 1.2_f1)[0] == std::hypot(1.f, 1.2f));
  static_assert(simd::hypot(1.2_f1, 1_cw)[0] == std::hypot(1.f, 1.2f));
  static_assert(simd::hypot(holder {1.f}, 1.2_f1)[0] == std::hypot(1.f, 1.2f));
  // the following must not be valid. if you want simd<double> be explicit about it:
  static_assert(not hypot_invocable<int, simd::vec<float, 1>>);
  static_assert(not hypot_invocable<int, simd::vec<float, 1>, simd::vec<float, 1>>);

  static_assert(hypot_invocable_r<simd::vec<float, 1>, holder<float>,
                                  vir::constexpr_wrapper<2>, simd::vec<float, 1>>);
  static_assert(hypot_invocable_r<simd::vec<float, 1>, holder<short>,
                                  simd::vec<float, 1>, float>);
}
