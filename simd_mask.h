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
    : public __detail::simd_mask<__detail::__int_with_sizeof_t<_Bytes>, _Abi>
    {
      using _Tp = __detail::__int_with_sizeof_t<_Bytes>;

      using _Traits = __detail::_SimdTraits<_Tp, _Abi>;

      using _MemberType = _Traits::_MaskMember;

      using _Base = std::experimental::simd_mask<_Tp, _Abi>;

      using _SimdType = std::basic_simd<_Tp, _Abi>;

    public:
      using value_type = bool;

      using _Impl = _Base::_Impl;

      static inline constexpr __detail::_Ic<__detail::_SimdSizeType(_Base::size())> size = {};

      _GLIBCXX_SIMD_INTRINSIC constexpr
      basic_simd_mask(const _Base& __x) noexcept
      : _Base(__x)
      {}

      constexpr
      basic_simd_mask() noexcept = default;

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
      basic_simd_mask(value_type __x) noexcept
      : _Base(__x)
      {}

      template <size_t _UBytes, class _UAbi>
        requires(_UBytes != _Bytes)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_simd_mask(const basic_simd_mask<_UBytes, _UAbi>& __x) noexcept
        : _Base(__detail::static_simd_cast<_Base>(__x))
        {}

      template <__detail::__simd_broadcast_invokable<value_type, size()> _Fp>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_simd_mask(_Fp&& __gen) noexcept
        : _Base(__detail::__private_init, static_cast<_Fp&&>(__gen))
        {}

      template <typename _It, typename _Flags = element_aligned_tag>
	requires std::same_as<std::iter_value_t<_It>, bool>
		   and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_simd_mask(_It __first, _Flags __f = {})
        : _Base(std::addressof(*__first), __f)
        {}

      template <typename _It, typename _Flags = element_aligned_tag>
        requires std::same_as<std::iter_value_t<_It>, bool>
                   and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_simd_mask(_It __first, const basic_simd_mask& __k, _Flags __f = {})
        : _Base()
        { __detail::where(__k, *this).copy_from(std::addressof(*__first), __f); }

      // private init
      _GLIBCXX_SIMD_INTRINSIC constexpr
      basic_simd_mask(__detail::_PrivateInit, const _MemberType& __init)
      : _Base(__detail::__private_init, __init)
      {}

/*      constexpr
      basic_simd_mask(const basic_simd_mask&) = default;

      constexpr
      basic_simd_mask(basic_simd_mask&&) = default;*/

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _SimdType
      operator-() const noexcept
      {
        if constexpr (sizeof(basic_simd_mask) == sizeof(_SimdType))
          return std::bit_cast<_SimdType>(*this);
        else
          {
            _SimdType __r = {};
            _SimdType::_Impl::_S_masked_assign(_M_data(), __r._M_data(), _Tp(-1));
            return __r;
          }
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _SimdType
      operator+() const noexcept
      { return operator _SimdType(); }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _SimdType
      operator~() const noexcept
      {
        if constexpr (sizeof(basic_simd_mask) == sizeof(_SimdType))
          return std::bit_cast<_SimdType>(*this) - _Tp(1);
        else
          {
            _SimdType __r = _Tp(-1);
            _SimdType::_Impl::_S_masked_assign(_M_data(), __r._M_data(), _Tp(-2));
            return __r;
          }
      }

      template <typename _Up, typename _UAbi>
        requires (simd_size_v<_Up, _UAbi> == size)
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

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      simd_select_impl(const basic_simd_mask& __k, const basic_simd_mask& __t,
                       const basic_simd_mask& __f)
      {
        basic_simd_mask __ret = __f;
        _Impl::_S_masked_assign(__k._M_data(), __ret._M_data(), __t._M_data());
        return __ret;
      }

      template <typename _U1, typename _U2>
        requires (__detail::__vectorizable<__detail::__nopromot_common_type_t<_U1, _U2>>
                    && sizeof(__detail::__nopromot_common_type_t<_U1, _U2>) == _Bytes)
        _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr
        simd<__detail::__nopromot_common_type_t<_U1, _U2>, size()>
        simd_select_impl(const basic_simd_mask& __k, const _U1& __t, const _U2& __f)
        {
          using _Rp = simd<__detail::__nopromot_common_type_t<_U1, _U2>, size()>;
          _Rp __ret = __f;
          _Rp::_Impl::_S_masked_assign(__data(__k), __data(__ret), __data(_Rp(__t)));
          return __ret;
        }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      simd_select_impl(const basic_simd_mask& __k, same_as<bool> auto __t, same_as<bool> auto __f)
      {
        if (__t == __f)
          return basic_simd_mask(__t);
        else if (__t)
          return __k;
        else
          return !__k;
      }

      constexpr const auto& _M_data() const
      { return __data(static_cast<const _Base&>(*this)); }

      constexpr auto& _M_data()
      { return __data(static_cast<_Base&>(*this)); }

      friend constexpr const auto& __data(const basic_simd_mask& __x)
      { return __x._M_data(); }

      friend constexpr auto& __data(basic_simd_mask& __x)
      { return __x._M_data(); }

      _GLIBCXX_SIMD_INTRINSIC constexpr bool
      _M_is_constprop() const
      { return static_cast<const _Base&>(*this)._M_is_constprop(); }
    };

  template <size_t _Bs, typename _Abi>
    struct is_simd_mask<basic_simd_mask<_Bs, _Abi>>
    : is_default_constructible<basic_simd_mask<_Bs, _Abi>>
    {};

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    all_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      if (__builtin_is_constant_evaluated() || __k._M_is_constprop())
        {
          for (int __i = 0; __i < __k.size(); ++__i)
            if (!__k[__i])
              return false;
          return true;
        }
      else
        return _Abi::_MaskImpl::_S_all_of(__k);
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    any_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      if (__builtin_is_constant_evaluated() || __k._M_is_constprop())
        {
          for (int __i = 0; __i < __k.size(); ++__i)
            if (__k[__i])
              return true;
          return false;
        }
      else
        return _Abi::_MaskImpl::_S_any_of(__k);
    }

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    none_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept
    {
      if (__builtin_is_constant_evaluated() || __k._M_is_constprop())
        {
          for (int __i = 0; __i < __k.size(); ++__i)
            if (__k[__i])
              return false;
          return true;
        }
      else
        return _Abi::_MaskImpl::_S_none_of(__k);
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
