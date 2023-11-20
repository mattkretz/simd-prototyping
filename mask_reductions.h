/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
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
      using _Tp = __pv2::__int_with_sizeof_t<_Bs>;
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

      else if constexpr (not _Kp::_S_is_bitmask and __size * _Bs <= sizeof(int64_t))
        {
          const auto __as_int
            = __builtin_bit_cast(__pv2::__int_with_sizeof_t<sizeof(__k)>, __data(__k));
          if constexpr (__has_single_bit(__size))
            return __as_int == -1;
          else
            return __as_int & ((1ll << (__size * _Bs * __CHAR_BIT__)) - 1)
                     == (1ll << (__size * _Bs * __CHAR_BIT__)) - 1;
        }

#if _GLIBCXX_SIMD_HAVE_SSE
      else if constexpr (__pv2::__is_avx512_abi<_Abi>())
        {
          constexpr auto _Mask = _Abi::template _S_implicit_mask<_Tp>();
          const auto __kk = __data(__k)._M_data;
          if constexpr (sizeof(__kk) == 1)
            {
              if constexpr (__pv2::__have_avx512dq)
                return __builtin_ia32_kortestcqi(__kk, _Mask == 0xff ? __kk : uint8_t(~_Mask));
              else
                return __builtin_ia32_kortestchi(__kk, uint16_t(~_Mask));
            }
          else if constexpr (sizeof(__kk) == 2)
            return __builtin_ia32_kortestchi(__kk, _Mask == 0xffff ? __kk : uint16_t(~_Mask));
          else if constexpr (sizeof(__kk) == 4 && __pv2::__have_avx512bw)
            return __builtin_ia32_kortestcsi(__kk, _Mask == 0xffffffffU ? __kk : uint32_t(~_Mask));
          else if constexpr (sizeof(__kk) == 8 && __pv2::__have_avx512bw)
            return __builtin_ia32_kortestcdi(
                     __kk, _Mask == 0xffffffffffffffffULL ? __kk : uint64_t(~_Mask));
          else
            __pv2::__assert_unreachable<_Tp>();
        }

      else if constexpr (__pv2::__is_sse_abi<_Abi>() or __pv2::__is_avx_abi<_Abi>())
        {
          static_assert(sizeof(__k) >= 16);
          if constexpr (__pv2::__have_sse4_1)
            {
              using _TI = __pv2::__intrinsic_type_t<_Tp, __size>;
              const _TI __a = reinterpret_cast<_TI>(__pv2::__to_intrin(__data(__k)));
              constexpr _TI __b = _Abi::template _S_implicit_mask_intrin<_Tp>();
              return 0 != __pv2::__testc(__a, __b);
            }
          else
            {
              constexpr int __valid_bits = (1 << (_Bs == 2 ? __size * 2 : __size)) - 1;
              return (__detail::__movmsk(__data(__k)._M_data) & __valid_bits) == __valid_bits;
            }
        }

#elif _GLIBCXX_SIMD_HAVE_NEON
      else if constexpr (__pv2::__is_neon_abi<_Abi>())
        {
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
          return [&]<__detail::_SimdSizeType... _Is>(__detail::_SimdIndexSequence<_Is...>)
                 _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                   return (... and (__k[_Is] != 0));
                 }(__detail::_MakeSimdIndexSequence<__size>());
        }
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    any_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      using _Kp = basic_simd_mask<_Bs, _Abi>;
      using _Tp = __pv2::__int_with_sizeof_t<_Bs>;
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

      else if constexpr (not _Kp::_S_is_bitmask and __size * _Bs <= sizeof(int64_t))
        return __builtin_bit_cast(__pv2::__int_with_sizeof_t<sizeof(__k)>, __data(__k)) != 0;

#if _GLIBCXX_SIMD_HAVE_SSE
      else if constexpr (__pv2::__is_avx512_abi<_Abi>())
        return (__data(__k)._M_data & _Abi::template _S_implicit_mask<_Tp>()) != 0;

      else if constexpr (__pv2::__is_sse_abi<_Abi>() or __pv2::__is_avx_abi<_Abi>())
        {
          static_assert(sizeof(__k) >= 16);
          if constexpr (__pv2::__have_sse4_1)
            {
              using _TI = __pv2::__intrinsic_type_t<_Tp, __size>;
              const _TI __a = reinterpret_cast<_TI>(__pv2::__to_intrin(__data(__k)));
              if constexpr (_Abi::template _S_is_partial<_Tp>)
                {
                  constexpr _TI __b = _Abi::template _S_implicit_mask_intrin<_Tp>();
                  return 0 == __pv2::__testz(__a, __b);
                }
              else
                return 0 == __pv2::__testz(__a, __a);
            }
          else
            {
              constexpr int __valid_bits = (1 << (_Bs == 2 ? __size * 2 : __size)) - 1;
              return (__detail::__movmsk(__data(__k)._M_data) & __valid_bits) != 0;
            }
        }

#elif
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
          return [&]<__detail::_SimdSizeType... _Is>(__detail::_SimdIndexSequence<_Is...>)
                 _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                   return (... or (__k[_Is] != 0));
                 }(__detail::_MakeSimdIndexSequence<__size>());
        }
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    none_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      using _Kp = basic_simd_mask<_Bs, _Abi>;
      using _Tp = __pv2::__int_with_sizeof_t<_Bs>;
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

      else if constexpr (not _Kp::_S_is_bitmask and sizeof(__k) <= sizeof(int64_t))
        return __builtin_bit_cast(__pv2::__int_with_sizeof_t<sizeof(__k)>,
                                  _Abi::_S_masked(__data(__k))) == 0;

#if _GLIBCXX_SIMD_HAVE_SSE
      else if constexpr (__pv2::__is_avx512_abi<_Abi>())
        return _Abi::_S_masked(__data(__k)) == 0;

      else if constexpr (__pv2::__is_sse_abi<_Abi>() || __pv2::__is_avx_abi<_Abi>())
        {
          static_assert(sizeof(__k) >= 16);
          if constexpr (__pv2::__have_sse4_1)
            {
              using _TI = __pv2::__intrinsic_type_t<_Tp, __size>;
              const _TI __a = reinterpret_cast<_TI>(__pv2::__to_intrin(__data(__k)));
              if constexpr (_Abi::template _S_is_partial<_Tp>)
                {
                  constexpr _TI __b = _Abi::template _S_implicit_mask_intrin<_Tp>();
                  return 0 != __pv2::__testz(__a, __b);
                }
              else
                return 0 != __pv2::__testz(__a, __a);
            }
          else
            {
              constexpr int __valid_bits = (1 << (_Bs == 2 ? __size * 2 : __size)) - 1;
              return (__detail::__movmsk(__data(__k)._M_data) & __valid_bits) == 0;
            }
        }

#elif _GLIBCXX_SIMD_HAVE_NEON
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
          return [&]<__detail::_SimdSizeType... _Is>(__detail::_SimdIndexSequence<_Is...>)
                 _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
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

#if _GLIBCXX_SIMD_HAVE_SSE
      if constexpr (__pv2::__is_avx512_abi<_Abi>())
        {
          const auto __kk = _Abi::_S_masked(__data(__k))._M_data;
          if constexpr (__size > 32)
            return __builtin_popcountll(__kk);
          else
            return __builtin_popcount(__kk);
        }

      else if constexpr (__pv2::__is_avx_abi<_Abi>() or __pv2::__is_sse_abi<_Abi>())
        {
          const auto __kk = _Abi::_S_masked(__data(__k))._M_data;
          const int __bits = __detail::__movmsk(__kk);
          if constexpr (__pv2::__have_popcnt)
            {
              const int __count = __builtin_popcount(__bits);
              return _Bs == 2 ? __count / 2 : __count;
            }
          else if constexpr (__size == 2 && _Bs != 2)
            return __bits - (__bits >> 1);
        }
#endif

      return -reduce(-__k);
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_min_index(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      constexpr int __size = basic_simd_mask<_Bs, _Abi>::size.value;
      if constexpr (__size == 1)
        return 0;

      else if (__builtin_is_constant_evaluated() or __k._M_is_constprop())
        {
          const int __r = [&] {
            for (int __i = 0; __i < __size - 1; ++__i)
              if (__k[__i])
                return __i;
            return __size - 1;
          }();
          if (__builtin_is_constant_evaluated() or __builtin_constant_p(__r))
            return __r;
        }

#if _GLIBCXX_SIMD_HAVE_SSE
      if constexpr (__pv2::__is_avx512_abi<_Abi>())
        return __detail::__lowest_bit(__data(__k)._M_data);

      if constexpr (__pv2::__is_avx_abi<_Abi>() or __pv2::__is_sse_abi<_Abi>())
        {
          const uint32_t __bits = __detail::__movmsk(__data(__k)._M_data);
          if constexpr (_Bs == 2)
            return __detail::__lowest_bit(__bits) / 2;
          else
            return __detail::__lowest_bit(__bits);
        }
#endif
      return __detail::__lowest_bit(_Abi::_MaskImpl::_S_to_bits(__data(__k))._M_to_bits());
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_max_index(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      constexpr int __size = basic_simd_mask<_Bs, _Abi>::size.value;
      if constexpr (__size == 1)
        return 0;

      else if (__builtin_is_constant_evaluated() or __k._M_is_constprop())
        {
          const int __r = [&] {
            for (int __i = __size - 1; __i > 0; --__i)
              if (__k[__i])
                return __i;
            return 0;
          }();
          if (__builtin_is_constant_evaluated() or __builtin_constant_p(__r))
            return __r;
        }

#if _GLIBCXX_SIMD_HAVE_SSE
      if constexpr (__pv2::__is_avx512_abi<_Abi>())
        return __detail::__highest_bit(__data(__k)._M_data);

      if constexpr (__pv2::__is_avx_abi<_Abi>() or __pv2::__is_sse_abi<_Abi>())
        {
          const uint32_t __bits = __detail::__movmsk(__data(__k)._M_data);
          if constexpr (_Bs == 2)
            return __detail::__highest_bit(__bits) / 2;
          else
            return __detail::__highest_bit(__bits);
        }
#endif
      return __detail::__highest_bit(_Abi::_MaskImpl::_S_to_bits(__data(__k))._M_to_bits());
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
  { return !__x; }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
  reduce_max_index(same_as<bool> auto __x) noexcept
  { return -!__x; }
}
#endif  // PROTOTYPE_MASK_REDUCTIONS_H_
