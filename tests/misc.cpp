/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#include "unittest.h"

#include "../interleave.h"
#include "../permute.h"
#include "../simd_split.h"
#include "../mask_reductions.h"

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

      auto x = std::iota_v<V>;
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

      constexpr V x = std::iota_v<V>;

      verify_equal(std::reduce_min_index(x == x), 0);
      verify_equal(std::reduce_max_index(x == x), V::size - 1);

      auto test = [](auto ii) {
        log_start();
        constexpr int i = ii;

        {
          constexpr M k = std::iota_v<V> == T(i);
          constexpr M k0 = std::iota_v<V> == T(0);

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
          M k = make_value_unknown(std::iota_v<V> == T(i));
          M k0 = make_value_unknown(std::iota_v<V> == T(0));

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

          k[0] = make_value_unknown(true);
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

      V x(it);
      verify_equal(x, V());

      if constexpr (requires {T() + T(1);})
        {
          std::iota(mem.begin(), mem.end(), T());
          x.copy_from(it);
          verify_equal(x, std::iota_v<V>);

          x.copy_from(it + 1);
          verify_equal(x, std::iota_v<V> + T(1));
        }
    }
  };

auto tests = register_tests<misc, mask_reductions, loads>();
