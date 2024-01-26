/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include "../unittest.h"

#include "../interleave.h"
#include "../permute.h"
#include "../simd_split.h"
#include "../mask_reductions.h"

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

      V zero = T();
      auto x = std::iota_v<V>;
      verify_equal(x, x);
      if constexpr (std::same_as<T, std::byte>)
        verify_not_equal(x, x | V(T(1)));
      else
        {
          verify_not_equal(x, x + V(T(1)));
          auto y = x;
          verify_equal(y++, x);
          verify_not_equal(y, x);
          verify_not_equal(y--, x);
          verify_equal(y, x);
          verify_not_equal(++y, x);
          verify_not_equal(y, x);
          verify_equal(--y, x);
        }

      verify_equal(std::reduce_min_index(x == x), 0);
      verify_equal(std::reduce_max_index(x == x), V::size - 1);

      const M k0 = x == zero;

      for (int i = 0; i < V::size; ++i)
        {
          const M k = x == T(i);
          verify_equal(k[i], true);
          verify_equal(std::reduce_min_index(k), i);
          verify_equal(std::reduce_max_index(k), i);

          verify_equal(std::reduce_min_index(k || k0), 0);
          verify_equal(std::reduce_max_index(k || k0), i);
        }
    }
  };

int main()
{
  instantiate_tests<misc>();
  std::cout << "Passed tests: " << passed_tests << "\nFailed tests: " << failed_tests << '\n';
  return failed_tests;
}
