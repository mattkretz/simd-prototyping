/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "unittest.h"

#include "../simd"

using namespace vir::literals;

template <typename V>
  struct plus
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;

    static void
    run()
    {
      log_start();

      {
        V x = {};
        V y = {};
        verify_equal(x + y, x);
        verify_equal(x = x + T(1), T(1));
        verify_equal(x + x, T(2));
        y = vec<V, 1, 2, 3, 4, 5, 6, 7>;
        verify_equal(x = x + y, vec<V, 2, 3, 4, 5, 6, 7, 8>);
        verify_equal(x = x + -y, T(1));
        verify_equal(x += y, vec<V, 2, 3, 4, 5, 6, 7, 8>);
        verify_equal(x, vec<V, 2, 3, 4, 5, 6, 7, 8>);
        verify_equal(x += -y, T(1));
        verify_equal(x, T(1));

        x = y = V();
        x = make_value_unknown(x);
        y = make_value_unknown(y);
        verify_equal(x + y, x);
        verify_equal(x = x + T(1), T(1));
        verify_equal(x + x, T(2));
        y = vec<V, 1, 2, 3, 4, 5, 6, 7>;
        verify_equal(x = x + y, vec<V, 2, 3, 4, 5, 6, 7, 8>);
        verify_equal(x = x + -y, T(1));
        verify_equal(x += y, vec<V, 2, 3, 4, 5, 6, 7, 8>);
        verify_equal(x, vec<V, 2, 3, 4, 5, 6, 7, 8>);
        verify_equal(x += -y, T(1));
        verify_equal(x, T(1));
      }

      auto x = test_iota<V>;
      verify_equal(x + 0_cw, x);
      verify_equal(0_cw + x, x);
      verify_equal(x + T(), x);
      verify_equal(T() + x, x);
      verify_equal(x + -x, V());
      verify_equal(-x + x, V());

      x = make_value_unknown(x);
      verify_equal(x + 0_cw, x);
      verify_equal(0_cw + x, x);
      verify_equal(x + T(), x);
      verify_equal(T() + x, x);
      verify_equal(x + -x, V());
      verify_equal(-x + x, V());
    }
  };

template <typename V>
  struct minus
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;

    static void
    run()
    {
      log_start();

      {
        V x = T(1);
        V y = T(0);
        verify_equal(x - y, x);
        verify_equal(x - T(1), y);
        verify_equal(y, x - T(1));
        verify_equal(x - x, y);
        y = vec<V, 1, 2, 3, 4, 5, 6, 7>;
        verify_equal(x = y - x, vec<V, 0, 1, 2, 3, 4, 5, 6>);
        verify_equal(x = y - x, V(1));
        verify_equal(y -= x, vec<V, 0, 1, 2, 3, 4, 5, 6>);
        verify_equal(y, vec<V, 0, 1, 2, 3, 4, 5, 6>);
        verify_equal(y -= y, V(0));
        verify_equal(y, V(0));

        x = T(1);
        y = T(0);
        x = make_value_unknown(x);
        y = make_value_unknown(y);
        verify_equal(x - y, x);
        verify_equal(x - T(1), y);
        verify_equal(y, x - T(1));
        verify_equal(x - x, y);
        y = vec<V, 1, 2, 3, 4, 5, 6, 7>;
        verify_equal(x = y - x, vec<V, 0, 1, 2, 3, 4, 5, 6>);
        verify_equal(x = y - x, V(1));
        verify_equal(y -= x, vec<V, 0, 1, 2, 3, 4, 5, 6>);
        verify_equal(y, vec<V, 0, 1, 2, 3, 4, 5, 6>);
        verify_equal(y -= y, V(0));
        verify_equal(y, V(0));
      }

      auto x = test_iota<V>;
      verify_equal(x - x, V());
      verify_equal(x - 0_cw, x);
      verify_equal(0_cw - x, -x);
      verify_equal(x - T(), x);
      verify_equal(T() - x, -x);

      x = make_value_unknown(x);
      verify_equal(x - x, V());
      verify_equal(x - 0_cw, x);
      verify_equal(0_cw - x, -x);
      verify_equal(x - T(), x);
      verify_equal(T() - x, -x);
    }
  };

template <typename V>
  struct times
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;

    static void
    run()
    {
      log_start();

      constexpr auto min = std::numeric_limits<T>::lowest();
      constexpr auto norm_min = std::numeric_limits<T>::min();
      constexpr auto max = std::numeric_limits<T>::max();

      {
        V x = T(1);
        V y = T(0);
        verify_equal(x * y, y);
        verify_equal(x = x * T(2), T(2));
        verify_equal(x * x, T(4));
        y = vec<V, 1, 2, 3, 4, 5, 6, 7>;
        verify_equal(x = x * y, vec<V, 2, 4, 6, 8, 10, 12, 14>);
        y = T(2);
        // don't test norm_min/2*2 in the following. There's no guarantee, in
        // general, that the result isn't flushed to zero (e.g. NEON without
        // subnormals)
        for (T n : {T(max - 1), std::is_floating_point_v<T> ? T(norm_min * 3) : min})
          {
            x = T(n / 2);
            verify_equal(x * y, V(n));
          }
        if (std::is_integral<T>::value && std::is_unsigned<T>::value)
          {
            // test modulo arithmetics
            T n = max;
            x = n;
            for (T m : {T(2), T(7), T(max / 127), max})
              {
                y = m;
                // if T is of lower rank than int, `n * m` will promote to int
                // before executing the multiplication. In this case an overflow
                // will be UB (and ubsan will warn about it). The solution is to
                // cast to uint in that case.
                using U
                  = std::conditional_t<(sizeof(T) < sizeof(int)), unsigned, T>;
                verify_equal(x * y, V(T(U(n) * U(m))));
              }
          }
        x = T(2);
        verify_equal(x *= vec<V, 1, 2, 3>, vec<V, 2, 4, 6>);
        verify_equal(x, vec<V, 2, 4, 6>);
      }

      auto x = test_iota<V, 0, 11>;
      verify_equal(x * x, V([](int i) { return T(T(i % 12) * T(i % 12)); }));
      verify_equal(x * 1_cw, x);
      verify_equal(1_cw * x, x);
      verify_equal(x * T(1), x);
      verify_equal(T(1) * x, x);
      verify_equal(x * T(-1), -x);
      verify_equal(T(-1) * x, -x);

      x = make_value_unknown(x);
      verify_equal(x * x, V([](int i) { return T(T(i % 12) * T(i % 12)); }));
      verify_equal(x * 1_cw, x);
      verify_equal(1_cw * x, x);
      verify_equal(x * T(1), x);
      verify_equal(T(1) * x, x);
      verify_equal(x * T(-1), -x);
      verify_equal(T(-1) * x, -x);
    }
  };

template <typename V>
  struct divide
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;

    static void
    run()
    {
      log_start();

      auto x = test_iota<V, 1>;
      verify_equal(x / x, V(T(1)));
      verify_equal(x / 1_cw, x);
      verify_equal(x / T(1), x);

      x = make_value_unknown(x);
      verify_equal(x / x, V(T(1)));
      verify_equal(x / 1_cw, x);
      verify_equal(x / T(1), x);
    }
  };

auto tests = register_tests<plus, minus, times, divide>();
