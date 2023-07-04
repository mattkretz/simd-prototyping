/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_MASK2_H_
#define PROTOTYPE_SIMD_MASK2_H_

#include "detail.h"
#include "simd_abi.h"

#include <concepts>
#include <climits>

namespace std
{
  template <size_t _Bytes, typename _Abi>
    class basic_simd_mask
    : public __detail::_SimdTraits<__detail::__int_with_sizeof_t<_Bytes>, _Abi>::_MaskBase
    {
      using _Tp = __detail::__int_with_sizeof_t<_Bytes>;

      using _Traits = __detail::_SimdTraits<_Tp, _Abi>;

      using _MemberType = typename _Traits::_MaskMember;


      using _SimdType = std::basic_simd<_Tp, _Abi>;

      // the only non-static data member
      alignas(_Traits::_S_mask_align) _MemberType _M_data;

    public:
      using _Impl = typename _Traits::_MaskImpl;

      static constexpr bool _S_is_bitmask = sizeof(_MemberType) < _SimdType::size.value;

      // really public:

      using value_type = bool;

      using reference = __detail::_SmartReference<_MemberType, _Impl, value_type>;

      using abi_type = _Abi;

      static constexpr auto size = _SimdType::size;

      constexpr
      basic_simd_mask() noexcept = default;

      // suggested extension [simd.mask.overview] p4
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
      basic_simd_mask(typename _Traits::_MaskCastType __init)
      : _M_data{__init} {}
      // conversions to internal type is done in _MaskBase

      // private init (implementation detail)
      _GLIBCXX_SIMD_INTRINSIC constexpr
      basic_simd_mask(__detail::_PrivateInit, const _MemberType& __init) noexcept
      : _M_data(__init)
      {}

      // broadcast ctor
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
      basic_simd_mask(value_type __x) noexcept
      : _M_data(_Impl::template _S_broadcast<_Tp>(__x))
      {}

      // conversion ctor
      template <size_t _UBytes, class _UAbi>
        requires(size() == basic_simd_mask<_UBytes, _UAbi>::size.value)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_simd_mask(const basic_simd_mask<_UBytes, _UAbi>& __x) noexcept
        : basic_simd_mask(__detail::static_simd_cast<basic_simd_mask>(__x))
        {}

      // generator ctor
      template <__detail::__simd_broadcast_invokable<value_type, size.value> _Fp>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_simd_mask(_Fp&& __gen) noexcept
        : _M_data([&]<__detail::_SimdSizeType... _Is>(__detail::_SimdIndexSequence<_Is...>) {
            if constexpr (size.value == 1)
              return __gen(__detail::__ic<0>);
            else if constexpr (_S_is_bitmask)
              return ((uint64_t(__gen(__detail::__ic<_Is>)) << _Is) | ...);
            else
              return _MemberType { -__gen(__detail::__ic<_Is>)... };
          }(__detail::_MakeSimdIndexSequence<size()>()))
        {}

      // load ctor
      template <typename _It, typename _Flags = element_aligned_tag>
        requires std::same_as<std::iter_value_t<_It>, bool>
                   and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_simd_mask(_It __first, _Flags __f = {})
        { copy_from(__first, __f); }

      // masked load ctor
      template <typename _It, typename _Flags = element_aligned_tag>
        requires std::same_as<std::iter_value_t<_It>, bool>
                   and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_simd_mask(_It __first, const basic_simd_mask& __k, _Flags = {})
        : _M_data {}
        {
          const auto* __ptr = _Flags::template _S_apply<basic_simd_mask>(std::addressof(*__first));
          _M_data = _Impl::_S_masked_load(_M_data, __k._M_data, __ptr);
        }

      // loads [simd.mask.copy]
      template <typename _It, typename _Flags = element_aligned_tag>
        requires std::same_as<std::iter_value_t<_It>, bool>
                   and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(_It __first, _Flags = {})
        {
          const auto* __ptr = _Flags::template _S_apply<basic_simd_mask>(std::addressof(*__first));
          if (__builtin_is_constant_evaluated())
            {
              _M_data = [&]<__detail::_SimdSizeType... _Is>(__detail::_SimdIndexSequence<_Is...>)
                        _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                          if constexpr (size.value == 1)
                            return *__first;
                          else if constexpr (_S_is_bitmask)
                            return ((__first[_Is] ? (1ull << _Is) : 0) | ...);
                          else
                            return _MemberType { -__first[_Is]... };
                        }(__detail::_MakeSimdIndexSequence<size.value>());
            }
          else
            _M_data = _Impl::template _S_load<_Tp>(__ptr);
        }

      template <typename _It, typename _Flags = element_aligned_tag>
        requires std::same_as<std::iter_value_t<_It>, bool>
                   and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(_It __first, const basic_simd_mask& __k, _Flags = {})
        {
          const auto* __ptr = _Flags::template _S_apply<basic_simd_mask>(std::addressof(*__first));
          _M_data = _Impl::_S_masked_load(_M_data, __k._M_data, __ptr);
        }

      // stores [simd.mask.copy]
      template <typename _It, typename _Flags>
        requires std::same_as<std::iter_value_t<_It>, bool>
                   and std::contiguous_iterator<_It>
                   and std::indirectly_writable<_It, value_type>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_It __first, _Flags) const
        {
          const auto* __ptr = _Flags::template _S_apply<basic_simd_mask>(std::addressof(*__first));
          _Impl::_S_store(_M_data, __ptr);
        }

      template <typename _It, typename _Flags>
        requires std::same_as<std::iter_value_t<_It>, bool>
                   and std::contiguous_iterator<_It>
                   and std::indirectly_writable<_It, value_type>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_It __first, const basic_simd_mask& __k, _Flags) const
        {
          const auto* __ptr = _Flags::template _S_apply<basic_simd_mask>(std::addressof(*__first));
          _Impl::_S_masked_store(_M_data, __ptr, __k._M_data);
        }

      // [simd.mask.subscr]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr reference
      operator[](__detail::_SimdSizeType __i) &
      {
        if (__i >= size())
          __detail::__invoke_ub("Subscript %d is out of range [0, %d]", __i, size() - 1);
        return {_M_data, __i};
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr value_type
      operator[](__detail::_SimdSizeType __i) const &
      {
        if (__i >= size())
          __detail::__invoke_ub("Subscript %d is out of range [0, %d]", __i, size() - 1);
        if constexpr (size.value == 1)
          return _M_data;
        else
          return static_cast<bool>(_M_data[__i]);
      }

      // [simd.mask.unary]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd_mask
      operator!() const noexcept
      { return {__detail::__private_init, _Impl::_S_bit_not(_M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _SimdType
      operator+() const noexcept
      { return operator _SimdType(); }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _SimdType
      operator-() const noexcept
      {
        if constexpr (sizeof(basic_simd_mask) == sizeof(_SimdType))
          return std::bit_cast<_SimdType>(*this);
        else
          {
            _SimdType __r = {};
            _SimdType::_Impl::_S_masked_assign(_M_data, __r._M_data(), _Tp(-1));
            return __r;
          }
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _SimdType
      operator~() const noexcept
      {
        if constexpr (sizeof(basic_simd_mask) == sizeof(_SimdType))
          return std::bit_cast<_SimdType>(*this) - _Tp(1);
        else
          {
            _SimdType __r = _Tp(-1);
            _SimdType::_Impl::_S_masked_assign(_M_data, __r._M_data(), _Tp(-2));
            return __r;
          }
      }

      // [simd.mask.conv]
      template <typename _Up, typename _UAbi>
        requires (__detail::simd_size_v<_Up, _UAbi> == size.value)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit(sizeof(_Up) != _Bytes)
        operator basic_simd<_Up, _UAbi>() const noexcept
        {
          using _Rp = basic_simd<_Up, _UAbi>;
          if constexpr (sizeof(basic_simd_mask) == sizeof(_Rp) && sizeof(_Up) == _Bytes)
            {
              using _Unsigned = simd<__detail::__make_unsigned_t<_Up>, _Rp::size()>;
              const auto __bits = std::bit_cast<_Unsigned>(__data(*this));
              if constexpr (std::integral<_Up>)
                return std::bit_cast<_Rp>(__bits >> (sizeof(_Up) * CHAR_BIT - 1));
              else
                return std::bit_cast<_Rp>(__bits & std::bit_cast<_Unsigned>(
                                                     _Rp::_Impl::_S_broadcast(_Up(1))));
            }
          else
            {
              _Rp __r {};
              _Rp::_Impl::_S_masked_assign(__data(*this), __data(__r), 1);
              return __r;
            }
        }

      // [simd.mask.binary]
      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator&&(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_logical_and(__x._M_data, __y._M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator||(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_logical_or(__x._M_data, __y._M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator&(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_bit_and(__x._M_data, __y._M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator|(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_bit_or(__x._M_data, __y._M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator^(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_bit_xor(__x._M_data, __y._M_data)}; }

      // [simd.mask.cassign]
      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask&
      operator&=(basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      {
        __x._M_data = _Impl::_S_bit_and(__x._M_data, __y._M_data);
        return __x;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask&
      operator|=(basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      {
        __x._M_data = _Impl::_S_bit_or(__x._M_data, __y._M_data);
        return __x;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask&
      operator^=(basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      {
        __x._M_data = _Impl::_S_bit_xor(__x._M_data, __y._M_data);
        return __x;
      }

      // [simd.mask.comparison]
      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator==(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      {
        return {__detail::__private_init,
                _Impl::_S_bit_not(_Impl::_S_bit_xor(__x._M_data, __y._M_data))};
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator!=(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_bit_xor(__x._M_data, __y._M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator>=(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      { return __x || !__y; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator<=(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      { return !__x || __y; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator>(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      { return __x && !__y; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      operator<(const basic_simd_mask& __x, const basic_simd_mask& __y) noexcept
      { return !__x && __y; }

      // [simd.mask.cond]
      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      simd_select_impl(const basic_simd_mask& __k, const basic_simd_mask& __t,
                       const basic_simd_mask& __f) noexcept
      {
        basic_simd_mask __ret = __f;
        _Impl::_S_masked_assign(__k._M_data, __ret._M_data, __t._M_data);
        return __ret;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      simd_select_impl(const basic_simd_mask& __k, same_as<bool> auto __t,
                       same_as<bool> auto __f) noexcept
      {
        if (__t == __f)
          return basic_simd_mask(__t);
        else if (__t)
          return __k;
        else
          return !__k;
      }

      template <typename _T0, typename _T1>
        requires (__detail::__vectorizable<__detail::__nopromot_common_type_t<_T0, _T1>>
                    and sizeof(__detail::__nopromot_common_type_t<_T0, _T1>) == _Bytes
                    and convertible_to<_T0, simd<__detail::__nopromot_common_type_t<_T0, _T1>,
                                                 size.value>>
                    and convertible_to<_T1, simd<__detail::__nopromot_common_type_t<_T0, _T1>,
                                                 size.value>>)
        _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr
        simd<__detail::__nopromot_common_type_t<_T0, _T1>, size.value>
        simd_select_impl(const basic_simd_mask& __k, const _T0& __t, const _T1& __f) noexcept
        {
          using _Rp = simd<__detail::__nopromot_common_type_t<_T0, _T1>, size()>;
          _Rp __ret = __f;
          _Rp::_Impl::_S_masked_assign(__data(__k), __data(__ret), __data(_Rp(__t)));
          return __ret;
        }

      _GLIBCXX_SIMD_INTRINSIC friend constexpr const auto&
      __data(const basic_simd_mask& __x)
      { return __x._M_data; }

      _GLIBCXX_SIMD_INTRINSIC friend constexpr auto&
      __data(basic_simd_mask& __x)
      { return __x._M_data; }

      _GLIBCXX_SIMD_INTRINSIC constexpr bool
      _M_is_constprop() const
      {
        if constexpr (size.value == 1)
          return __builtin_constant_p(_M_data);
        else
          return _M_data._M_is_constprop();
      }
    };

  template <size_t _Bs, typename _Abi>
    struct is_simd_mask<basic_simd_mask<_Bs, _Abi>>
    : is_default_constructible<basic_simd_mask<_Bs, _Abi>>
    {};

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    all_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      using _Kp = basic_simd_mask<_Bs, _Abi>;
      using _Tp = __detail::__int_with_sizeof_t<_Bs>;
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
            = __builtin_bit_cast(__detail::__int_with_sizeof_t<sizeof(__k)>, __data(__k));
          if constexpr (__has_single_bit(__size))
            return __as_int == -1;
          else
            return __as_int & ((1ll << (__size * _Bs * __CHAR_BIT__)) - 1)
                     == (1ll << (__size * _Bs * __CHAR_BIT__)) - 1;
        }

#if _GLIBCXX_SIMD_HAVE_SSE
      else if constexpr (__detail::__is_avx512_abi<_Abi>())
        {
          constexpr auto _Mask = _Abi::template _S_implicit_mask<_Tp>();
          const auto __kk = __data(__k)._M_data;
          if constexpr (sizeof(__kk) == 1)
            {
              if constexpr (__detail::__have_avx512dq)
                return __builtin_ia32_kortestcqi(__kk, _Mask == 0xff ? __kk : uint8_t(~_Mask));
              else
                return __builtin_ia32_kortestchi(__kk, uint16_t(~_Mask));
            }
          else if constexpr (sizeof(__kk) == 2)
            return __builtin_ia32_kortestchi(__kk, _Mask == 0xffff ? __kk : uint16_t(~_Mask));
          else if constexpr (sizeof(__kk) == 4 && __detail::__have_avx512bw)
            return __builtin_ia32_kortestcsi(__kk, _Mask == 0xffffffffU ? __kk : uint32_t(~_Mask));
          else if constexpr (sizeof(__kk) == 8 && __detail::__have_avx512bw)
            return __builtin_ia32_kortestcdi(
                     __kk, _Mask == 0xffffffffffffffffULL ? __kk : uint64_t(~_Mask));
          else
            __detail::__assert_unreachable<_Tp>();
        }

      else if constexpr (__detail::__is_sse_abi<_Abi>() or __detail::__is_avx_abi<_Abi>())
        {
          static_assert(sizeof(__k) >= 16);
          if constexpr (__detail::__have_sse4_1)
            {
              using _TI = __detail::__intrinsic_type_t<_Tp, __size>;
              const _TI __a = reinterpret_cast<_TI>(__detail::__to_intrin(__data(__k)));
              constexpr _TI __b = _Abi::template _S_implicit_mask_intrin<_Tp>();
              return 0 != __detail::__testc(__a, __b);
            }
          else if constexpr (is_same_v<_Tp, float>)
            return (__builtin_ia32_movmskps(__data(__k)) & ((1 << __size) - 1))
                     == (1 << __size) - 1;
          else if constexpr (is_same_v<_Tp, double>)
            return (__builtin_ia32_movmskpd(__data(__k)) & ((1 << __size) - 1))
                     == (1 << __size) - 1;
          else
            return (__builtin_ia32_pmovmskb128(__detail::__vector_bitcast<char>(__data(__k)))
                      & ((1 << (__size * _Bs)) - 1))
                     == (1 << (__size * _Bs)) - 1;
        }

#elif _GLIBCXX_SIMD_HAVE_NEON
      else if constexpr (__detail::__is_neon_abi<_Abi>())
        {
          static_assert(sizeof(__k) == 16);
          const auto __kk
            = __detail::__vector_bitcast<char>(__data(__k))
                | ~__detail::__vector_bitcast<char>(_Abi::template _S_implicit_mask<_Tp>());
          const auto __x = __detail::__vector_bitcast<int64_t>(__kk);
          return __x[0] + __x[1] == -2;
        }

#endif
      else
        {
          return [&]<__detail::_SimdSizeType... _Is>(__detail::_SimdIndexSequence<_Is...>)
                 _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                   return (... and not (__k[_Is] == 0));
                 }(__detail::_MakeSimdIndexSequence<__size>());
        }
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    any_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      using _Kp = basic_simd_mask<_Bs, _Abi>;
      using _Tp = __detail::__int_with_sizeof_t<_Bs>;
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
        return __builtin_bit_cast(__detail::__int_with_sizeof_t<sizeof(__k)>, __data(__k)) != 0;

#if _GLIBCXX_SIMD_HAVE_SSE
      else if constexpr (__detail::__is_avx512_abi<_Abi>())
        return (__data(__k)._M_data & _Abi::template _S_implicit_mask<_Tp>()) != 0;

      else if constexpr (__detail::__is_sse_abi<_Abi>() or __detail::__is_avx_abi<_Abi>())
        {
          static_assert(sizeof(__k) >= 16);
          if constexpr (__detail::__have_sse4_1)
            {
              using _TI = __detail::__intrinsic_type_t<_Tp, __size>;
              const _TI __a = reinterpret_cast<_TI>(__detail::__to_intrin(__data(__k)));
              if constexpr (_Abi::template _S_is_partial<_Tp>)
                {
                  constexpr _TI __b = _Abi::template _S_implicit_mask_intrin<_Tp>();
                  return 0 == __detail::__testz(__a, __b);
                }
              else
                return 0 == __detail::__testz(__a, __a);
            }
          else if constexpr (is_same_v<_Tp, float>)
            return (__builtin_ia32_movmskps(__data(__k)) & ((1 << __size) - 1)) != 0;
          else if constexpr (is_same_v<_Tp, double>)
            return (__builtin_ia32_movmskpd(__data(__k)) & ((1 << __size) - 1)) != 0;
          else
            return (__builtin_ia32_pmovmskb128(__detail::__vector_bitcast<char>(__data(__k)))
                      & ((1 << (__size * _Bs)) - 1)) != 0;
        }

#elif
      else if constexpr (__detail::__is_neon_abi<_Abi>())
        {
          static_assert(sizeof(__k) == 16);
          const auto __kk = _Abi::_S_masked(__data(__k));
          const auto __x = __detail::__vector_bitcast<int64_t>(__kk);
          return (__x[0] | __x[1]) != 0;
        }

#endif
      else
        {
          return [&]<__detail::_SimdSizeType... _Is>(__detail::_SimdIndexSequence<_Is...>)
                 _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                   return (... or not (__k[_Is] == 0));
                 }(__detail::_MakeSimdIndexSequence<__size>());
        }
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    none_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      using _Kp = basic_simd_mask<_Bs, _Abi>;
      using _Tp = __detail::__int_with_sizeof_t<_Bs>;
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
        return __builtin_bit_cast(__detail::__int_with_sizeof_t<sizeof(__k)>,
                                  _Abi::_S_masked(__data(__k))) == 0;

#if _GLIBCXX_SIMD_HAVE_SSE
      else if constexpr (__detail::__is_avx512_abi<_Abi>())
        return _Abi::_S_masked(__data(__k)) == 0;

      else if constexpr (__detail::__is_sse_abi<_Abi>() || __detail::__is_avx_abi<_Abi>())
        {
          static_assert(sizeof(__k) >= 16);
          using _TI = __detail::__intrinsic_type_t<_Tp, __size>;
          const _TI __a = reinterpret_cast<_TI>(__detail::__to_intrin(__data(__k)));
          if constexpr (__detail::__have_sse4_1)
            {
              if constexpr (_Abi::template _S_is_partial<_Tp>)
                {
                  constexpr _TI __b = _Abi::template _S_implicit_mask_intrin<_Tp>();
                  return 0 != __detail::__testz(__a, __b);
                }
              else
                return 0 != __detail::__testz(__a, __a);
            }
          else if constexpr (is_same_v<_Tp, float>)
            return (__detail::__movemask(__a) & ((1 << __size) - 1)) == 0;
          else if constexpr (is_same_v<_Tp, double>)
            return (__detail::__movemask(__a) & ((1 << __size) - 1)) == 0;
          else
            return (__detail::__movemask(__a) & int((1ull << (__size * _Bs)) - 1)) == 0;
        }

#elif _GLIBCXX_SIMD_HAVE_NEON
      else if constexpr (__detail::__is_neon_abi<_Abi>())
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

  template <typename _Tp, typename _Up>
    constexpr auto
    simd_select(bool __c, const _Tp& __x0, const _Up& __x1)
    -> remove_cvref_t<decltype(__c ? __x0 : __x1)>
    { return __c ? __x0 : __x1; }

  template <size_t _Np, typename _A0>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr auto
    simd_select(const basic_simd_mask<_Np, _A0>& __k, const auto& __x0, const auto& __x1)
    -> decltype(simd_select_impl(__k, __x0, __x1))
    { return simd_select_impl(__k, __x0, __x1); }
}

#endif  // PROTOTYPE_SIMD_MASK2_H_
// vim: et ts=8 sw=2 tw=100 cc=101
