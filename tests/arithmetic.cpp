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

      auto x = std::simd_iota<V>;
      verify_equal(x + x, V([](T i) { return T(i + i); }));
      verify_equal(x + 0_cw, x);
      verify_equal(0_cw + x, x);
      verify_equal(x + T(), x);
      verify_equal(T() + x, x);
      verify_equal(x + -x, V());
      verify_equal(-x + x, V());

      x = make_value_unknown(x);
      verify_equal(x + x, V([](T i) { return T(i + i); }));
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

      auto x = std::simd_iota<V>;
      verify_equal(x - x, V());
      verify_equal(x - 0_cw, x);
      verify_equal(0_cw - x, -x);
      verify_equal(x - T(), x);
      verify_equal(T() - x, -x);
      verify_equal(x - -x, V([](T i) { return T(i - -i); }));
      verify_equal(-x - x, V([](T i) { return T(-i - i); }));

      x = make_value_unknown(x);
      verify_equal(x - x, V());
      verify_equal(x - 0_cw, x);
      verify_equal(0_cw - x, -x);
      verify_equal(x - T(), x);
      verify_equal(T() - x, -x);
      verify_equal(x - -x, V([](T i) { return T(i - -i); }));
      verify_equal(-x - x, V([](T i) { return T(-i - i); }));
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

      auto x = std::simd_iota<V>;
      verify_equal(x * x, V([](T i) { return T(i * i); }));
      verify_equal(x * 1_cw, x);
      verify_equal(1_cw * x, x);
      verify_equal(x * T(1), x);
      verify_equal(T(1) * x, x);
      verify_equal(x * T(-1), -x);
      verify_equal(T(-1) * x, -x);

      x = make_value_unknown(x);
      verify_equal(x * x, V([](T i) { return T(i * i); }));
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

      auto x = std::simd_iota<V> + T(1);
      verify_equal(x / x, V(T(1)));
      verify_equal(x / 1_cw, x);
      verify_equal(1_cw / x, V([](T i) { return T(T(1) / (i + 1)); }));
      verify_equal(x / T(1), x);
      verify_equal(T(1) / x, V([](T i) { return T(T(1) / (i + 1)); }));

      x = make_value_unknown(x);
      verify_equal(x / x, V(T(1)));
      verify_equal(x / 1_cw, x);
      verify_equal(1_cw / x, V([](T i) { return T(T(1) / (i + 1)); }));
      verify_equal(x / T(1), x);
      verify_equal(T(1) / x, V([](T i) { return T(T(1) / (i + 1)); }));
    }
  };

auto tests = register_tests<plus, minus, times, divide>();
