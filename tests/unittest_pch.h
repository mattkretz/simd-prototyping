/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef TESTS_UNITTEST_PCH_H_
#define TESTS_UNITTEST_PCH_H_

#include "../simd"

namespace simd = std;

#include <source_location>
#include <iostream>
#include <concepts>

template <typename T>
  struct is_character_type
  : std::bool_constant<false>
  {};

template <typename T>
  inline constexpr bool is_character_type_v = is_character_type<T>::value;

template <typename T>
  struct is_character_type<const T>
  : is_character_type<T>
  {};

template <typename T>
  struct is_character_type<T&>
  : is_character_type<T>
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
  using U = std::conditional_t<
              sizeof(T) == 1, int, std::conditional_t<
                                     is_character_type_v<T>,
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

template <std::__detail::__vec_builtin V>
  std::ostream& operator<<(std::ostream& s, V v)
  { return s << std::simd<std::__detail::__value_type_of<V>, std::__detail::__width_of<V>>(v); }

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
        std::cout << "  " << value0;
        ((std::cout << ' ' << more), ...);
        std::cout << std::endl;
      }();
    return *this;
  }
};

struct log_novalue {};

template <typename X, typename Y>
  additional_info
  log_failure(const X& x, const Y& y, std::source_location loc, std::string_view s)
  {
    ++failed_tests;
    std::cout << loc.file_name() << ':' << loc.line() << ':' << loc.column() << ": in '"
              << loc.function_name() << "' verification failed:\n  " << std::boolalpha;
    std::cout << s;
    if constexpr (is_character_type_v<X>)
      std::cout << int(x);
    else
      std::cout << x;
    if constexpr (not std::is_same_v<decltype(y), const log_novalue&>)
      {
        std::cout << "\n   expected: ";
        if constexpr (is_character_type_v<X>)
          std::cout << int(y);
        else
          std::cout << y;
      }
    std::cout << std::endl;
    return additional_info {true};
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

[[gnu::always_inline]]
additional_info
verify(auto&& k, std::source_location loc = std::source_location::current())
{
  if (std::all_of(k))
    {
      ++passed_tests;
      return {};
    }
  else
    return log_failure(k, log_novalue(), loc, "not true: ");
}

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
#if defined __LZCNT__ and not defined __clang__
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

template <template<typename> class Test>
  void
  instantiate_tests_for_value_type();

using run_function = void(*)();

static std::vector<run_function> run_functions = {};

template <template<typename> class... Tests>
  int
  register_tests()
  {
    (instantiate_tests_for_value_type<Tests>(), ...);
    return 0;
  }

int main()
{
  if (not check_cpu_support())
    {
      std::cerr << "Incompatible CPU.\n";
      return EXIT_SUCCESS;
    }

  for (auto f : run_functions)
    f();

  std::cout << "Passed tests: " << passed_tests << "\nFailed tests: " << failed_tests << '\n';
  return failed_tests != 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

#endif  // TESTS_UNITTEST_PCH_H_
