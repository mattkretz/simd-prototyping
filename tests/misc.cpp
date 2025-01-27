/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
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

      auto x = std::simd_iota<V>;
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

      constexpr V x = std::simd_iota<V>;

      verify_equal(std::reduce_min_index(x == x), 0);
      verify_equal(std::reduce_max_index(x == x), V::size - 1);

      auto test = [](auto ii) {
        log_start();
        constexpr int i = ii;

        {
          constexpr M k = std::simd_iota<V> == T(i);
          constexpr M k0 = std::simd_iota<V> == T(0);

          // constexpr
          static_assert(k[i] == true);
          static_assert(std::reduce_min_index(k) == i);
          static_assert(std::reduce_max_index(k) == i);
          static_assert(std::reduce_min_index(k || k0) == 0);
          static_assert(std::reduce_max_index(k || k0) == i);
          static_assert(all_of(k == k));
          static_assert(none_of((not k) == k));
          static_assert(all_of((k | k) == k));
          static_assert(all_of((k & k) == k));
          static_assert(none_of(k ^ k));
          static_assert(std::reduce_count(k) == 1);
          static_assert(std::reduce_count(not k) == V::size - 1);
          static_assert(any_of(k));
          static_assert(any_of(k & k0) ^ (i != 0));

          // constprop
          verify_equal(k[i], true);
          verify_equal(std::reduce_min_index(k), i)(k);
          verify_equal(std::reduce_max_index(k), i)(k);
          verify_equal(std::reduce_min_index(k || k0), 0);
          verify_equal(std::reduce_max_index(k || k0), i);
          verify_equal(k, k);
          verify_not_equal(not k, k);
          verify_equal(k | k, k);
          verify_equal(k & k, k);
          verify(none_of(k ^ k));
          verify_equal(std::reduce_count(k), 1);
          verify_equal(std::reduce_count(not k), V::size - 1);
          verify(any_of(k));
          verify(bool(any_of(k & k0) ^ (i != 0)));
        }

        { // runtime
          M k = make_value_unknown(std::simd_iota<V> == T(i));
          M k0 = make_value_unknown(std::simd_iota<V> == T(0));

          verify_equal(k[i], true);
          verify_equal(std::as_const(k)[i], true);
          verify_equal(std::reduce_min_index(k), i)(k);
          verify_equal(std::reduce_max_index(k), i)(k);
          verify_equal(std::reduce_min_index(k || k0), 0);
          verify_equal(std::reduce_max_index(k || k0), i);
          verify_equal(k, k);
          verify_not_equal(not k, k);
          verify_equal(k | k, k);
          verify_equal(k & k, k);
          verify(none_of(k ^ k));
          verify_equal(std::reduce_count(k), 1);
          verify_equal(-std::reduce(-k), 1)(k, -k);
          verify_equal(std::reduce_count(not k), V::size - 1)(not k);
          if constexpr (V::size <= 128)
            verify_equal(-std::reduce(-not k), V::size - 1)(-not k);
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
          verify_equal(std::reduce_max_index(k), i)(k);
        }
      };
      _GLIBCXX_SIMD_INT_PACK(V::size, is, { (test(vir::cw<is>), ...); });
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
          constexpr V ref = std::simd_iota<V> + vir::cw<1>;
          constexpr V ref1 = std::simd_select(ref == T(1), T(1), T());

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

auto tests = register_tests<misc, mask_reductions, loads>();
