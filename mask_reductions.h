/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_MASK_REDUCTIONS_H_
#define PROTOTYPE_MASK_REDUCTIONS_H_

#include "simd_mask.h"
#include "simd_reductions.h"
#include "x86_detail.h"

namespace std
{
  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    all_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      using _Kp = basic_simd_mask<_Bs, _Abi>;
      constexpr __detail::_SimdSizeType __size = _Kp::size.value;
      if constexpr (__size == 1)
        return __data(__k);

      else if (__builtin_is_constant_evaluated() or __k._M_is_constprop())
        {
          for (int __i = 0; __i < __size; ++__i)
            if (!__k[__i])
              return false;
          return true;
        }

      else if constexpr (requires {_Abi::_MaskImpl::_S_all_of(__k);})
        return _Abi::_MaskImpl::_S_all_of(__k);

      else if constexpr (not _Kp::_S_is_bitmask and sizeof(__k) <= sizeof(int64_t))
        {
          const auto __as_int
            = __builtin_bit_cast(__detail::__mask_integer_from<sizeof(__k)>, __data(__k));
          if constexpr (__has_single_bit(__size))
            return __as_int == -1;
          else
            return (__as_int & ((1ll << (__size * _Bs * __CHAR_BIT__)) - 1))
                     == (1ll << (__size * _Bs * __CHAR_BIT__)) - 1;
        }

#if _GLIBCXX_SIMD_HAVE_NEON
      else if constexpr (__pv2::__is_neon_abi<_Abi>())
        {
          using _Tp = __detail::__mask_integer_from<_Bs>;
          static_assert(sizeof(__k) == 16);
          const auto __kk
            = __pv2::__vector_bitcast<char>(__data(__k))
                | ~__pv2::__vector_bitcast<char>(_Abi::template _S_implicit_mask<_Tp>());
          const auto __x = __pv2::__vector_bitcast<int64_t>(__kk);
          return __x[0] + __x[1] == -2;
        }

#endif
      else
        {
          return [&]<__detail::_SimdSizeType... _Is> [[__gnu__::__always_inline__]]
                 (__detail::_SimdIndexSequence<_Is...>) {
                   return (... and (__k[_Is] != 0));
                 }(__detail::_MakeSimdIndexSequence<__size>());
        }
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    any_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      using _Kp = basic_simd_mask<_Bs, _Abi>;
      constexpr __detail::_SimdSizeType __size = _Kp::size.value;
      if constexpr (__size == 1)
        return __data(__k);

      else if (__builtin_is_constant_evaluated() or __k._M_is_constprop())
        {
          for (int __i = 0; __i < __size; ++__i)
            if (__k[__i])
              return true;
          return false;
        }

      else if constexpr (requires {_Abi::_MaskImpl::_S_any_of(__k);})
        return _Abi::_MaskImpl::_S_any_of(__k);

      else if constexpr (not _Kp::_S_is_bitmask and sizeof(__k) <= sizeof(int64_t))
        return __builtin_bit_cast(__detail::__mask_integer_from<sizeof(__k)>, __data(__k)) != 0;

#if _GLIBCXX_SIMD_HAVE_NEON
      else if constexpr (__pv2::__is_neon_abi<_Abi>())
        {
          static_assert(sizeof(__k) == 16);
          const auto __kk = _Abi::_S_masked(__data(__k));
          const auto __x = __pv2::__vector_bitcast<int64_t>(__kk);
          return (__x[0] | __x[1]) != 0;
        }

#endif
      else
        {
          return [&]<__detail::_SimdSizeType... _Is> [[__gnu__::__always_inline__]]
                 (__detail::_SimdIndexSequence<_Is...>) {
                   return (... or (__k[_Is] != 0));
                 }(__detail::_MakeSimdIndexSequence<__size>());
        }
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    none_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      using _Kp = basic_simd_mask<_Bs, _Abi>;
      constexpr __detail::_SimdSizeType __size = _Kp::size.value;
      if constexpr (__size == 1)
        return !__data(__k);

      else if (__builtin_is_constant_evaluated() or __k._M_is_constprop())
        {
          for (int __i = 0; __i < __k.size(); ++__i)
            if (__k[__i])
              return false;
          return true;
        }

      else if constexpr (requires {_Abi::_MaskImpl::_S_none_of(__k);})
        return _Abi::_MaskImpl::_S_none_of(__k);

      else if constexpr (not _Kp::_S_is_bitmask and sizeof(__k) <= sizeof(int64_t))
        return __builtin_bit_cast(__detail::__mask_integer_from<sizeof(__k)>,
                                  _Abi::_S_masked(__data(__k))) == 0;

#if _GLIBCXX_SIMD_HAVE_NEON
      else if constexpr (__pv2::__is_neon_abi<_Abi>())
        {
          static_assert(sizeof(__k) == 16);
          const auto __kk = _Abi::_S_masked(__data(__k));
          const auto __x = __vector_bitcast<int64_t>(__kk);
          return (__x[0] | __x[1]) == 0;
        }

#endif
      else
        {
          return [&]<__detail::_SimdSizeType... _Is> [[__gnu__::__always_inline__]]
                 (__detail::_SimdIndexSequence<_Is...>) {
                   return (... and (__k[_Is] == 0));
                 }(__detail::_MakeSimdIndexSequence<__size>());
        }
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_count(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      using _Kp = basic_simd_mask<_Bs, _Abi>;
      constexpr __detail::_SimdSizeType __size = _Kp::size.value;
      if constexpr (__size == 1)
        return +__data(__k);

      else if (__builtin_is_constant_evaluated() || __k._M_is_constprop())
        {
          int __r = [&]<__detail::_SimdSizeType... _Is>(__detail::_SimdIndexSequence<_Is...>) {
            return (int(__k[_Is] != 0) + ...);
          }(__detail::_MakeSimdIndexSequence<__size>());
          if (__builtin_is_constant_evaluated() || __builtin_constant_p(__r))
            return __r;
        }

      else if constexpr (requires {_Abi::_MaskImpl::_S_popcount(__k);})
        return _Abi::_MaskImpl::_S_popcount(__k);

      return -reduce(-__k);
    }

  /**
   * Precondition: any_of(__k) is true.
   * on failure: ill-formed (constant expression) or returns unspecified value
   */
  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_min_index(const basic_simd_mask<_Bs, _Abi>& __k)
    {
      if (__builtin_is_constant_evaluated() and not any_of(__k))
        __detail::__invoke_ub("reduce_max_index(x): precondition any_of(x) failed");

      constexpr int __size = basic_simd_mask<_Bs, _Abi>::size.value;
      if constexpr (__size == 1)
        return 0;

      else if (__builtin_is_constant_evaluated() or __k._M_is_constprop())
        {
          const int __r = [&] {
            for (int __i = 0; __i < __size; ++__i)
              if (__k[__i])
                return __i;
            return -1;
          }();
          if (__builtin_is_constant_evaluated() or __builtin_constant_p(__r))
            return __r;
        }

      if constexpr (__size == 2)
        return __k[0] ? 0 : 1;

      else if constexpr (requires {_Abi::_MaskImpl::_S_find_first_set(__k);})
        return _Abi::_MaskImpl::_S_find_first_set(__k);

      else
        return __detail::__lowest_bit(_Abi::_MaskImpl::_S_to_bits(__data(__k))._M_to_bits());
    }

  /**
   * Precondition: any_of(__k) is true.
   * on failure: ill-formed (constant expression) or returns unspecified value
   */
  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_max_index(const basic_simd_mask<_Bs, _Abi>& __k)
    {
      if (__builtin_is_constant_evaluated() and not any_of(__k))
        __detail::__invoke_ub("reduce_max_index(x): precondition any_of(x) failed");

      constexpr int __size = basic_simd_mask<_Bs, _Abi>::size.value;
      if constexpr (__size == 1)
        return 0;

      else if (__builtin_is_constant_evaluated() or __k._M_is_constprop())
        {
          const int __r = [&] {
            for (int __i = __size - 1; __i >= 0; --__i)
              if (__k[__i])
                return __i;
            return -1;
          }();
          if (__builtin_is_constant_evaluated() or __builtin_constant_p(__r))
            return __r;
        }

      if constexpr (__size == 2)
        return __k[1] ? 1 : 0;

      else if constexpr (requires {_Abi::_MaskImpl::_S_find_last_set(__k);})
        return _Abi::_MaskImpl::_S_find_last_set(__k);

      else
        return __detail::__highest_bit(
                 _Abi::_MaskImpl::_S_to_bits(__data(__k))._M_sanitized()._M_to_bits());
    }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
  all_of(same_as<bool> auto __x) noexcept
  { return __x; }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
  any_of(same_as<bool> auto __x) noexcept
  { return __x; }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
  none_of(same_as<bool> auto __x) noexcept
  { return not __x; }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
  reduce_count(same_as<bool> auto __x) noexcept
  { return static_cast<__detail::_SimdSizeType>(__x); }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
  reduce_min_index(same_as<bool> auto __x) noexcept
  {
    if (__builtin_is_constant_evaluated() and !__x)
      __detail::__invoke_ub("reduce_max_index(x): precondition any_of(x) failed");
    return 0;
  }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
  reduce_max_index(same_as<bool> auto __x) noexcept
  {
    if (__builtin_is_constant_evaluated() and !__x)
      __detail::__invoke_ub("reduce_max_index(x): precondition any_of(x) failed");
    return 0;
  }
}
#endif  // PROTOTYPE_MASK_REDUCTIONS_H_
