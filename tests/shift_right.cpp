/* SPDX-License-Identifier: BSD-3-Clause */
// Copyright © 2023–2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

#include "unittest.h"

#include "../simd"

using namespace vir::literals;

template <typename V>
  struct shift_right
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;

    static void
    run()
    {
      if constexpr (std::is_integral_v<T>)
        {
          constexpr int max = sizeof(T) == 8 ? 64 : 32;
          auto test_shift = [=](auto _shift) {
            log_start();

            const auto x = std::simd_iota<V>;
            const auto y = ~x;
            constexpr T tmax = std::numeric_limits<T>::max();
            const auto z = tmax - x;
            constexpr int shift = _shift;
            constexpr V vshift = T(shift);
            const V vshiftx = vshift ^ (x & 1_cw);

            V ref([](T i) -> T { return i >> shift; });
            V refx([](T i) -> T { return i >> (shift ^ (i & 1)); });
            verify_equal(x >> shift, ref)(x, ">>", shift);
            verify_equal(x >> vshift, ref)(x, ">>", vshift);
            verify_equal(x >> vshiftx, refx)(x, ">>", vshiftx);
            verify_equal(make_value_unknown(x)._M_is_constprop(), false);
            verify_equal(make_value_unknown(x) >> shift, ref)(x, ">>", shift);
            verify_equal(make_value_unknown(x) >> vshift, ref)(x, ">>", vshift);
            verify_equal(make_value_unknown(x) >> vshiftx, refx)(x, ">>", vshiftx);

            ref = V([](T i) -> T { return T(~i) >> shift; });
            refx = V([](T i) -> T { return T(~i) >> (shift ^ (i & 1)); });
            verify_equal(y >> shift, ref)(y, ">>", shift);
            verify_equal(y >> vshift, ref)(y, ">>", vshift);
            verify_equal(y >> vshiftx, refx)(y, ">>", vshiftx);
            verify_equal(make_value_unknown(y)._M_is_constprop(), false);
            verify_equal(make_value_unknown(y) >> shift, ref)(y, ">>", shift);
            verify_equal(make_value_unknown(y) >> vshift, ref)(y, ">>", vshift);
            verify_equal(make_value_unknown(y) >> vshiftx, refx)(y, ">>", vshiftx);

            ref = V([](T i) -> T { return T(tmax - i) >> shift; });
            refx = V([](T i) -> T { return T(tmax - i) >> (shift ^ (i & 1)); });
            verify_equal(z >> shift, ref)(z, ">>", shift);
            verify_equal(z >> vshift, ref)(z, ">>", vshift);
            verify_equal(z >> vshiftx, refx)(z, ">>", vshiftx);
            verify_equal(make_value_unknown(z)._M_is_constprop(), false);
            verify_equal(make_value_unknown(z) >> shift, ref)(z, ">>", shift);
            verify_equal(make_value_unknown(z) >> vshift, ref)(z, ">>", vshift);
            verify_equal(make_value_unknown(z) >> vshiftx, refx)(z, ">>", vshiftx);
          };
          _GLIBCXX_SIMD_INT_PACK(max, shift, {
            (test_shift(vir::cw<shift>), ...);
          });
        }
    }
  };

auto tests = register_tests<shift_right>();
