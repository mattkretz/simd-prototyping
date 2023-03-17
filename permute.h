/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

// Implements non-members of P2664

#ifndef PROTOTYPE_PERMUTE_H_
#define PROTOTYPE_PERMUTE_H_

#include "simd.h"
#include "iota.h"

namespace std
{
  constexpr int simd_permute_zero = INT_MIN;

  namespace simd_permutations
  {
    struct _DuplicateEven
    {
      consteval unsigned
      operator()(unsigned __i) const
      { return __i & ~1u; }
    };

    inline constexpr _DuplicateEven duplicate_even {};

    struct _DuplicateOdd
    {
      consteval unsigned
      operator()(unsigned __i) const
      { return __i | 1u; }
    };

    inline constexpr _DuplicateOdd duplicate_odd {};

    struct _SwapNeighbors
    {
      consteval unsigned
      operator()(unsigned __i) const
      { return __i ^ 1u; }
    };

    inline constexpr _SwapNeighbors swap_neighbors {};

    template <unsigned _Position>
      struct _Broadcast
      {
        consteval unsigned
        operator()(unsigned, auto __size) const
        {
          static_assert(_Position < __size);
          return _Position;
        }
      };

    template <unsigned _Position = 0u>
      inline constexpr _Broadcast<_Position> broadcast {};

    inline constexpr _Broadcast<0u> broadcast_first {};

    struct _BroadcastLast
    {
      consteval unsigned
      operator()(unsigned, auto __size) const
      { return __size() - 1; }
    };

    inline constexpr _BroadcastLast broadcast_last {};

    struct _Reverse
    {
      consteval unsigned
      operator()(unsigned __i, auto __size) const
      { return __size() - 1 - __i; }
    };

    inline constexpr _Reverse reverse {};
  }

  template <std::size_t _Np = 0, __detail::__simd_or_mask _Vp,
            __detail::__index_permutation_function<_Vp> _Fp>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr std::resize_simd_t<_Np == 0 ? _Vp::size() : _Np, _Vp>
    permute(_Vp const& __v, _Fp const __idx_perm) noexcept
    {
      using _Tp = typename _Vp::value_type;
      using _Rp = resize_simd_t<_Np == 0 ? _Vp::size() : _Np, _Vp>;
      return _Rp([&](auto __i) -> _Tp {
               constexpr int __j = [&] {
                 if constexpr (__detail::__index_permutation_function_nosize<_Fp>)
                   return __idx_perm(__i);
                 else
                   return __idx_perm(__i, _Vp::size);
               }();
               if constexpr (__j == simd_permute_zero)
                 return 0;
               else if constexpr (__j < 0)
                 return __v[__v.size() + __j];
               else
                 return __v[__j];
             });
    }

  static_assert(all_of(permute(iota_v<simd<int>>, simd_permutations::duplicate_even)
                         == iota_v<simd<int>> / 2 * 2));

  static_assert(all_of(permute(iota_v<simd<int>>, simd_permutations::duplicate_odd)
                         == iota_v<simd<int>> / 2 * 2 + 1));

  static_assert(all_of(permute(iota_v<simd<int>>, simd_permutations::swap_neighbors)
                         == simd<int>([](int i) { return i ^ 1; })));

  static_assert(all_of(permute(iota_v<simd<int>>, simd_permutations::broadcast<1>)
                         == simd<int>(1)));

  static_assert(all_of(permute(iota_v<simd<int>>, simd_permutations::broadcast_first)
                         == simd<int>(0)));

  static_assert(all_of(permute(iota_v<simd<int>>, simd_permutations::broadcast_last)
                         == simd<int>(int(simd_size_v<int> - 1))));

  static_assert(all_of(permute(iota_v<simd<int>>, simd_permutations::reverse)
                         == simd<int>([](int i) { return int(simd_size_v<int>) - 1 - i; })));
}

#endif  // PROTOTYPE_PERMUTE_H_
