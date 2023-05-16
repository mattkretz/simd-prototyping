/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD2_H_
#define PROTOTYPE_SIMD2_H_

#include "detail.h"
#include "simd_mask.h"
#include <span>
#include <iterator>

namespace std
{
  template <typename _Tp, typename _Abi>
    class simd : protected __detail::simd<_Tp, _Abi>
    {
      using _Traits = __detail::_SimdTraits<_Tp, _Abi>;

      using _MemberType = typename _Traits::_SimdMember;

      using _Base = __detail::simd<_Tp, _Abi>;

    public:
      using _Impl = typename _Traits::_SimdImpl;
      friend _Impl;

      using reference = _Base::reference;

      using value_type = _Base::value_type;

      using abi_type = _Base::abi_type;

      using mask_type = std::simd_mask<_Tp, _Abi>;

      static inline constexpr __detail::_Cnst<_Base::size()> size = {};

      constexpr
      simd() = default;

/*      constexpr
      simd(const simd&) = default;

      constexpr
      simd(simd&&) = default;*/

      constexpr
      simd(const _Base& b)
      : _Base(b)
      {}

      // implicit broadcast constructor
      template <__detail::__value_preserving_or_int<value_type> _Up>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        simd(_Up&& __x) noexcept
        : _Base(std::forward<_Up>(__x))
        {}

      // implicit type conversion constructor
      template <typename _Up, typename _UAbi>
        requires(__detail::simd_size_v<_Up, _UAbi> == size())
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        explicit(not __detail::__value_preserving_convertible_to<_Up, value_type>
                   || __detail::__higher_rank_than<_Up, value_type>)
        simd(const simd<_Up, _UAbi>& __x) noexcept
        : _Base(__detail::static_simd_cast<_Base>(__x))
        {}

      // generator constructor
      template <__detail::__simd_broadcast_invokable<value_type, size()> _Fp>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        simd(_Fp&& __gen) noexcept
        : _Base(std::forward<_Fp>(__gen))
        {}

      // load constructor
      template <std::contiguous_iterator _It, typename _Flags = __detail::element_aligned_tag>
        requires std::input_iterator<_It> && __detail::__vectorizable<std::iter_value_t<_It>>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        simd(const _It& __first, _Flags __f = {})
        : _Base(std::addressof(*__first), __f)
        {}

      // private init
      _GLIBCXX_SIMD_INTRINSIC constexpr
      simd(__detail::_PrivateInit, const _MemberType& __init)
      : _Base(__detail::__private_init, __init)
      {}

      // loads and stores
      template <std::contiguous_iterator _It, typename _Flags = __detail::element_aligned_tag>
        requires std::input_iterator<_It> && __detail::__vectorizable<std::iter_value_t<_It>>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(const _It& __first, _Flags __f = {})
        { _Base::copy_from(std::addressof(*__first), __f); }

      template <std::contiguous_iterator _It, typename _Flags = __detail::element_aligned_tag>
        requires std::input_iterator<_It> && __detail::__vectorizable<std::iter_value_t<_It>>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from_if(const _It& __first, const mask_type& __k, _Flags __f = {})
        { __detail::where(__k, *this).copy_from(std::addressof(*__first), __f); }

      template <std::contiguous_iterator _It, typename _Flags = __detail::element_aligned_tag>
        requires std::output_iterator<_It, _Tp> && __detail::__vectorizable<std::iter_value_t<_It>>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(const _It& __first, _Flags __f = {}) const
        { _Base::copy_to(std::addressof(*__first), __f); }

      template <std::contiguous_iterator _It, typename _Flags = __detail::element_aligned_tag>
        requires std::output_iterator<_It, _Tp> && __detail::__vectorizable<std::iter_value_t<_It>>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to_if(const _It& __first, const mask_type& __k, _Flags __f = {}) const
        { __detail::where(__k, *this).copy_to(std::addressof(*__first), __f); }

      // compound assignment [simd.cassign]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend simd&
      operator+=(simd& __lhs, const simd& __x)
      { return __lhs = __lhs + __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend simd&
      operator-=(simd& __lhs, const simd& __x)
      { return __lhs = __lhs - __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend simd&
      operator*=(simd& __lhs, const simd& __x)
      { return __lhs = __lhs * __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend simd&
      operator/=(simd& __lhs, const simd& __x)
      { return __lhs = __lhs / __x; }

      // binary operators [simd.binary]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend simd
      operator+(const simd& __x, const simd& __y)
      { return {__detail::__private_init, _Impl::_S_plus(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend simd
      operator-(const simd& __x, const simd& __y)
      { return {__detail::__private_init, _Impl::_S_minus(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend simd
      operator*(const simd& __x, const simd& __y)
      { return {__detail::__private_init, _Impl::_S_multiplies(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend simd
      operator/(const simd& __x, const simd& __y)
      { return {__detail::__private_init, _Impl::_S_divides(__data(__x), __data(__y))}; }

      // compares [simd.comparison]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator==(const simd& __x, const simd& __y)
      { return {__detail::__private_init, _Impl::_S_equal_to(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator!=(const simd& __x, const simd& __y)
      { return {__detail::__private_init, _Impl::_S_not_equal_to(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator<(const simd& __x, const simd& __y)
      { return {__detail::__private_init, _Impl::_S_less(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator<=(const simd& __x, const simd& __y)
      { return {__detail::__private_init, _Impl::_S_less_equal(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator>(const simd& __x, const simd& __y)
      { return {__detail::__private_init, _Impl::_S_less(__data(__y), __data(__x))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator>=(const simd& __x, const simd& __y)
      { return {__detail::__private_init, _Impl::_S_less_equal(__data(__y), __data(__x))}; }

      // construction from span is simple
      constexpr
      simd(std::span<_Tp, size()> __mem)
      : _Base(__mem.data(), __detail::element_aligned)
      {}

      // ranges typically don't have a static size() function :(
      // but if one does, this ctor is useful
      template <std::ranges::contiguous_range _Rg>
        requires (std::same_as<std::ranges::range_value_t<_Rg>, _Tp>
                    && __detail::__static_range_size<_Rg> == size())
        constexpr
        simd(const _Rg& __range)
        : _Base(std::ranges::data(__range), __detail::element_aligned)
        {}

      template <std::ranges::random_access_range _Rg>
        requires(std::convertible_to<std::ranges::range_value_t<_Rg>, _Tp>)
        constexpr explicit (not std::same_as<std::ranges::range_value_t<_Rg>, _Tp>)
        simd(const _Rg& __range)
        : _Base([&__range](auto __i) -> _Tp { return __range[__i]; })
        {}

      constexpr std::array<_Tp, size()>
      to_array() const noexcept
      {
        std::array<_Tp, size()> __r = {};
        this->copy_to(__r.data(), __detail::element_aligned);
        return __r;
      }

      explicit
      operator std::array<_Tp, size()>() const noexcept
      { return to_array(); }

#ifdef __GXX_CONDITIONAL_IS_OVERLOADABLE__
#define conditional_operator operator?:
#endif

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr simd
      conditional_operator(const mask_type& __k, const simd& __t, const simd& __f)
      {
        auto __ret = __f;
        _Impl::_S_masked_assign(__data(__k), __data(__ret), __data(__t));
        return __ret;
      }

#ifdef __GXX_CONDITIONAL_IS_OVERLOADABLE__
#undef conditional_operator
#endif

      ///////////////////////
      // P2664::begin

      using _Base::operator[];

      template <std::integral _Up, typename _Ap>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr rebind_simd_t<_Tp, simd<_Up, _Ap>>
        operator[](simd<_Up, _Ap> const& __idx) const noexcept
        {
          using _Rp = rebind_simd_t<_Tp, simd<_Up, _Ap>>;
          const simd& __v = *this;
          return _Rp([&](auto __i) {
                   return __v[__idx[__i]];
                 });
        }

      // P2664::end
      ///////////////////////
    };

  template <typename _Tp, typename _Abi>
    struct is_simd<simd<_Tp, _Abi>> : is_default_constructible<simd<_Tp, _Abi>>
    {};

  template <class _Tp, size_t _Extend>
    simd(std::span<_Tp, _Extend>) -> simd<_Tp, simd_abi::deduce_t<_Tp, _Extend>>;

  template <std::ranges::contiguous_range _Rg>
    simd(const _Rg& x)
    -> simd<std::ranges::range_value_t<_Rg>,
            simd_abi::deduce_t<std::ranges::range_value_t<_Rg>,
                               __detail::__static_range_size<_Rg>>>;

  template <typename _Tp, typename _Abi>
    simd(std::simd_mask<_Tp, _Abi>) -> simd<_Tp, _Abi>;

  template <__detail::__vectorizable _Tp, __detail::__simd_type _Simd>
    requires requires { typename simd_abi::deduce_t<_Tp, _Simd::size()>; }
    struct rebind_simd<_Tp, _Simd>
    { using type = simd<_Tp, simd_abi::deduce_t<_Tp, _Simd::size()>>; };

  template <__detail::__vectorizable _Tp, __detail::__mask_type _Mask>
    requires requires { typename simd_abi::deduce_t<_Tp, _Mask::size()>; }
    struct rebind_simd<_Tp, _Mask>
    { using type = simd_mask<_Tp, simd_abi::deduce_t<_Tp, _Mask::size()>>; };

  template <size_t _Np, __detail::__simd_type _Simd>
    requires (_Np != _Simd::size())
      && requires { typename simd_abi::deduce_t<typename _Simd::value_type, _Np>; }
    struct resize_simd<_Np, _Simd>
    { using type = __detail::__deduced_simd<typename _Simd::value_type, _Np>; };

  template <size_t _Np, __detail::__simd_type<_Np> _Simd>
    struct resize_simd<_Np, _Simd>
    { using type = _Simd; };

  template <size_t _Np, __detail::__mask_type _Mask>
    requires (_Np != _Mask::size())
      && requires { typename simd_abi::deduce_t<typename _Mask::simd_type::value_type, _Np>; }
    struct resize_simd<_Np, _Mask>
    { using type = __detail::__deduced_simd_mask<typename _Mask::simd_type::value_type, _Np>; };

  template <size_t _Np, __detail::__mask_type<_Np> _Mask>
    struct resize_simd<_Np, _Mask>
    { using type = _Mask; };

}

#endif  // PROTOTYPE_SIMD2_H_
