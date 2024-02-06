/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef TESTS_UNITTEST_H_
#define TESTS_UNITTEST_H_

#include "unittest_pch.h"

template <template<typename> class Test>
  void
  instantiate_tests_for_value_type()
  {
#ifndef UNITTEST_TYPE
#define UNITTEST_TYPE int
#endif
    using T = UNITTEST_TYPE;
    if constexpr (std::destructible<std::simd<T>>)
      {
#ifndef UNITTEST_WIDTH
        constexpr int Width = 8;
#else
        constexpr int Width = UNITTEST_WIDTH;
#endif
        constexpr int N = Width <= 64 ? Width // 1-64
                                       : 64 + (Width - 64) * std::simd<T>::size();
        if constexpr (std::destructible<std::simd<T, N>>)
          {
            static_assert(std::destructible<std::simd_mask<T, N>>);
            static_assert(std::destructible<typename std::simd<T, N>::mask_type>);
            static_assert(std::simd<T, N>::size() == N);
            static_assert(std::simd_mask<T, N>::size() == N);
            run_functions.push_back(Test<std::simd<T, N>>::run);
          }
        else
          {
            std::cout << "Test type not supported.\n";
            static_assert(std::simd<T, N>::size() == 0);
            static_assert(std::simd_mask<T, N>::size() == 0);
          }
      }
  }

#endif  // TESTS_UNITTEST_H_
