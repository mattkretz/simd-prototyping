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
  template <typename _Tp, typename _Abi>
    class simd_mask : public __detail::simd_mask<_Tp, _Abi>
    {
      using _Traits = __detail::_SimdTraits<_Tp, _Abi>;

      using _MemberType = typename _Traits::_MaskMember;

      using _Base = std::experimental::simd_mask<_Tp, _Abi>;

    public:
      using value_type = bool;
      using simd_type = std::simd<_Tp, _Abi>;
      using _Impl = _Base::_Impl;

      static inline constexpr __detail::_Cnst<_Base::size()> size = {};

      _GLIBCXX_SIMD_INTRINSIC constexpr
      simd_mask(const _Base& __x) noexcept
      : _Base(__x)
      {}

      constexpr
      simd_mask() noexcept = default;

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
      simd_mask(value_type __x) noexcept
      : _Base(__x)
      {}

      template <typename _Up, class _UAbi>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit(sizeof(_Up) != sizeof(_Tp))
        simd_mask(const simd_mask<_Up, _UAbi>& __x) noexcept
        : _Base(__detail::static_simd_cast<_Base>(__x))
        {}

      template <__detail::__simd_broadcast_invokable<value_type, size()> _Fp>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        simd_mask(_Fp&& __gen) noexcept
        : _Base(__detail::__private_init, static_cast<_Fp&&>(__gen))
        {}

      template <typename _Flags = element_aligned_tag>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        simd_mask(const value_type* __mem, _Flags __f = {})
        : _Base(__mem, __f)
        {}

      // private init
      _GLIBCXX_SIMD_INTRINSIC constexpr
      simd_mask(__detail::_PrivateInit, const _MemberType& __init)
      : _Base(__detail::__private_init, __init)
      {}

/*      constexpr
      simd_mask(const simd_mask&) = default;

      constexpr
      simd_mask(simd_mask&&) = default;*/

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr simd_type
      operator-() const
      {
        if constexpr (sizeof(simd_mask) == sizeof(simd_type))
          {
            if constexpr (std::integral<_Tp>)
              return std::bit_cast<simd_type>(*this);

            using U = rebind_simd_t<__detail::__make_unsigned_t<_Tp>, simd_type>;
            const auto __bits = std::bit_cast<U>(*this);
            return std::bit_cast<simd_type>(__bits & std::bit_cast<U>(simd_type(_Tp(-1))));
          }
        else
          {
            simd_type __r = {};
            where(*this, __r) = _Tp(-1);
            return __r;
          }
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr simd_type
      operator+() const
      { return operator simd_type(); }

      template <typename _Up, typename _UAbi>
        requires (simd_size_v<_Up, _UAbi> == size)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit(sizeof(_Up) != sizeof(_Tp))
        operator simd<_Up, _UAbi>() const
        {
          using _Rp = simd<_Up, _UAbi>;
          if constexpr (sizeof(simd_mask) == sizeof(_Rp) && sizeof(_Up) == sizeof(_Tp))
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
            return _Rp::_Impl::_S_blend(__data(*this), {}, _Rp::_Impl::_S_broadcast(_Up(1)));
        }

#ifdef __GXX_CONDITIONAL_IS_OVERLOADABLE__
#define conditional_operator operator?:
#endif

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr simd_mask
      conditional_operator(const simd_mask& __k, const simd_mask& __t, const simd_mask& __f)
      {
        simd_mask __ret = __f;
        _Impl::_S_masked_assign(__k._M_data, __ret._M_data, __t._M_data);
        return __ret;
      }

      template <typename _Kp, typename _Ak, typename _Up, typename _Au>
        requires (convertible_to<simd_mask<_Kp, _Ak>, simd_mask>
                    && convertible_to<simd_mask<_Up, _Au>, simd_mask>)
        _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr simd_mask
        conditional_operator(const simd_mask<_Kp, _Ak>& __k, const simd_mask& __t,
                             const simd_mask<_Up, _Au>& __f)
        {
          simd_mask __ret = __f;
          _Impl::_S_masked_assign(simd_mask(__k)._M_data, __ret._M_data,
                                  __t._M_data);
          return __ret;
        }

      template <typename _U1, typename _U2>
        requires (__detail::__vectorizable<common_type_t<_U1, _U2>>
                    && convertible_to<simd_mask, rebind_simd_t<common_type_t<_U1, _U2>, simd_mask>>)
        _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr
        rebind_simd_t<common_type_t<_U1, _U2>, simd_type>
        conditional_operator(const simd_mask& __k, const _U1& __t, const _U2& __f)
        {
          using _Rp = rebind_simd_t<common_type_t<_U1, _U2>, simd_type>;
          _Rp __ret = __f;
          _Rp::_Impl::_S_masked_assign(
            __data(static_cast<typename _Rp::mask_type>(__k)), __data(__ret),
            __data(_Rp(__t)));
          return __ret;
        }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr simd_mask
      conditional_operator(const simd_mask& __k, bool __t, bool __f)
      {
        if (__t == __f)
          return simd_mask(__t);
        else if (__t)
          return __k;
        else
          return !__k;
      }

#ifdef __GXX_CONDITIONAL_IS_OVERLOADABLE__
#undef conditional_operator
#endif
    };

  template <typename _Tp, typename _Abi>
    struct is_simd_mask<simd_mask<_Tp, _Abi>> : is_default_constructible<simd_mask<_Tp, _Abi>>
    {};

}

#endif  // PROTOTYPE_SIMD_MASK2_H_
