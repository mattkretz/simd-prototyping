/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_SPLIT_H_
#define PROTOTYPE_SIMD_SPLIT_H_

#include "simd.h"
#include <array>
#include <tuple>

namespace std
{
  namespace __detail
  {
    template <typename _Tp, size_t>
      using __repeat_type = _Tp;
  }

  template <typename _V, typename _Tp, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr auto
    simd_split(const basic_simd<_Tp, _Abi>& __x) noexcept
    {
      constexpr int __in = simd_size_v<_Tp, _Abi>;
      constexpr int __out = _V::size();
      constexpr int __rem = __in % __out;
      if constexpr (__rem == 0)
        { // -> array
          using _Rp = std::array<_V, __in / __out>;
          if constexpr (sizeof(_Rp) == sizeof(__x))
            return __builtin_bit_cast(_Rp, __x);
          else
            return [&]<size_t... _Is>(std::index_sequence<_Is...>) {
              return _Rp {_V([&](auto __i) { return __x[__i + __out * _Is]; })...};
            }(std::make_index_sequence<__in / __out>());
        }
      else // -> tuple
        return [&]<size_t... _Is>(std::index_sequence<_Is...>) {
          using _Rem = std::resize_simd_t<__rem, _V>;
          using _Rp = std::tuple<__detail::__repeat_type<_V, _Is>..., _Rem>;
          return _Rp {_V([&](auto __i) {
                        return __x[__i + __out * _Is];
                      })..., _Rem([&](auto __i) {
                        return __x[__i + __in - __rem];
                      })};
        }(std::make_index_sequence<__in / __out>());
    }

  template <typename _M, size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr auto
    simd_split(const basic_simd_mask<_Bs, _Abi>& __x) noexcept
    {
      constexpr int __in = basic_simd_mask<_Bs, _Abi>::size();
      constexpr int __out = _M::size();
      constexpr int __rem = __in % __out;
      if constexpr (__rem == 0)
        { // -> array
          using _Rp = std::array<_M, __in / __out>;
          if constexpr (sizeof(_Rp) == sizeof(__x))
            return __builtin_bit_cast(_Rp, __x);
          else
            return [&]<size_t... _Is>(std::index_sequence<_Is...>) {
              return _Rp {_M([&](auto __i) { return __x[__i + __out * _Is]; })...};
            }(std::make_index_sequence<__in / __out>());
        }
      else // -> tuple
        return [&]<size_t... _Is>(std::index_sequence<_Is...>) {
          using _Rem = std::resize_simd_t<__rem, _M>;
          using _Rp = std::tuple<__detail::__repeat_type<_M, _Is>..., _Rem>;
          return _Rp {_M([&](auto __i) {
                        return __x[__i + __out * _Is];
                      })..., _Rem([&](auto __i) {
                        return __x[__i + __in - __rem];
                      })};
        }(std::make_index_sequence<__in / __out>());
    }
}

#endif  // PROTOTYPE_SIMD_SPLIT_H_
