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

    public:
      using value_type = bool;

      using simd_type = std::simd<_Tp, _Abi>;

      using _Impl = _Base::_Impl;

      static inline constexpr __detail::_Cnst<_Base::size()> size = {};

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

      template <size_t _Np, class _UAbi>
        requires(_Np != _Bytes)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_simd_mask(const basic_simd_mask<_Np, _UAbi>& __x) noexcept
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

      // private init
      _GLIBCXX_SIMD_INTRINSIC constexpr
      basic_simd_mask(__detail::_PrivateInit, const _MemberType& __init)
      : _Base(__detail::__private_init, __init)
      {}

/*      constexpr
      basic_simd_mask(const basic_simd_mask&) = default;

      constexpr
      basic_simd_mask(basic_simd_mask&&) = default;*/

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr simd_type
      operator-() const noexcept
      {
        if constexpr (sizeof(basic_simd_mask) == sizeof(simd_type))
          return std::bit_cast<simd_type>(*this);
        else
          {
            simd_type __r = {};
            where(*this, __r) = _Tp(-1);
            return __r;
          }
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr simd_type
      operator+() const noexcept
      { return operator simd_type(); }

      template <typename _Up, typename _UAbi>
        requires (simd_size_v<_Up, _UAbi> == size)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit(sizeof(_Up) != _Bytes)
        operator simd<_Up, _UAbi>() const noexcept
        {
          using _Rp = simd<_Up, _UAbi>;
          if constexpr (sizeof(basic_simd_mask) == sizeof(_Rp) && sizeof(_Up) == _Bytes)
            {
              using _Unsigned = rebind_simd_t<__detail::__make_unsigned_t<_Up>, _Rp>;
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

#ifdef __GXX_CONDITIONAL_IS_OVERLOADABLE__
#define conditional_operator_impl operator?:
#endif

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      conditional_operator_impl(const basic_simd_mask& __k, const basic_simd_mask& __t, const basic_simd_mask& __f)
      {
        basic_simd_mask __ret = __f;
        _Impl::_S_masked_assign(__k._M_data, __ret._M_data, __t._M_data);
        return __ret;
      }

      template <typename _U1, typename _U2>
        requires (__detail::__vectorizable<common_type_t<_U1, _U2>>
                    && sizeof(common_type_t<_U1, _U2>) == _Bytes)
        _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr
        simd<common_type_t<_U1, _U2>, _Abi>
        conditional_operator_impl(const basic_simd_mask& __k, const _U1& __t, const _U2& __f)
        {
          using _Rp = simd<common_type_t<_U1, _U2>, _Abi>;
          _Rp __ret = __f;
          _Rp::_Impl::_S_masked_assign(__data(__k), __data(__ret), __data(_Rp(__t)));
          return __ret;
        }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd_mask
      conditional_operator_impl(const basic_simd_mask& __k, bool __t, bool __f)
      {
        if (__t == __f)
          return basic_simd_mask(__t);
        else if (__t)
          return __k;
        else
          return !__k;
      }

#ifdef __GXX_CONDITIONAL_IS_OVERLOADABLE__
#undef conditional_operator_impl
#endif
    };

  template <size_t _Np, typename _Abi>
    struct is_simd_mask<basic_simd_mask<_Np, _Abi>>
    : is_default_constructible<basic_simd_mask<_Np, _Abi>>
    {};

  namespace __cust_condop
  {
    void conditional_operator_impl(const auto&, const auto&, const auto&) = delete;

    template <typename _Cond, typename _T0, typename _T1>
      concept __adl_condop
        = (__detail::__class_or_enum<remove_reference_t<_Cond>>
             or __detail::__class_or_enum<remove_reference_t<_T0>>
             or __detail::__class_or_enum<remove_reference_t<_T1>>)
            and requires(_Cond&& __c, _T0&& __x0, _T1&& __x1)
      {
        conditional_operator_impl(static_cast<_Cond&&>(__c),
                                  static_cast<_T0&&>(__x0), static_cast<_T1&&>(__x1));
      };

    struct _ConditionalOperator
    {
      template <typename _Cond, typename _T0, typename _T1>
        constexpr auto
        operator()(_Cond&& __c, _T0&& __x0, _T1&& __x1) const
        {
          if constexpr (__adl_condop<_Cond, _T0, _T1>)
            return conditional_operator_impl(static_cast<_Cond&&>(__c),
                                             static_cast<_T0&&>(__x0), static_cast<_T1&&>(__x1));
          else
            return static_cast<_Cond&&>(__c) ? static_cast<_T0&&>(__x0) : static_cast<_T1&&>(__x1);
        }
    };
  }

  inline constexpr __cust_condop::_ConditionalOperator conditional_operator{};
}

#endif  // PROTOTYPE_SIMD_MASK2_H_
// vim: et ts=8 sw=2 tw=120 cc=121
