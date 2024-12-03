/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "simd"

namespace simd = std;

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

  static_assert(simd::simd<float>::size > 1);
  static_assert(alignof(simd::simd<float>) > alignof(float));
  static_assert(alignof(simd::simd<float, 4>) > alignof(float));
  static_assert(alignof(simd::simd<float, 3>) > alignof(float));
  static_assert(alignof(simd::simd<float, 7>) > alignof(float));
#endif
#if defined __AVX__ and not defined __AVX512F__
  static_assert(std::same_as<__deduce_t<float, 8>, simd::_VecAbi<8>>);
  static_assert(std::same_as<__deduce_t<float, 16>, simd::_AbiArray<simd::_VecAbi<8>, 2>>);
  static_assert(std::same_as<__deduce_t<float, 16>::_SimdMember<float>,
                             std::array<__vec_builtin_type<float, 8>, 2>>);
  static_assert(std::same_as<__deduce_t<float, 16>::_MaskMember<int>,
                             std::array<__vec_builtin_type<int, 8>, 2>>);
  static_assert(std::same_as<simd::simd_mask<float, 16>::abi_type, __deduce_t<float, 16>>);
  static_assert(std::same_as<_SimdMaskTraits<4, __deduce_t<float, 16>>::_MaskMember,
                             std::array<__vec_builtin_type<int, 8>, 2>>);
#endif
}

#if defined __AVX__ and not defined __AVX2__
static_assert(alignof(simd::simd_mask<int, 8>) == 16);
static_assert(alignof(simd::simd_mask<float, 8>) == 32);
static_assert(alignof(simd::simd_mask<int, 16>) == 16);
static_assert(alignof(simd::simd_mask<float, 16>) == 32);
static_assert(alignof(simd::simd_mask<long long, 4>) == 16);
static_assert(alignof(simd::simd_mask<double, 4>) == 32);
static_assert(alignof(simd::simd_mask<long long, 8>) == 16);
static_assert(alignof(simd::simd_mask<double, 8>) == 32);
static_assert(std::same_as<decltype(+simd::simd_mask<float, 8>()), simd::simd<int, 8>>);
#endif

template <auto X>
  using Ic = simd::__detail::_Ic<X>;

static_assert(    std::convertible_to<Ic<1>, std::simd<float>>);
static_assert(not std::convertible_to<Ic<1.1>, std::simd<float>>);
static_assert(not std::convertible_to<std::simd<int, 4>, std::simd<float, 4>>);
static_assert(not std::convertible_to<std::simd<float, 4>, std::simd<int, 4>>);
static_assert(    std::convertible_to<std::simd<int, 4>, std::simd<double, 4>>);

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
        and ext::simd_integral<V> == std::integral<T>
        and ext::simd_floating_point<V> == std::floating_point<T>
        and ext::simd_regular<V>
        and ext::simd_equality_comparable<V>
        and has_static_size<V>
      ;

template <typename V, typename T = typename V::value_type>
  concept usable_simd
    = usable_simd_or_mask<V, T>
        and std::convertible_to<V, std::array<T, V::size()>>
        and std::convertible_to<std::array<T, V::size()>, V>
      // Not for masks because no implicit conversion from bool -> mask
        and ext::simd_equality_comparable_with<V, T>
        and ext::simd_equality_comparable_with<T, V>
      ;

template <typename T>
  struct test_usable_simd
  {
    static_assert(not usable_simd<simd::simd<T, 0>>);
#if SIMD_DISABLED_HAS_API
    static_assert(has_static_size<simd::simd<T, 0>>);
    static_assert(simd::simd<T, 0>::size() == 0);
#else
    static_assert(not has_static_size<simd::simd<T, 0>>);
#endif
    static_assert(usable_simd<simd::simd<T, 1>>);
    static_assert(usable_simd<simd::simd<T, 2>>);
    static_assert(usable_simd<simd::simd<T, 3>>);
    static_assert(usable_simd<simd::simd<T, 4>>);
    static_assert(usable_simd<simd::simd<T, 7>>);
    static_assert(usable_simd<simd::simd<T, 8>>);
    static_assert(usable_simd<simd::simd<T, 16>>);
    static_assert(usable_simd<simd::simd<T, 32>>);
    static_assert(usable_simd<simd::simd<T, 63>>);
    static_assert(usable_simd<simd::simd<T, 64>>);

#if SIMD_DISABLED_HAS_API
    static_assert(has_static_size<simd::simd_mask<T, 0>>);
    static_assert(simd::simd_mask<T, 0>::size() == 0);
#else
    static_assert(not has_static_size<simd::simd_mask<T, 0>>);
#endif
    static_assert(usable_simd_or_mask<simd::simd_mask<T, 1>>);
    static_assert(usable_simd_or_mask<simd::simd_mask<T, 2>>);
    static_assert(usable_simd_or_mask<simd::simd_mask<T, 3>>);
    static_assert(usable_simd_or_mask<simd::simd_mask<T, 4>>);
    static_assert(usable_simd_or_mask<simd::simd_mask<T, 7>>);
    static_assert(usable_simd_or_mask<simd::simd_mask<T, 8>>);
    static_assert(usable_simd_or_mask<simd::simd_mask<T, 16>>);
    static_assert(usable_simd_or_mask<simd::simd_mask<T, 32>>);
    static_assert(usable_simd_or_mask<simd::simd_mask<T, 63>>);
    static_assert(usable_simd_or_mask<simd::simd_mask<T, 64>>);
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

  static_assert(    std::constructible_from<simd::simd<float>, float (&)(int)>);
  static_assert(not std::convertible_to<float (&)(int), simd::simd<float>>);
  static_assert(not std::constructible_from<simd::simd<float>, int (&)(int)>);
  static_assert(not std::constructible_from<simd::simd<float>, double (&)(int)>);
  static_assert(    std::constructible_from<simd::simd<float>, short (&)(int)>);
  // should be invalid with wording update:
  static_assert(    std::constructible_from<simd::simd<float>, long double (&)(int)>);
  static_assert(    std::constructible_from<simd::simd<float>,
                                            udt_convertible_to_float (&)(int)>);
}

// mask generator ctor ///////////////

static_assert(
  all_of(std::simd_mask<float, 4>([](int) { return true; }) == std::simd_mask<float, 4>(true)));
static_assert(
  all_of(std::simd_mask<float, 4>([](int) { return false; }) == std::simd_mask<float, 4>(false)));
static_assert(
  all_of(std::simd_mask<float, 4>([](int i) { return i < 2; })
           == std::simd_mask<float, 4>([](int i) {
                return std::array{true, true, false, false}[i];
              })));

static_assert(all_of((std::generate<std::simd<int, 4>>([](int i) { return i << 10; }) >> 10)
                == std::iota_v<std::simd<int, 4>>));

// mask to simd ///////////////////////

static_assert([] constexpr {
  constexpr std::simd_mask<float, 7> a([](int i) -> bool { return i < 3; });
  constexpr std::basic_simd b = -a;
  static_assert(b[0] == -(0 < 3));
  static_assert(b[1] == -(1 < 3));
  static_assert(b[2] == -(2 < 3));
  static_assert(b[3] == -(3 < 3));
  return all_of(b == std::generate<std::simd<int, 7>>([](int i) { return -int(i < 3); }));
}());

static_assert([] constexpr {
  constexpr std::simd_mask<float, 7> a([](int i) -> bool { return i < 3; });
  constexpr std::basic_simd b = ~a;
  static_assert(b[0] == ~int(0 < 3));
  static_assert(b[1] == ~int(1 < 3));
  static_assert(b[2] == ~int(2 < 3));
  static_assert(b[3] == ~int(3 < 3));
  return all_of(b == std::generate<std::simd<int, 7>>([](int i) { return ~int(i < 3); }));
}());

static_assert([] constexpr {
  constexpr std::simd_mask<float, 4> a([](int i) -> bool { return i < 2; });
  constexpr std::basic_simd b = a;
  static_assert(b[0] == 1);
  static_assert(b[1] == 1);
  static_assert(b[2] == 0);
  return b[3] == 0;
}());

static_assert([] constexpr {
  // Corner case on AVX w/o AVX2 systems. <float, 5> is an AVX register;
  // <int, 5> is deduced as SSE + scalar.
  constexpr std::simd_mask<float, 5> a([](int i) -> bool { return i >= 2; });
  constexpr std::basic_simd b = a;
  static_assert(b[0] == 0);
  static_assert(b[1] == 0);
  static_assert(b[2] == 1);
  static_assert(b[3] == 1);
  static_assert(b[4] == 1);
  static_assert(all_of((b == 1) == a));
  constexpr std::simd_mask<float, 8> a8([](int i) -> bool { return i <= 4; });
  constexpr std::basic_simd b8 = a8;
  static_assert(b8[0] == 1);
  static_assert(b8[1] == 1);
  static_assert(b8[2] == 1);
  static_assert(b8[3] == 1);
  static_assert(b8[4] == 1);
  static_assert(b8[5] == 0);
  static_assert(b8[6] == 0);
  static_assert(b8[7] == 0);
  static_assert(all_of((b8 == 1) == a8));
  constexpr std::simd_mask<float, 15> a15([](int i) -> bool { return i <= 4; });
  constexpr std::basic_simd b15 = a15;
  static_assert(b15[0] == 1);
  static_assert(b15[4] == 1);
  static_assert(b15[5] == 0);
  static_assert(b15[8] == 0);
  static_assert(b15[14] == 0);
  static_assert(all_of((b15 == 1) == a15));
  return true;
}());

static_assert([] constexpr {
  constexpr std::simd_mask<float, 4> a([](int i) -> bool { return i < 2; });
  constexpr std::basic_simd b = ~a;
  constexpr std::basic_simd c = a;
  static_assert(c[0] == int(a[0]));
  static_assert(c[1] == int(a[1]));
  static_assert(c[2] == int(a[2]));
  static_assert(c[3] == int(a[3]));
  static_assert(b[0] == ~int(0 < 2));
  static_assert(b[1] == ~int(1 < 2));
  static_assert(b[2] == ~int(2 < 2));
  static_assert(b[3] == ~int(3 < 2));
  return all_of(b == std::generate<std::simd<int, 4>>([](int i) { return ~int(i < 2); }));
}());

// simd reductions ///////////////////

namespace simd_reduction_tests
{
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
  static_assert(reduce(std::simd<int, 4>(2), std::simd_mask<int, 4>(false)) == 0);
  static_assert(reduce(std::simd<int, 4>(2), std::simd_mask<int, 4>(false), std::multiplies<>()) == 1);
  static_assert(reduce(std::simd<int, 4>(2), std::simd_mask<int, 4>(false), std::bit_and<>()) == ~0);
  static_assert(reduce(std::simd<int, 4>(2), std::simd_mask<int, 4>(false), [](auto a, auto b) {
                  return select(a < b, a, b);
                }, INT_MAX) == INT_MAX);

  template <typename BinaryOperation>
    concept masked_reduce_works = requires(simd::simd<int, 4> a, simd::simd<int, 4> b) {
      reduce(a, a < b, BinaryOperation());
    };

  static_assert(not masked_reduce_works<std::minus<>>);
}

// mask reductions ///////////////////

static_assert(all_of(std::simd<float>() == std::simd<float>()));
static_assert(any_of(std::simd<float>() == std::simd<float>()));
static_assert(not none_of(std::simd<float>() == std::simd<float>()));
static_assert(reduce_count(std::simd<float>() == std::simd<float>()) == std::simd<float>::size);
static_assert(reduce_min_index(std::simd<float>() == std::simd<float>()) == 0);
static_assert(reduce_max_index(std::simd<float>() == std::simd<float>()) == std::simd<float>::size - 1);

// split ////////////////////////

static_assert([] {
  constexpr auto a = std::generate<std::simd<int, 8>>([] (int i) { return i; });
  auto a4 = split<std::simd<int, 4>>(a);
  auto a3 = split<std::simd<int, 3>>(a);
  return a4.size() == 2 and std::same_as<decltype(a4), std::array<std::simd<int, 4>, 2>>
           and std::tuple_size_v<decltype(a3)> == 3
           and all_of(std::get<0>(a3) == std::generate<std::simd<int, 3>>([] (int i) { return i; }))
           and all_of(std::get<1>(a3) == std::generate<std::simd<int, 3>>([] (int i) { return i + 3; }))
           and all_of(std::get<2>(a3) == std::generate<std::simd<int, 2>>([] (int i) { return i + 6; }));
}());

static_assert([] {
  constexpr std::simd_mask<int, 8> a([] (int i) -> bool { return i & 1; });
  auto a4 = split<std::simd_mask<int, 4>>(a);
  auto a3 = split<std::simd_mask<int, 3>>(a);
  return a4.size() == 2 and std::same_as<decltype(a4), std::array<std::simd_mask<int, 4>, 2>>
           and std::tuple_size_v<decltype(a3)> == 3
           and all_of(std::get<0>(a3) == std::simd_mask<int, 3>(
                                           [] (int i) -> bool { return i & 1; }))
           and all_of(std::get<1>(a3) == std::simd_mask<int, 3>(
                                           [] (int i) -> bool { return (i + 3) & 1; }))
           and all_of(std::get<2>(a3) == std::simd_mask<int, 2>(
                                           [] (int i) -> bool { return (i + 6) & 1; }));
}());

// cat ///////////////////////////

static_assert(all_of(simd::cat(simd::iota_v<simd::simd<int, 3>>, simd::simd<int, 1>(3))
                       == simd::iota_v<simd::simd<int, 4>>));

static_assert(all_of(simd::cat(simd::iota_v<simd::simd<int, 4>>,
                                   simd::iota_v<simd::simd<int, 4>> + 4)
                       == simd::iota_v<simd::simd<int, 8>>));

static_assert(all_of(simd::cat(simd::iota_v<simd::simd<double, 4>>,
                                   simd::iota_v<simd::simd<double, 2>> + 4)
                       == simd::iota_v<simd::simd<double, 6>>));

static_assert(all_of(simd::cat(simd::iota_v<simd::simd<double, 4>>,
                                   simd::iota_v<simd::simd<double, 4>> + 4)
                       == simd::iota_v<simd::simd<double, 8>>));

// select ////////////////////////

static_assert(all_of(simd::simd<long long, 8>(std::array{0, 0, 0, 0, 4, 4, 4, 4})
                       == select(simd::iota_v<simd::simd<double, 8>> < 4, 0ll, 4ll)));

static_assert(all_of(simd::simd<int, 8>(std::array{0, 0, 0, 0, 4, 4, 4, 4})
                       == select(simd::iota_v<simd::simd<float, 8>> < 4.f, 0, 4)));

// interleave /////////////////////

static_assert(
  all_of(std::get<0>(simd::interleave(simd::iota_v<simd::simd<int>>))
	   == simd::iota_v<simd::simd<int>>));

static_assert(
  all_of(std::get<0>(simd::interleave(simd::simd<int>(0), simd::simd<int>(1)))
	   == (simd::iota_v<simd::simd<int>> & 1)));

static_assert(
  all_of(std::get<1>(simd::interleave(simd::simd<int>(0), simd::simd<int>(1)))
	   == (simd::iota_v<simd::simd<int>> & 1)));

// permute ////////////////////////

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int>>, simd::permutations::duplicate_even)
	   == simd::iota_v<simd::simd<int>> / 2 * 2));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int>>, simd::permutations::duplicate_odd)
	   == simd::iota_v<simd::simd<int>> / 2 * 2 + 1));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int>>, simd::permutations::swap_neighbors<1>)
           == std::generate<simd::simd<int>>([](int i) { return i ^ 1; })));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int, 8>>,
		      simd::permutations::swap_neighbors<2>)
	   == simd::simd<int, 8>(std::array{2, 3, 0, 1, 6, 7, 4, 5})));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int, 12>>,
		      simd::permutations::swap_neighbors<3>)
	   == simd::simd<int, 12>(
		std::array{3, 4, 5, 0, 1, 2, 9, 10, 11, 6, 7, 8})));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int>>, simd::permutations::broadcast<1>)
	   == simd::simd<int>(1)));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int>>, simd::permutations::broadcast_first)
	   == simd::simd<int>(0)));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int>>, simd::permutations::broadcast_last)
           == simd::simd<int>(int(simd::simd<int>::size() - 1))));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int>>, simd::permutations::reverse)
           == std::generate<simd::simd<int>>([](int i) { return int(simd::simd<int>::size()) - 1 - i; })));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int>>, simd::permutations::rotate<1>)
           == (simd::iota_v<simd::simd<int>> + 1) % int(simd::simd<int>::size())));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int>>, simd::permutations::rotate<2>)
           == (simd::iota_v<simd::simd<int>> + 2) % int(simd::simd<int>::size())));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int, 7>>, simd::permutations::rotate<2>)
           == simd::simd<int, 7>(std::array {2, 3, 4, 5, 6, 0, 1})));

static_assert(
  all_of(simd::permute(simd::iota_v<simd::simd<int, 7>>, simd::permutations::rotate<-2>)
           == simd::simd<int, 7>(std::array {5, 6, 0, 1, 2, 3, 4})));

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

static_assert(ext::simd_regular<int>);
static_assert(ext::simd_regular<simd::simd<int>>);
static_assert(ext::simd_regular<simd::simd_mask<int>>);

// simd.math ///////////////////////////////////////

namespace math_tests
{
  using namespace vir::literals;

  constexpr simd::simd<float, 1>
    operator""_f1(long double x)
  { return float(x); }

  constexpr simd::simd<float, 4>
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

  static_assert(std::same_as<simd::simd<float, 2>,
                             simd::__detail::__math_common_simd_t<
                               short, holder<float>, holder<simd::simd<float, 2>>>
                            >);

  static_assert(std::same_as<simd::simd<float, 3>,
                             simd::__detail::__math_common_simd_t<
                               holder<simd::simd<float, 3>>, float, holder<short>>
                            >);

  static_assert(std::same_as<simd::simd<float, 3>,
                             simd::__detail::__math_common_simd_t<
                               holder<simd::simd<float, 3>>, float, holder<short>,
                               simd::simd<char, 3>>
                            >);

  static_assert(std::same_as<simd::simd<float, 3>,
                             simd::__detail::__math_common_simd_t<
                               simd::simd<char, 3>,
                               holder<simd::simd<float, 3>>, float, holder<short>>
                            >);

  static_assert(simd::floor(1.1_f1)[0] == std::floor(1.1f));
  static_assert(simd::floor(std::basic_simd(std::array{1.1f, 1.2f, 2.f, 3.f}))[0] == std::floor(1.1f));
  static_assert(simd::floor(holder {1.1_f1})[0] == std::floor(1.1f));
  static_assert(simd::hypot(1.1_f1, 1.2_f1)[0] == std::hypot(1.1f, 1.2f));
  static_assert(simd::hypot(1.1_f1, 1.2f)[0] == std::hypot(1.1f, 1.2f));
  // the next doesn't work with the P1928 spec, but it can be made to work
  static_assert(simd::hypot(std::basic_simd(std::array{1.1f}), 1.2f)[0] == std::hypot(1.1f, 1.2f));
  static_assert(simd::hypot(1.1f, 1.2_f1)[0] == std::hypot(1.1f, 1.2f));
  static_assert(simd::hypot(1_cw, 1.2_f1)[0] == std::hypot(1.f, 1.2f));
  static_assert(simd::hypot(1.2_f1, 1_cw)[0] == std::hypot(1.f, 1.2f));
  static_assert(simd::hypot(holder {1.f}, 1.2_f1)[0] == std::hypot(1.f, 1.2f));
  // the following must not be valid. if you want simd<double> be explicit about it:
  static_assert(not hypot_invocable<int, simd::simd<float, 1>>);
  static_assert(not hypot_invocable<int, simd::simd<float, 1>, simd::simd<float, 1>>);

  static_assert(hypot_invocable_r<simd::simd<float, 1>, holder<float>,
                                  vir::constexpr_wrapper<2>, simd::simd<float, 1>>);
  static_assert(hypot_invocable_r<simd::simd<float, 1>, holder<short>,
                                  simd::simd<float, 1>, float>);
}
