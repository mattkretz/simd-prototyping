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
    using T = int;
#else
    using T = UNITTEST_TYPE;
#endif
    if constexpr (std::destructible<simd::simd<T>>)
      {
#ifndef UNITTEST_WIDTH
        constexpr int Width = 8;
#else
        constexpr int Width = UNITTEST_WIDTH;
#endif
        constexpr int N = Width <= 64 ? Width // 1-64
                                      : 64 + (Width - 64) * simd::simd<T>::size();
        if constexpr (std::destructible<simd::simd<T, N>>)
          {
            static_assert(std::destructible<simd::mask<T, N>>);
            static_assert(std::destructible<typename simd::simd<T, N>::mask_type>);
            static_assert(simd::simd<T, N>::size() == N);
            static_assert(simd::mask<T, N>::size() == N);
            run_functions.push_back(Test<simd::simd<T, N>>::run);
          }
        else
          {
            std::cout << "Test type not supported.\n";
            static_assert(simd::simd<T, N>::size() == 0);
            static_assert(simd::mask<T, N>::size() == 0);
          }
      }
    else
      {
        std::cout << "Test type not supported.\n";
        static_assert(not std::default_initializable<simd::simd<T>>);
        static_assert(not std::copy_constructible<simd::simd<T>>);
        static_assert(not std::is_copy_assignable_v<simd::simd<T>>);
        static_assert(simd::simd<T>::size() == 0);
        static_assert(simd::mask<T>::size() == 0);
      }
  }

#endif  // TESTS_UNITTEST_H_
