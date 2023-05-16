/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#define SIMPLE_CONVERSIONS 3
#define INT_NOT_SPECIAL 1

#include "../simd.h"

template <std::__detail::__vectorizable T, typename U = T>
  requires std::__detail::__simd_type<std::simd<U>>
  using V = std::rebind_simd_t<T, std::simd<U>>;

template <std::__detail::__vectorizable T>
  using V4 = std::resize_simd_t<4, std::simd<T>>;

template <auto X>
  inline constexpr std::integral_constant<std::remove_const_t<decltype(X)>, X> c {};

struct Error {};
using Err = Error;
struct LHS {};
struct RHS {};

template <typename Op1, typename Op2, typename Result0,
          typename Result1 = Result0, typename Result2 = Result1, typename Result3 = Result2>
  void expect()
  {
    using RR =
#if SIMPLE_CONVERSIONS == 3
      Result3;
#elif SIMPLE_CONVERSIONS == 2
      Result2;
#elif SIMPLE_CONVERSIONS == 1
      Result1;
#else
      Result0;
#endif
    using R = std::conditional_t<std::same_as<RR, LHS>,
                                 typename std::conditional_t<std::__detail::__vectorizable<Op1>,
                                                             std::rebind_simd<Op1, Op2>,
                                                             std::type_identity<Op1>>::type,
                                 std::conditional_t<std::same_as<RR, RHS>, Op2, RR>>;
    if constexpr (std::convertible_to<Op1, Op2> or std::convertible_to<Op2, Op1>
                    or requires (Op1 a, Op2 b) { {a + b}; })
      static_assert(std::same_as<R, decltype(std::declval<Op1>() + std::declval<Op2>())>);
    else
      static_assert(std::same_as<R, Error>);
  }

#if INT_NOT_SPECIAL
using Rrr = Err;
#else
using Rrr = RHS;
#endif

void
test()
{
  using schar = signed char;
  using uchar = unsigned char;
  using sshort = signed short;
  using ushort = unsigned short;
  using sint = signed int;
  using uint = unsigned int;
  using slong = signed long;
  using ulong = unsigned long;
  using sllong = signed long long;
  using ullong = unsigned long long;

  expect< schar, V4<sshort>, RHS, RHS, RHS>();
  expect< uchar, V4<sshort>, RHS, RHS, RHS>();
  expect<sshort, V4<sshort>, RHS, RHS, RHS>();
  expect<ushort, V4<sshort>, Err, Err, LHS>();
  expect<  sint, V4<sshort>, Rrr, Err, LHS>();
  expect<  uint, V4<sshort>, Err, Err, LHS>();
  expect< slong, V4<sshort>, Err, Err, LHS>();
  expect< ulong, V4<sshort>, Err, Err, LHS>();
  expect<sllong, V4<sshort>, Err, Err, LHS>();
  expect<ullong, V4<sshort>, Err, Err, LHS>();
  expect< float, V4<sshort>, Err, Err, LHS>();
  expect<double, V4<sshort>, Err, Err, LHS>();
  expect<  bool, V4<sshort>, Err, RHS, RHS>();

  expect<V4< schar>, V4<sshort>, RHS, RHS, RHS>();
  expect<V4< uchar>, V4<sshort>, RHS, RHS, RHS>();
  expect<V4<sshort>, V4<sshort>, RHS, RHS, RHS>();
  expect<V4<ushort>, V4<sshort>, Err, LHS, LHS>();
  expect<V4<  sint>, V4<sshort>, LHS, LHS, LHS>();
  expect<V4<  uint>, V4<sshort>, Err, LHS, LHS>();
  expect<V4< slong>, V4<sshort>, LHS, LHS, LHS>();
  expect<V4< ulong>, V4<sshort>, Err, LHS, LHS>();
  expect<V4<sllong>, V4<sshort>, LHS, LHS, LHS>();
  expect<V4<ullong>, V4<sshort>, Err, LHS, LHS>();
  expect<V4< float>, V4<sshort>, LHS, LHS, LHS>();
  expect<V4<double>, V4<sshort>, LHS, LHS, LHS>();

  expect< schar, V4<ushort>, Err, RHS, RHS>();
  expect< uchar, V4<ushort>, RHS, RHS, RHS>();
  expect<sshort, V4<ushort>, Err, RHS, RHS>();
  expect<ushort, V4<ushort>, RHS, RHS, RHS>();
  expect<  sint, V4<ushort>, Rrr, Err, LHS>();
  expect<  uint, V4<ushort>, Rrr, Err, LHS>();
  expect< slong, V4<ushort>, Err, Err, LHS>();
  expect< ulong, V4<ushort>, Err, Err, LHS>();
  expect<sllong, V4<ushort>, Err, Err, LHS>();
  expect<ullong, V4<ushort>, Err, Err, LHS>();
  expect< float, V4<ushort>, Err, Err, LHS>();
  expect<double, V4<ushort>, Err, Err, LHS>();
  expect<  bool, V4<ushort>, Err, RHS, RHS>();

  expect<V4< schar>, V4<ushort>, Err, RHS, RHS>();
  expect<V4< uchar>, V4<ushort>, RHS, RHS, RHS>();
  expect<V4<sshort>, V4<ushort>, Err, RHS, RHS>();
  expect<V4<ushort>, V4<ushort>, RHS, RHS, RHS>();
  expect<V4<  sint>, V4<ushort>, LHS, LHS, LHS>();
  expect<V4<  uint>, V4<ushort>, LHS, LHS, LHS>();
  expect<V4< slong>, V4<ushort>, LHS, LHS, LHS>();
  expect<V4< ulong>, V4<ushort>, LHS, LHS, LHS>();
  expect<V4<sllong>, V4<ushort>, LHS, LHS, LHS>();
  expect<V4<ullong>, V4<ushort>, LHS, LHS, LHS>();
  expect<V4< float>, V4<ushort>, LHS, LHS, LHS>();
  expect<V4<double>, V4<ushort>, LHS, LHS, LHS>();

  expect< schar, V4<uint>, Err, RHS, RHS>();
  expect< uchar, V4<uint>, RHS, RHS, RHS>();
  expect<sshort, V4<uint>, Err, RHS, RHS>();
  expect<ushort, V4<uint>, RHS, RHS, RHS>();
  expect<  sint, V4<uint>, Rrr, RHS, RHS>();
  expect<  uint, V4<uint>, RHS, RHS, RHS>();
  expect< slong, V4<uint>, Err, Err, LHS>();
  expect< ulong, V4<uint>, Err, Err, LHS>();
  expect<sllong, V4<uint>, Err, Err, LHS>();
  expect<ullong, V4<uint>, Err, Err, LHS>();
  expect< float, V4<uint>, Err, Err, LHS>();
  expect<double, V4<uint>, Err, Err, LHS>();
  expect<  bool, V4<uint>, Err, RHS, RHS>();

  expect<V4< schar>, V4<uint>, Err, RHS, RHS>();
  expect<V4< uchar>, V4<uint>, RHS, RHS, RHS>();
  expect<V4<sshort>, V4<uint>, Err, RHS, RHS>();
  expect<V4<ushort>, V4<uint>, RHS, RHS, RHS>();
  expect<V4<  sint>, V4<uint>, Err, RHS, RHS>();
  expect<V4<  uint>, V4<uint>, RHS, RHS, RHS>();
  expect<V4< slong>, V4<uint>, LHS, LHS, LHS>();
  expect<V4< ulong>, V4<uint>, LHS, LHS, LHS>();
  expect<V4<sllong>, V4<uint>, LHS, LHS, LHS>();
  expect<V4<ullong>, V4<uint>, LHS, LHS, LHS>();
  expect<V4< float>, V4<uint>, Err, LHS, LHS>();
  expect<V4<double>, V4<uint>, LHS, LHS, LHS>();

  expect< schar, V4<ulong>, Err, RHS, RHS>();
  expect< uchar, V4<ulong>, RHS, RHS, RHS>();
  expect<sshort, V4<ulong>, Err, RHS, RHS>();
  expect<ushort, V4<ulong>, RHS, RHS, RHS>();
  expect<  sint, V4<ulong>, Rrr, RHS, RHS>();
  expect<  uint, V4<ulong>, RHS, RHS, RHS>();
  expect< slong, V4<ulong>, Err, RHS, RHS>();
  expect< ulong, V4<ulong>, RHS, RHS, RHS>();
  expect<sllong, V4<ulong>, Err, Err, V4<ullong>>();
  expect<ullong, V4<ulong>, RHS, Err, LHS>();
  expect< float, V4<ulong>, Err, Err, LHS>();
  expect<double, V4<ulong>, Err, Err, LHS>();
  expect<  bool, V4<ulong>, Err, RHS, RHS>();

  expect<V4< schar>, V4<ulong>, Err, RHS, RHS>();
  expect<V4< uchar>, V4<ulong>, RHS, RHS, RHS>();
  expect<V4<sshort>, V4<ulong>, Err, RHS, RHS>();
  expect<V4<ushort>, V4<ulong>, RHS, RHS, RHS>();
  expect<V4<  sint>, V4<ulong>, Err, RHS, RHS>();
  expect<V4<  uint>, V4<ulong>, RHS, RHS, RHS>();
  expect<V4< slong>, V4<ulong>, Err, RHS, RHS>();
  expect<V4< ulong>, V4<ulong>, RHS, RHS, RHS>();
  expect<V4<sllong>, V4<ulong>, Err, Err, V4<ullong>>();
  expect<V4<ullong>, V4<ulong>, LHS, LHS, LHS>();
  expect<V4< float>, V4<ulong>, Err, LHS, LHS>();
  expect<V4<double>, V4<ulong>, Err, LHS, LHS>();

  expect< schar, V4<float>, RHS, RHS, RHS>();
  expect< uchar, V4<float>, RHS, RHS, RHS>();
  expect<sshort, V4<float>, RHS, RHS, RHS>();
  expect<ushort, V4<float>, RHS, RHS, RHS>();
  expect<  sint, V4<float>, Rrr, RHS, RHS>();
  expect<  uint, V4<float>, Err, RHS, RHS>();
  expect< slong, V4<float>, Err, RHS, RHS>();
  expect< ulong, V4<float>, Err, RHS, RHS>();
  expect<sllong, V4<float>, Err, RHS, RHS>();
  expect<ullong, V4<float>, Err, RHS, RHS>();
  expect< float, V4<float>, RHS, RHS, RHS>();
  expect<double, V4<float>, Err, Err, LHS>();
  expect<  bool, V4<float>, Err, RHS, RHS>();

  expect<V4< schar>, V4<float>, RHS, RHS, RHS>();
  expect<V4< uchar>, V4<float>, RHS, RHS, RHS>();
  expect<V4<sshort>, V4<float>, RHS, RHS, RHS>();
  expect<V4<ushort>, V4<float>, RHS, RHS, RHS>();
  expect<V4<  sint>, V4<float>, Err, RHS, RHS>();
  expect<V4<  uint>, V4<float>, Err, RHS, RHS>();
  expect<V4< slong>, V4<float>, Err, RHS, RHS>();
  expect<V4< ulong>, V4<float>, Err, RHS, RHS>();
  expect<V4<sllong>, V4<float>, Err, RHS, RHS>();
  expect<V4<ullong>, V4<float>, Err, RHS, RHS>();
  expect<V4< float>, V4<float>, RHS, RHS, RHS>();
  expect<V4<double>, V4<float>, LHS, LHS, LHS>();

  expect< schar, V4<double>, RHS, RHS, RHS>();
  expect< uchar, V4<double>, RHS, RHS, RHS>();
  expect<sshort, V4<double>, RHS, RHS, RHS>();
  expect<ushort, V4<double>, RHS, RHS, RHS>();
  expect<  sint, V4<double>, RHS, RHS, RHS>();
  expect<  uint, V4<double>, RHS, RHS, RHS>();
  expect< slong, V4<double>, Err, RHS, RHS>();
  expect< ulong, V4<double>, Err, RHS, RHS>();
  expect<sllong, V4<double>, Err, RHS, RHS>();
  expect<ullong, V4<double>, Err, RHS, RHS>();
  expect< float, V4<double>, RHS, RHS, RHS>();
  expect<double, V4<double>, RHS, RHS, RHS>();
  expect<  bool, V4<double>, Err, RHS, RHS>();

  expect<V4< schar>, V4<double>, RHS, RHS, RHS>();
  expect<V4< uchar>, V4<double>, RHS, RHS, RHS>();
  expect<V4<sshort>, V4<double>, RHS, RHS, RHS>();
  expect<V4<ushort>, V4<double>, RHS, RHS, RHS>();
  expect<V4<  sint>, V4<double>, RHS, RHS, RHS>();
  expect<V4<  uint>, V4<double>, RHS, RHS, RHS>();
  expect<V4< slong>, V4<double>, Err, RHS, RHS>();
  expect<V4< ulong>, V4<double>, Err, RHS, RHS>();
  expect<V4<sllong>, V4<double>, Err, RHS, RHS>();
  expect<V4<ullong>, V4<double>, Err, RHS, RHS>();
  expect<V4< float>, V4<double>, RHS, RHS, RHS>();
  expect<V4<double>, V4<double>, RHS, RHS, RHS>();

#if SIMPLE_CONVERSIONS == 1
static_assert( std::convertible_to<    char, V<short>>);
static_assert(!std::convertible_to<     int, V<short>>);
static_assert( std::convertible_to<decltype(c<0>), V<short>>);
static_assert( std::convertible_to<decltype(c<-1>), V<short>>);
static_assert( std::convertible_to<decltype(c<1>), V<unsigned short>>);
static_assert(!std::convertible_to<decltype(c<-1>), V<unsigned short>>);
static_assert(!std::convertible_to<unsigned, V<short>>);
static_assert(!std::convertible_to<     int, V<unsigned short>>);
static_assert(!std::convertible_to<unsigned, V<unsigned short>>);
static_assert( std::convertible_to<     int, V<int>>);
static_assert( std::convertible_to<     int, V<unsigned int>>);
static_assert( std::convertible_to<     int, V<long>>);
static_assert( std::convertible_to<     int, V<unsigned long>>);
static_assert(!std::convertible_to<unsigned, V<int>>);
static_assert( std::convertible_to<unsigned, V<unsigned int>>);
static_assert( std::convertible_to<unsigned, V<long>>);
static_assert( std::convertible_to<unsigned, V<unsigned long>>);

static_assert( std::convertible_to<V<char, short>, V<short>>);
static_assert(!std::convertible_to<V<short>, V<char>>);
static_assert( std::convertible_to<V<long>, V<long long>>);
static_assert(!std::convertible_to<V<long long>, V<long>>);
#elif SIMPLE_CONVERSIONS == 0
static_assert( std::convertible_to<    char, V<short>>);
//static_assert( std::convertible_to<     int, V<short>>);
static_assert(!std::convertible_to<unsigned, V<short>>);
//static_assert( std::convertible_to<     int, V<unsigned short>>);
//static_assert( std::convertible_to<unsigned, V<unsigned short>>);
static_assert( std::convertible_to<     int, V<int>>);
//static_assert( std::convertible_to<     int, V<unsigned int>>);
static_assert( std::convertible_to<     int, V<long>>);
//static_assert( std::convertible_to<     int, V<unsigned long>>);
static_assert(!std::convertible_to<unsigned, V<int>>);
static_assert( std::convertible_to<unsigned, V<unsigned int>>);
static_assert( std::convertible_to<unsigned, V<long>>);
static_assert( std::convertible_to<unsigned, V<unsigned long>>);

static_assert( std::convertible_to<V<char, short>, V<short>>);
static_assert(!std::convertible_to<V<short>, V<char>>);
static_assert( std::convertible_to<V<long>, V<long long>>);
static_assert(!std::convertible_to<V<long long>, V<long>>);
#endif
}

void f0(std::common_type_t<V<char, short>, V<signed short>>) {}
//void f1(std::common_type_t<long, V<float>>) {}
//void f2(std::common_type_t<unsigned long long, V<float>>) {}
//void f3(decltype(float() + V<int>())) {}
//void f4(decltype(int() + V<float>())) {}
//void f5(decltype(V<int, float>() + V<float>())) {}
//void f6(decltype(V<long long, float>() + V<float>())) {}
