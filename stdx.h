/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_STDX_H_
#define PROTOTYPE_STDX_H_

#include "detail.h"

#include <experimental/simd>
#include <concepts>

namespace stdx
{
  using namespace std::experimental;
  using namespace std::experimental::__proposed;

  template <class T>
    concept arithmetic = std::integral<T> || std::floating_point<T>;

  template <class T>
    concept vectorizable = arithmetic<T> && not std::same_as<T, bool>;

  template <class A>
    concept simd_abi_tag = std::experimental::is_abi_tag_v<A>;

  template <class V, std::size_t Width = 0>
    concept simd_type = std::experimental::is_simd_v<V>
                          && vectorizable<typename V::value_type>
                          && simd_abi_tag<typename V::abi_type>
                          && (Width == 0 || V::size() == Width);

  template <typename From, typename To>
    concept value_preserving_convertible_to
      = std::convertible_to<From, To>
          && (not arithmetic<From> || not arithmetic<To>
                || (not (std::is_signed_v<From> && std::is_unsigned_v<To>)
                      && std::numeric_limits<From>::digits <= std::numeric_limits<To>::digits
                      && std::numeric_limits<From>::max() <= std::numeric_limits<To>::max()
                      && std::numeric_limits<From>::lowest() >= std::numeric_limits<To>::lowest());

  template <typename From, typename To>
    concept value_preserving_or_int
      = value_preserving_convertible_to<std::remove_cvref_t<From>, To>
          || (arithmetic<To> && std::same_as<std::remove_cvref_t<From>, int>)
          || (std::unsigned_integral<To> && std::same_as<std::remove_cvref_t<From>, unsigned>);

  // higher_floating_point_rank_than<T, U> (T has higher or equal floating point rank than U)
  template <typename From, typename To>
    concept higher_floating_point_rank_than
      = std::floating_point<From> && std::floating_point<To>
          && std::same_as<std::common_type_t<From, To>, From>;

  // higher_integer_rank_than<T, U> (T has higher or equal integer rank than U)
  template <typename From, typename To>
    concept higher_integer_rank_than
      = std::integral<From> && std::integral<To>
          && (sizeof(From) > sizeof(To) || std::same_as<std::common_type_t<From, To>, From>);

  template <typename From, typename To>
    concept higher_rank_than
      = higher_floating_point_rank_than<From, To> || higher_integer_rank_than<From, To>;

  template <typename F, typename T, std::size_t... Is>
    constexpr
    detail::cnst<(value_preserving_or_int<std::invoke_result_t<F, detail::cnst_t<Is>, T>> && ...)>
    simd_broadcast_invokable_impl(std::index_sequence<Is...>);

  template <typename F, typename T, std::size_t N>
    concept simd_broadcast_invokable = requires
    {
      { simd_broadcast_invokable_impl<F, T>(std::make_index_sequence<N>()) }
        -> std::same_as<detail::cnst<true>>;
    };
}

#endif  // PROTOTYPE_STDX_H_
