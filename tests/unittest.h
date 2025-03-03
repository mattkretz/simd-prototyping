/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2023–2025 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef TESTS_UNITTEST_H_
#define TESTS_UNITTEST_H_

#include "unittest_pch.h"

#ifndef UNITTEST_TYPE
#define UNITTEST_TYPE int
#endif
#ifndef UNITTEST_WIDTH
#define UNITTEST_WIDTH 8
#endif

template <typename U>
void test_runner()
{
  using T = std::conditional_t<true, UNITTEST_TYPE, U>; // makes T dependent => constexpr-if works
  constexpr int Width = UNITTEST_WIDTH;
  if constexpr (std::destructible<std::simd<T>>)
    {
      constexpr int N = Width <= 64 ? Width // 1-64
                                    : 64 + (Width - 64) * std::simd<T>::size();
      if constexpr (std::destructible<std::simd<T, N>>)
        {
          static_assert(std::destructible<std::simd_mask<T, N>>);
          static_assert(std::destructible<typename std::simd<T, N>::mask_type>);
          static_assert(std::simd<T, N>::size() == N);
          static_assert(std::simd_mask<T, N>::size() == N);
          std::cout << "Testing " << type_to_string<std::simd<T, N>>()
                    << ':' << std::endl;
          run_functions.clear();
          [[maybe_unused]] Tests<std::simd<T, N>> t = {};
          for (auto f : run_functions)
            f();

          using std::__detail::__canonical_vec_type_t;
          static_assert(std::is_same_v<__canonical_vec_type_t<T>, T>);

          if constexpr (std::is_same_v<__canonical_vec_type_t<char>, T>)
            {
              std::cout << "Testing " << type_to_string<std::simd<char, N>>()
                        << ':' << std::endl;
              run_functions.clear();
              [[maybe_unused]] Tests<std::simd<char, N>> t0 = {};
              for (auto f : run_functions)
                f();
            }
          if constexpr (std::is_same_v<__canonical_vec_type_t<char8_t>, T>)
            {
              std::cout << "Testing " << type_to_string<std::simd<char8_t, N>>()
                        << ':' << std::endl;
              run_functions.clear();
              [[maybe_unused]] Tests<std::simd<char8_t, N>> t0 = {};
              for (auto f : run_functions)
                f();
            }
          if constexpr (std::is_same_v<__canonical_vec_type_t<char16_t>, T>)
            {
              std::cout << "Testing " << type_to_string<std::simd<char16_t, N>>()
                        << ':' << std::endl;
              run_functions.clear();
              [[maybe_unused]] Tests<std::simd<char16_t, N>> t0 = {};
              for (auto f : run_functions)
                f();
            }
          if constexpr (std::is_same_v<__canonical_vec_type_t<char32_t>, T>)
            {
              std::cout << "Testing " << type_to_string<std::simd<char32_t, N>>()
                        << ':' << std::endl;
              run_functions.clear();
              [[maybe_unused]] Tests<std::simd<char32_t, N>> t0 = {};
              for (auto f : run_functions)
                f();
            }
          if constexpr (std::is_same_v<__canonical_vec_type_t<wchar_t>, T>)
            {
              std::cout << "Testing " << type_to_string<std::simd<wchar_t, N>>()
                        << ':' << std::endl;
              run_functions.clear();
              [[maybe_unused]] Tests<std::simd<wchar_t, N>> t0 = {};
              for (auto f : run_functions)
                f();
            }
          if constexpr (std::is_same_v<__canonical_vec_type_t<_Float64>, T>)
            {
              std::cout << "Testing " << type_to_string<std::simd<_Float64, N>>()
                        << ':' << std::endl;
              run_functions.clear();
              [[maybe_unused]] Tests<std::simd<_Float64, N>> t0 = {};
              for (auto f : run_functions)
                f();
            }
          if constexpr (std::is_same_v<__canonical_vec_type_t<_Float32>, T>)
            {
              std::cout << "Testing " << type_to_string<std::simd<_Float32, N>>()
                        << ':' << std::endl;
              run_functions.clear();
              [[maybe_unused]] Tests<std::simd<_Float32, N>> t0 = {};
              for (auto f : run_functions)
                f();
            }
        }
      else
        {
          std::cout << "Test type not supported.\n";
          static_assert(not std::default_initializable<std::simd<T, N>>);
          static_assert(not std::copy_constructible<std::simd<T, N>>);
          static_assert(not std::is_copy_assignable_v<std::simd<T, N>>);
        }
    }
  else
    {
      std::cout << "Test type not supported.\n";
      static_assert(not std::default_initializable<std::simd<T>>);
      static_assert(not std::copy_constructible<std::simd<T>>);
      static_assert(not std::is_copy_assignable_v<std::simd<T>>);
    }
}

#endif  // TESTS_UNITTEST_H_
