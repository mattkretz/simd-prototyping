/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "unittest.h"

#include "../simd"

using namespace vir::literals;

template <typename V>
  struct Min
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;
    static constexpr bool use_iota = V::size() <= std::numeric_limits<T>::max();

    static void
    run() requires std::totally_ordered<T>
    {
      log_start();

      auto x = [] {
        if constexpr (use_iota)
          return std::simd_iota<V>;
        else
          return V([](int i) { return T(i & 7); });
      }();
      auto y = T(use_iota ? V::size() : 8) - x;
      verify_equal(min(x, x), x);
      verify_equal(min(V(), x), V());
      verify_equal(min(x, V()), V());
      if constexpr (std::is_signed_v<T>)
        {
          verify_equal(min(-x, x), -x);
          verify_equal(min(x, -x), -x);
        }
      verify_equal(min(x + T(1), x), x);
      verify_equal(min(x, x + T(1)), x);
      verify_equal(min(x, y), min(y, x));
      if constexpr (use_iota)
        verify_equal(min(x, y), V([](T i) { return std::min(T(V::size() - i), i); }));

      x = make_value_unknown(x);
      y = make_value_unknown(y);
      verify_equal(min(x, x), x);
      verify_equal(min(V(), x), V());
      verify_equal(min(x, V()), V());
      if constexpr (std::is_signed_v<T>)
        {
          verify_equal(min(-x, x), -x);
          verify_equal(min(x, -x), -x);
        }
      verify_equal(min(x + T(1), x), x);
      verify_equal(min(x, x + T(1)), x);
      verify_equal(min(x, y), min(y, x));
      if constexpr (use_iota)
        verify_equal(min(x, y), V([](T i) { return std::min(T(V::size() - i), i); }));
    }
  };

template <typename V>
  struct Max
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;
    static constexpr bool use_iota = V::size() <= std::numeric_limits<T>::max();

    static void
    run() requires std::totally_ordered<T>
    {
      log_start();

      auto x = [] {
        if constexpr (use_iota)
          return std::simd_iota<V>;
        else
          return V([](int i) { return T(i & 7); });
      }();
      auto y = T(use_iota ? V::size() : 8) - x;
      verify_equal(max(x, x), x);
      verify_equal(max(V(), x), x);
      verify_equal(max(x, V()), x);
      if constexpr (std::is_signed_v<T>)
        {
          verify_equal(max(-x, x), x);
          verify_equal(max(x, -x), x);
        }
      verify_equal(max(x + T(1), x), x + T(1));
      verify_equal(max(x, x + T(1)), x + T(1));
      verify_equal(max(x, y), max(y, x));
      if constexpr (use_iota)
        verify_equal(max(x, y), V([](T i) { return std::max(T(V::size() - i), i); }));

      x = make_value_unknown(x);
      y = make_value_unknown(y);
      verify_equal(max(x, x), x);
      verify_equal(max(V(), x), x);
      verify_equal(max(x, V()), x);
      if constexpr (std::is_signed_v<T>)
        {
          verify_equal(max(-x, x), x);
          verify_equal(max(x, -x), x);
        }
      verify_equal(max(x + T(1), x), x + T(1));
      verify_equal(max(x, x + T(1)), x + T(1));
      verify_equal(max(x, y), max(y, x));
      if constexpr (use_iota)
        verify_equal(max(x, y), V([](T i) { return std::max(T(V::size() - i), i); }));
    }
  };

template <typename V>
  struct Minmax
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;
    static constexpr bool use_iota = V::size() <= std::numeric_limits<T>::max();

    static void
    run() requires std::totally_ordered<T>
    {
      log_start();

      using pair = std::pair<V, V>;

      auto x = [] {
        if constexpr (use_iota)
          return std::simd_iota<V>;
        else
          return V([](int i) { return T(i & 7); });
      }();
      auto y = T(use_iota ? V::size() : 8) - x;
      verify_equal(minmax(x, x), pair{x, x});
      verify_equal(minmax(V(), x), pair{V(), x});
      verify_equal(minmax(x, V()), pair{V(), x});
      if constexpr (std::is_signed_v<T>)
        {
          verify_equal(minmax(-x, x), pair{-x, x});
          verify_equal(minmax(x, -x), pair{-x, x});
        }
      verify_equal(minmax(x + T(1), x), pair{x, x + T(1)});
      verify_equal(minmax(x, x + T(1)), pair{x, x + T(1)});
      verify_equal(minmax(x, y), minmax(y, x));
      if constexpr (use_iota)
        verify_equal(minmax(x, y), pair{V([](T i) { return std::min(T(V::size() - i), i); }),
                                        V([](T i) { return std::max(T(V::size() - i), i); })});

      x = make_value_unknown(x);
      y = make_value_unknown(y);
      verify_equal(minmax(x, x), pair{x, x});
      verify_equal(minmax(V(), x), pair{V(), x});
      verify_equal(minmax(x, V()), pair{V(), x});
      if constexpr (std::is_signed_v<T>)
        {
          verify_equal(minmax(-x, x), pair{-x, x});
          verify_equal(minmax(x, -x), pair{-x, x});
        }
      verify_equal(minmax(x + T(1), x), pair{x, x + T(1)});
      verify_equal(minmax(x, x + T(1)), pair{x, x + T(1)});
      verify_equal(minmax(x, y), minmax(y, x));
      if constexpr (use_iota)
        verify_equal(minmax(x, y), pair{V([](T i) { return std::min(T(V::size() - i), i); }),
                                        V([](T i) { return std::max(T(V::size() - i), i); })});
    }
  };

template <typename V>
  struct Clamp
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;
    static constexpr bool use_iota = V::size() <= std::numeric_limits<T>::max();

    static void
    run() requires std::totally_ordered<T>
    {
      log_start();

      if constexpr (V::size() <= std::simd<T>::size())
        { // avoid GCC 15.0 ICE
          auto x = [] {
            if constexpr (use_iota)
              return std::simd_iota<V>;
            else
              return V([](int i) { return T(i & 7); });
          }();
          auto y = T(use_iota ? V::size() : 8) - x;
          verify_equal(clamp(x, V(), x), x);
          verify_equal(clamp(x, x, x), x);
          verify_equal(clamp(V(), x, x), x);
          verify_equal(clamp(V(), V(), x), V());
          verify_equal(clamp(x, V(), V()), V());
          verify_equal(clamp(x, V(), y), min(x, y));
          verify_equal(clamp(y, V(), x), min(x, y));
          if constexpr (std::is_signed_v<T>)
            {
              verify_equal(clamp(V(T(-V::size())), -x, x), -x);
              verify_equal(clamp(V(T(V::size())), -x, x), x);
            }

          x = make_value_unknown(x);
          y = make_value_unknown(y);
          verify_equal(clamp(x, x, x), x);
          verify_equal(clamp(V(), x, x), x);
          verify_equal(clamp(x, V(), x), x);
          verify_equal(clamp(V(), V(), x), V());
          verify_equal(clamp(x, V(), V()), V());
          verify_equal(clamp(x, V(), y), min(x, y));
          verify_equal(clamp(y, V(), x), min(x, y));
          if constexpr (std::is_signed_v<T>)
            {
              verify_equal(clamp(V(T(-V::size())), -x, x), -x);
              verify_equal(clamp(V(T(V::size())), -x, x), x);
            }
        }
    }
  };

template <typename V>
  struct Select
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;

    static void
    run()
    {
      log_start();

      auto x = test_iota<V, 0, 63>;
      auto y = T(64) - x;
      verify_equal(simd_select(M(true), x, y), x);
      verify_equal(simd_select(M(false), x, y), y);
      verify_equal(simd_select(M(true), y, x), y);
      verify_equal(simd_select(M(false), y, x), x);
      verify_equal(simd_select(M([](int i) { return 1 == (i & 1); }), x, T()),
                   V([](int i) { return (1 == (i & 1)) ? T(i & 63) : T(); }));

      x = make_value_unknown(x);
      y = make_value_unknown(y);
      verify_equal(simd_select(M(true), x, y), x);
      verify_equal(simd_select(M(false), x, y), y);
      verify_equal(simd_select(M(true), y, x), y);
      verify_equal(simd_select(M(false), y, x), x);
      verify_equal(simd_select(M([](int i) { return 1 == (i & 1); }), x, T()),
                   V([](int i) { return (1 == (i & 1)) ? T(i & 63) : T(); }));
    }
  };

auto tests = register_tests<Min, Max, Minmax, Clamp, Select>();
