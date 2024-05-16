/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#include "unittest.h"
#include "fixed_point.h"

template <typename V>
  struct test_fixed_point
  {
    using T = typename V::value_type;

    static void
    run()
    {
      using namespace vir::literals;
      log_start();

      using vir::fixed_point;

      if constexpr (std::integral<T> and V::size() == 1)
        {
          constexpr T max = std::numeric_limits<T>::max();
          fixed_point<T, 0> a = T();
          fixed_point one = T(1);
          static_assert(one.exponent() == 0);
          fixed_point three = T(3);
          static_assert(three.exponent() == 0);
          fixed_point<T, -2> b = 0_cw;
          fixed_point<T, -3> c(1.25f);
          if constexpr (std::is_signed_v<T>)
            {
              fixed_point<T, -2> d(-1.25f);
              fixed_point<T, -3> e(-1.125f);
              verify_equal(c / d, -1_cw);
              verify_equal(d / c, -1_cw);
              verify(d < e)(d, e);
              verify(e > d)(d, e);
              verify_not_equal(d, e);
              verify_equal(a * d, a);
              verify_equal(c * d, fixed_point<T, -4>(-1.25 * 1.25));
              verify_equal(a / d, 0_cw);
              verify_equal(a / e, 0_cw);
            }
          fixed_point<T, -6> f(max >> 6);

          verify_equal(fixed_point<T, 0>(1), fixed_point<T, -2>(1));
          verify_equal(fixed_point<T, -2>(1), fixed_point<T, 0>(1));
          verify_equal(a, a);
          verify_equal(T(a), T());
          verify_equal(float(a), 0.f);

          verify_equal(b, a);
          verify_equal(a, b);
          verify_equal(T(b), T());
          verify_equal(float(b), 0.f);
          verify_equal(f, vir::cw<(max >> 6)>);

          verify_not_equal(c, a);
          verify_equal(static_cast<float>(c), 1.25f);
          verify(a < c)(a, c, static_cast<fixed_point<T, -3>>(a),
                        static_cast<fixed_point<T, 0>>(c));
          verify(a <= c)(a, c);
          verify(c > a)(a, c);
          verify(c >= a)(a, c);

          verify(fixed_point(1) < fixed_point(2));
          verify(not (fixed_point(2) < fixed_point(2)));
          verify(not (fixed_point(3) < fixed_point(2)));
          verify(fixed_point(1) <= fixed_point(2));
          verify(fixed_point(2) <= fixed_point(2));
          verify(not (fixed_point(3) <= fixed_point(2)));

          verify_equal(a / c, 0_cw);
          verify_equal(f / one, f);
          if constexpr (sizeof(long double) > sizeof(double))
            verify_equal(f / c, fixed_point<T, -3>((max >> 6) / 1.25l));
          verify_equal(one / three, 0_cw);
          if constexpr (sizeof(T) >= 4)
            verify_equal(fixed_point<T, -29>(1) / three, fixed_point<T, -29>(1./3));
          verify_equal(fixed_point<T, 50>(1.), 0_cw);
          verify_equal(fixed_point(2_cw).exponent(), 1);
          verify_equal(fixed_point(5_cw).exponent(), 0);
          verify_equal(fixed_point(8_cw).exponent(), 3);
          constexpr int E = 82 - sizeof(T) * CHAR_BIT;
          verify_equal(fixed_point<T, E>(1.e24) / fixed_point(T(2)), fixed_point<T, E>(5.e23))(
            fixed_point<T, E>(1.e24), fixed_point(T(2)), fixed_point<T, E>(5.e23));

          verify_equal(++one, 2_cw);
          verify_equal(--one, 1_cw);
          verify_equal(one++, 1_cw);
          verify_equal(one--, 2_cw);
          verify_equal(one, 1_cw);

          verify_equal(one << 1, 2_cw);
          verify_equal(one >> 1, 0_cw);
          verify_equal(one >> 1_cw, fixed_point<T, -1>(.5));

          verify_equal(~fixed_point<T, 0>() * fixed_point(T(2)), ~fixed_point<T, 1>() >> 1);
          verify_equal(~fixed_point<T, 0>() * 2_cw, ~fixed_point<T, 1>());
          verify_equal(~fixed_point<T, 0>() * 6_cw, ~fixed_point<T, 1>() * 3_cw);
      }
    }
  };

template <typename V0>
  struct simd_udt
  {
    using T = typename V0::value_type;
    template <int E>
      using V = std::rebind_simd_t<vir::fixed_point<T, E>, V0>;

    static void
    run()
    {
      using namespace vir::literals;
      log_start();

      using vir::fixed_point;

      if constexpr (std::integral<T>)
        {
          V<0> x = {};
          verify_equal(x, x);
        }
    }
  };

auto tests = register_tests<test_fixed_point, simd_udt>();
