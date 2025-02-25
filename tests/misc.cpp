/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023–2025 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#include "unittest.h"

#include "../simd"

#include <numeric>

using namespace vir::literals;

template <typename V>
  struct misc
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;

    static void
    run()
    {
      log_start();

      auto x = vec<V, 0, 100, 2, 54, 3>;
      verify_equal(x, x);
      if constexpr (std::same_as<T, std::byte>)
        verify_not_equal(x, x | V(T(1)));
      else
        {
          auto x1 = x + V(T(1));
          verify_not_equal(x, x1);
          auto y = x;
          verify_equal(y++, x);
          verify_not_equal(y, x);
          verify_not_equal(y--, x);
          verify_equal(y, x);
          verify_not_equal(++y, x);
          verify_not_equal(y, x);
          verify_equal(--y, x);
        }
#ifdef __SSE__
      if constexpr (sizeof(x) == 16 and std::is_same_v<T, float>)
        verify_equal(_mm_and_ps(x, x), x);
#endif
#ifdef __SSE2__
      if constexpr (sizeof(x) == 16 and std::is_integral_v<T>)
        verify_equal(_mm_and_si128(x, x), x);
      if constexpr (sizeof(x) == 16 and std::is_same_v<T, double>)
        verify_equal(_mm_and_pd(x, x), x);
#endif
    }
  };

template <typename V>
  struct mask_reductions
  {
    using T = typename V::value_type;
    using M = typename V::mask_type;

    static void
    run()
    {
      log_start();

      constexpr V x = test_iota<V>;

      verify_equal(std::reduce_min_index(x == x), 0);
      verify_equal(std::reduce_max_index(x == x), V::size - 1);

      auto test = [](auto ii) {
        log_start();
        constexpr int i = ii;

        // Caveat:
        // k0[n0 * (test_iota_max<V> + 1)] is true if it exists
        // k[n * (test_iota_max<V> + 1) + i] is true if it exists
        // none_of(k) is true if i > test_iota_max<V>
        // => by test construction:
        static_assert(i <= test_iota_max<V>);
        // also by construction of test_iota_max:
        static_assert(test_iota_max<V> < V::size());

        constexpr int nk = 1 + (V::size() - i - 1) / (test_iota_max<V> + 1);
        constexpr int maxk = (nk - 1) * (test_iota_max<V> + 1) + i;
        static_assert(maxk < V::size());

        constexpr int nk0 = 1 + (V::size() - 1) / (test_iota_max<V> + 1);
        constexpr int maxk0 = (nk0 - 1) * (test_iota_max<V> + 1);
        static_assert(maxk0 < V::size());

        constexpr int maxkork0 = std::max(maxk, maxk0);

        {
          constexpr M k = test_iota<V> == T(i);
          constexpr M k0 = test_iota<V> == T(0);

          // constexpr
          static_assert(k[i] == true);
          static_assert(std::reduce_min_index(k) == i);
          static_assert(std::reduce_max_index(k) == maxk);
          static_assert(std::reduce_min_index(k || k0) == 0);
          static_assert(std::reduce_max_index(k || k0) == maxkork0);
          static_assert(all_of(k == k));
          static_assert(none_of((not k) == k));
          static_assert(all_of((k | k) == k));
          static_assert(all_of((k & k) == k));
          static_assert(none_of(k ^ k));
          static_assert(std::reduce_count(k) == nk);
          static_assert(std::reduce_count(not k) == V::size - nk);
          static_assert(any_of(k));
          static_assert(any_of(k & k0) ^ (i != 0));

          // constprop
          verify_equal(k[i], true);
          verify_equal(std::reduce_min_index(k), i)(k);
          verify_equal(std::reduce_max_index(k), maxk)(k);
          verify_equal(std::reduce_min_index(k || k0), 0);
          verify_equal(std::reduce_max_index(k || k0), maxkork0);
          verify_equal(k, k);
          verify_not_equal(not k, k);
          verify_equal(k | k, k);
          verify_equal(k & k, k);
          verify(none_of(k ^ k));
          verify_equal(std::reduce_count(k), nk);
          verify_equal(std::reduce_count(not k), V::size - nk);
          verify(any_of(k));
          verify(bool(any_of(k & k0) ^ (i != 0)));
        }

        { // runtime
          M k = make_value_unknown(test_iota<V> == T(i));
          M k0 = make_value_unknown(test_iota<V> == T(0));

          verify_equal(k[i], true);
          verify_equal(std::as_const(k)[i], true);
          verify_equal(std::reduce_min_index(k), i)(k);
          verify_equal(std::reduce_max_index(k), maxk)(k);
          verify_equal(std::reduce_min_index(k || k0), 0);
          verify_equal(std::reduce_max_index(k || k0), maxkork0);
          verify_equal(k, k);
          verify_not_equal(not k, k);
          verify_equal(k | k, k);
          verify_equal(k & k, k);
          verify(none_of(k ^ k));
          verify_equal(std::reduce_count(k), nk);
          verify_equal(-std::reduce(-k), nk)(k, -k);
          verify_equal(std::reduce_count(not k), V::size - nk)(not k);
          if constexpr (V::size <= 128)
            verify_equal(-std::reduce(-not k), V::size - nk)(-not k);
          verify(any_of(k));
          verify(bool(any_of(k & k0) ^ (i != 0)));

          k = M([&](int i) {
                if (i == 0)
                  return make_value_unknown(true);
                else
                  return k[i];
              });
          verify_equal(k[i], true);
          verify_equal(std::as_const(k)[i], true);
          verify_equal(k[0], true);
          verify_equal(std::as_const(k)[0], true);
          verify_equal(std::reduce_min_index(k), 0)(k);
          verify_equal(std::reduce_max_index(k), maxk)(k);
        }
      };
      _GLIBCXX_SIMD_INT_PACK(test_iota_max<V> + 1, is, { (test(vir::cw<is>), ...); });
    }
  };

template <typename V>
  struct loads
  {
    using T = typename V::value_type;

    static void
    run()
    {
      log_start();

      static_assert(std::simd_alignment_v<V> <= 256);
      alignas(256) std::array<T, V::size * 2> mem = {};

      auto it = mem.begin();
      auto end = mem.end();

      auto x = std::simd_unchecked_load<V>(mem);
      verify_equal(x, V());
      verify_equal(std::simd_partial_load<V>(mem), V());

      auto x2 = std::simd_unchecked_load<V>(mem, std::simd_flag_aligned);
      verify_equal(x2, V());
      verify_equal(std::simd_partial_load<V>(mem, std::simd_flag_aligned), V());

      auto x3 = std::simd_unchecked_load<V>(mem, std::simd_flag_overaligned<256>);
      verify_equal(x3, V());
      verify_equal(std::simd_partial_load<V>(mem, std::simd_flag_overaligned<256>), V());

      auto x4 = std::simd_unchecked_load<V>(it + 1, end);
      verify_equal(x4, V());
      verify_equal(std::simd_partial_load<V>(it + 1, end), V());
      verify_equal(std::simd_partial_load<V>(it + 1, it + 1), V());
      verify_equal(std::simd_partial_load<V>(it + 1, it + 2), V());

      std::array<int, V::size * 2> ints = {};
      auto x5 = std::simd_unchecked_load<V>(ints, std::simd_flag_convert);
      verify_equal(x5, V());
      verify_equal(std::simd_partial_load<V>(ints, std::simd_flag_convert), V());

      if constexpr (requires {T() + T(1);})
        {
          std::iota(ints.begin(), ints.end(), T(1));
          std::iota(it, end, T(1));
          constexpr V ref = test_iota<V, 1, 0>;
          constexpr V ref1 = V([](int i) { return i == 0 ? T(1): T(); });

          verify_equal(std::simd_unchecked_load<V>(mem), ref);
          verify_equal(std::simd_partial_load<V>(mem), ref);

          verify_equal(std::simd_unchecked_load<V>(it + 1, end), ref + T(1));
          verify_equal(std::simd_partial_load<V>(it + 1, end), ref + T(1));
          verify_equal(std::simd_partial_load<V>(it, it + 1), ref1);

          verify_equal(std::simd_unchecked_load<V>(mem, std::simd_flag_aligned), ref);
          verify_equal(std::simd_partial_load<V>(mem, std::simd_flag_aligned), ref);

          verify_equal(std::simd_unchecked_load<V>(ints, std::simd_flag_convert), ref);
          verify_equal(std::simd_partial_load<V>(ints, std::simd_flag_convert), ref);
          verify_equal(std::simd_partial_load<V>(
                         ints.begin(), ints.begin(), std::simd_flag_convert), V());
          verify_equal(std::simd_partial_load<V>(
                         ints.begin(), ints.begin() + 1, std::simd_flag_convert), ref1);
        }
    }
  };

template <typename V>
  struct stores
  {
    using T = typename V::value_type;

    static void
    run() requires requires {T() + T(1);}
    {
      log_start();

      static_assert(std::simd_alignment_v<V> <= 256);
      alignas(256) std::array<T, V::size * 2> mem = {};
      alignas(256) std::array<int, V::size * 2> ints = {};

      const V v = test_iota<V, 1, 0>;
      std::simd_unchecked_store(v, mem, std::simd_flag_aligned);
      std::simd_unchecked_store(v, mem.begin() + V::size(), mem.end());
      std::simd_unchecked_store(v, ints, std::simd_flag_convert);
      std::simd_partial_store(v, ints.begin() + V::size() + 1, ints.end(),
                              std::simd_flag_convert | std::simd_flag_overaligned<alignof(T)>);
      for (int i = 0; i < V::size; ++i)
        {
          verify_equal(mem[i], T(i + 1));
          verify_equal(ints[i], int(T(i + 1)));
        }
      for (int i = 0; i < V::size; ++i)
        {
          verify_equal(mem[V::size + i], T(i + 1));
          verify_equal(ints[V::size + i], int(T(i)));
        }

      std::simd_unchecked_store(V(), ints.begin(), V::size(), std::simd_flag_convert);
      std::simd_unchecked_store(V(), ints.begin() + V::size(), V::size(), std::simd_flag_convert);
      for (int i = 0; i < 2 * V::size; ++i)
        verify_equal(ints[i], 0)("i =", i);

      if constexpr (V::size() > 1)
        {
          std::simd_partial_store(v, ints.begin() + 1, V::size() - 2, std::simd_flag_convert);
          for (int i = 0; i < V::size - 2; ++i)
            verify_equal(ints[i], int(T(i)));
          verify_equal(ints[V::size - 1], 0);
          verify_equal(ints[V::size], 0);
        }
      else
        {
          std::simd_partial_store(v, ints.begin() + 1, 0, std::simd_flag_convert);
          verify_equal(ints[0], 0);
          verify_equal(ints[1], 0);
        }
    }
  };

auto tests = register_tests<misc, mask_reductions, loads, stores>();
