/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#include "simd.h"
#include "mask_reductions.h"

#include <source_location>
#include <iostream>

template <typename T>
  struct is_character_type
  : std::bool_constant<false>
  {};

template <> struct is_character_type<char> : std::bool_constant<true> {};
template <> struct is_character_type<wchar_t> : std::bool_constant<true> {};
template <> struct is_character_type<char8_t> : std::bool_constant<true> {};
template <> struct is_character_type<char16_t> : std::bool_constant<true> {};
template <> struct is_character_type<char32_t> : std::bool_constant<true> {};

std::ostream& operator<<(std::ostream& s, std::byte b)
{ return s << std::hex << static_cast<unsigned>(b) << std::dec; }

template <typename T, typename Abi>
std::ostream& operator<<(std::ostream& s, std::basic_simd<T, Abi> const& v)
{
  using U = std::conditional_t<sizeof(T) == 1, int,
                               std::conditional_t<is_character_type<T>::value,
                                                  std::__detail::__make_unsigned_int_t<T>, T>>;
  s << '[' << U(v[0]);
  for (int i = 1; i < v.size(); ++i)
    s << ", " << U(v[i]);
  return s << ']';
}

template <std::size_t B, typename Abi>
std::ostream& operator<<(std::ostream& s, std::basic_simd_mask<B, Abi> const& v)
{
  s << '<';
  for (int i = 0; i < v.size(); ++i)
    s << int(v[i]);
  return s << '>';
}

static std::int64_t passed_tests = 0;
static std::int64_t failed_tests = 0;

struct additional_info
{
  const bool failed = false;

  additional_info
  operator()(auto const& value0, auto const&... more)
  {
    if (failed)
      [&] {
        std::cout << value0;
        ((std::cout << ' ' << more), ...);
        std::cout << std::endl;
      }();
    return *this;
  }
};

additional_info
log_failure(auto const& x, auto const& y, std::source_location loc, std::string_view s)
{
  ++failed_tests;
  std::cout << loc.file_name() << ':' << loc.line() << ':' << loc.column() << ": "
            << loc.function_name() << '\n' << std::boolalpha;
  std::cout << s << x << "\n       to: " << y << std::endl;
  return additional_info {true};
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
[[gnu::always_inline]]
additional_info
verify_equal(auto&& x, auto&& y,
             std::source_location loc = std::source_location::current())
{
  if (std::all_of(x == y) and std::none_of(x != y))
    {
      ++passed_tests;
      return {};
    }
  else
    return log_failure(x, y, loc, "not equal: ");
}

[[gnu::always_inline]]
additional_info
verify_not_equal(auto&& x, auto&& y,
                 std::source_location loc = std::source_location::current())
{
  if (std::all_of(x != y) and std::none_of(x == y))
    {
      ++passed_tests;
      return {};
    }
  else
    return log_failure(x, y, loc, "    equal: ");
}
#pragma GCC diagnostic pop

template <typename T, template<typename> class Test>
  void
  instantiate_tests_for_value_type()
  {
    if constexpr (std::destructible<std::simd<T>>)
      {
#ifndef UNITTEST_WIDTH
        constexpr int Width = 8;
#else
        constexpr int Width = UNITTEST_WIDTH;
#endif
        constexpr int N = Width <= 64 ? Width // 1-64
                                       : 64 + (Width - 64) * std::simd<T>::size();
        if constexpr (std::destructible<std::simd<T, N>>)
          {
            static_assert(std::simd<T, N>::size() == N);
            Test<std::simd<T, N>>::run();
          }
        else
          static_assert(std::simd<T, N>::size() == 0);
      }
  }

bool
check_cpu_support()
{
#if defined __x86_64__ or defined __i386__
    __builtin_cpu_init();
#ifdef __SSE3__
    if (not __builtin_cpu_supports("sse3")) return false;
#endif
#ifdef __SSSE3__
    if (not __builtin_cpu_supports("ssse3")) return false;
#endif
#ifdef __SSE4_1__
    if (not __builtin_cpu_supports("sse4.1")) return false;
#endif
#ifdef __SSE4_2__
    if (not __builtin_cpu_supports("sse4.2")) return false;
#endif
#ifdef __SSE4A__
    if (not __builtin_cpu_supports("sse4a")) return false;
#endif
#ifdef __XOP__
    if (not __builtin_cpu_supports("xop")) return false;
#endif
#ifdef __FMA__
    if (not __builtin_cpu_supports("fma")) return false;
#endif
#ifdef __FMA4__
    if (not __builtin_cpu_supports("fma4")) return false;
#endif
#ifdef __AVX__
    if (not __builtin_cpu_supports("avx")) return false;
#endif
#ifdef __AVX2__
    if (not __builtin_cpu_supports("avx2")) return false;
#endif
#ifdef __BMI__
    if (not __builtin_cpu_supports("bmi")) return false;
#endif
#ifdef __BMI2__
    if (not __builtin_cpu_supports("bmi2")) return false;
#endif
#ifdef __LZCNT__
    if (not __builtin_cpu_supports("lzcnt")) return false;
#endif
#ifdef __F16C__
    if (not __builtin_cpu_supports("f16c")) return false;
#endif
#ifdef __POPCNT__
    if (not __builtin_cpu_supports("popcnt")) return false;
#endif
#ifdef __AVX512F__
    if (not __builtin_cpu_supports("avx512f")) return false;
#endif
#ifdef __AVX512DQ__
    if (not __builtin_cpu_supports("avx512dq")) return false;
#endif
#ifdef __AVX512BW__
    if (not __builtin_cpu_supports("avx512bw")) return false;
#endif
#ifdef __AVX512VL__
    if (not __builtin_cpu_supports("avx512vl")) return false;
#endif
#ifdef __AVX512BITALG__
    if (not __builtin_cpu_supports("avx512bitalg")) return false;
#endif
#ifdef __AVX512VBMI__
    if (not __builtin_cpu_supports("avx512vbmi")) return false;
#endif
#ifdef __AVX512VBMI2__
    if (not __builtin_cpu_supports("avx512vbmi2")) return false;
#endif
#ifdef __AVX512IFMA__
    if (not __builtin_cpu_supports("avx512ifma")) return false;
#endif
#ifdef __AVX512CD__
    if (not __builtin_cpu_supports("avx512cd")) return false;
#endif
#ifdef __AVX512VNNI__
    if (not __builtin_cpu_supports("avx512vnni")) return false;
#endif
#ifdef __AVX512VPOPCNTDQ__
    if (not __builtin_cpu_supports("avx512vpopcntdq")) return false;
#endif
#ifdef __AVX512VP2INTERSECT__
    if (not __builtin_cpu_supports("avx512vp2intersect")) return false;
#endif
#ifdef __AVX512FP16__
    if (not __builtin_cpu_supports("avx512fp16")) return false;
#endif
#endif
    return true;
}

template <template<typename> class Test>
  void
  instantiate_tests()
  {
    if (not check_cpu_support())
      {
        std::cerr << "Incompatible CPU.\n";
        return;
      }

#ifndef UNITTEST_TYPE
    instantiate_tests_for_value_type<int, Test>();
#else
    instantiate_tests_for_value_type<UNITTEST_TYPE, Test>();
#endif
  }

void
log_start(std::source_location const& loc = std::source_location::current())
{
  std::cout << "Testing " << loc.function_name() << '\n';
}

template <typename T>
  T
  make_value_unknown(const T& x)
  {
    T y = x;
    asm("" : "+m"(y));
    return y;
  }
