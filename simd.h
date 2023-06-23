/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD2_H_
#define PROTOTYPE_SIMD2_H_

#include "detail.h"
#include "simd_mask.h"
#include "flags.h"
#include <span>
#include <iterator>

namespace std
{
  template <typename _Tp, typename _Abi>
    class basic_simd : protected __detail::simd<_Tp, _Abi>
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

      using mask_type = std::basic_simd_mask<sizeof(_Tp), _Abi>;

      static inline constexpr __detail::_Ic<__detail::_SimdSizeType(_Base::size())> size = {};

      constexpr
      basic_simd() = default;

/*      constexpr
      basic_simd(const basic_simd&) = default;

      constexpr
      basic_simd(basic_simd&&) = default;*/

      constexpr
      basic_simd(const _Base& b)
      : _Base(b)
      {}

      // implicit broadcast constructor
      template <__detail::__broadcast_constructible<value_type> _Up>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_simd(_Up&& __x) noexcept
        : _Base(static_cast<_Tp>(__x))
        {}

      // type conversion constructor
      template <typename _Up, typename _UAbi>
        requires(__detail::simd_size_v<_Up, _UAbi> == size() and std::constructible_from<_Tp, _Up>)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        explicit(not __detail::__value_preserving_convertible_to<_Up, value_type>
                   || __detail::__higher_rank_than<_Up, value_type>)
        basic_simd(const basic_simd<_Up, _UAbi>& __x) noexcept
        : _Base(__detail::static_simd_cast<_Base>(__x))
        {}

      // generator constructor
      template <__detail::__simd_broadcast_invokable<value_type, size()> _Fp>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_simd(_Fp&& __gen) noexcept
        : _Base(std::forward<_Fp>(__gen))
        {}

      // load constructor
      template <typename _It, typename _Flags = __detail::element_aligned_tag>
        requires __detail::__vectorizable<std::iter_value_t<_It>>
                   and std::convertible_to<std::iter_value_t<_It>, value_type>
                   and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_simd(_It __first, _Flags __f = {})
        : _Base(std::addressof(*__first), __f)
        {}

      template <typename _It, typename _Flags = __detail::element_aligned_tag>
        requires __detail::__vectorizable<std::iter_value_t<_It>>
                   and std::convertible_to<std::iter_value_t<_It>, value_type>
                   and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_simd(_It __first, const mask_type& __k, _Flags __f = {})
        : _Base()
        { __detail::where(__k, *this).copy_from(std::addressof(*__first), __f); }

      // private init
      _GLIBCXX_SIMD_INTRINSIC constexpr
      basic_simd(__detail::_PrivateInit, const _MemberType& __init)
      : _Base(__detail::__private_init, __init)
      {}

      // loads and stores
      template <std::contiguous_iterator _It, typename _Flags = __detail::element_aligned_tag>
        requires __detail::__vectorizable<std::iter_value_t<_It>>
                   and std::convertible_to<std::iter_value_t<_It>, value_type>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(_It __first, _Flags __f = {})
        { _Base::copy_from(std::addressof(*__first), __f); }

      template <std::contiguous_iterator _It, typename _Flags = __detail::element_aligned_tag>
        requires __detail::__vectorizable<std::iter_value_t<_It>>
                   and std::convertible_to<std::iter_value_t<_It>, value_type>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(_It __first, const mask_type& __k, _Flags __f = {})
        { __detail::where(__k, *this).copy_from(std::addressof(*__first), __f); }

      template <std::contiguous_iterator _It, typename _Flags = __detail::element_aligned_tag>
        requires std::output_iterator<_It, _Tp> && __detail::__vectorizable<std::iter_value_t<_It>>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_It __first, _Flags __f = {}) const
        { _Base::copy_to(std::addressof(*__first), __f); }

      template <std::contiguous_iterator _It, typename _Flags = __detail::element_aligned_tag>
        requires std::output_iterator<_It, _Tp> && __detail::__vectorizable<std::iter_value_t<_It>>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_It __first, const mask_type& __k, _Flags __f = {}) const
        { __detail::where(__k, *this).copy_to(std::addressof(*__first), __f); }

      // unary operators (for any _Tp)
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr mask_type
        operator!() const
        { return {__detail::__private_init, _Impl::_S_negate(__data(*this))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd
        operator+() const
        { return *this; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd
        operator-() const
        { return {__detail::__private_init, _Impl::_S_unary_minus(__data(*this))}; }

      // compound assignment [basic_simd.cassign]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator+=(basic_simd& __lhs, const basic_simd& __x)
      { return __lhs = __lhs + __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator-=(basic_simd& __lhs, const basic_simd& __x)
      { return __lhs = __lhs - __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator*=(basic_simd& __lhs, const basic_simd& __x)
      { return __lhs = __lhs * __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator/=(basic_simd& __lhs, const basic_simd& __x)
      { return __lhs = __lhs / __x; }

      // binary operators [basic_simd.binary]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator+(const basic_simd& __x, const basic_simd& __y)
      { return {__detail::__private_init, _Impl::_S_plus(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator-(const basic_simd& __x, const basic_simd& __y)
      { return {__detail::__private_init, _Impl::_S_minus(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator*(const basic_simd& __x, const basic_simd& __y)
      { return {__detail::__private_init, _Impl::_S_multiplies(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator/(const basic_simd& __x, const basic_simd& __y)
      { return {__detail::__private_init, _Impl::_S_divides(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator%(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a % __a; }
      { return {__detail::__private_init, _Impl::_S_modulus(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator&(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a & __a; }
      { return {__detail::__private_init, _Impl::_S_bit_and(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator|(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a | __a; }
      { return {__detail::__private_init, _Impl::_S_bit_or(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator^(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a ^ __a; }
      { return {__detail::__private_init, _Impl::_S_bit_xor(__data(__x), __data(__y))}; }

      // compares [basic_simd.comparison]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator==(const basic_simd& __x, const basic_simd& __y)
      { return {__detail::__private_init, _Impl::_S_equal_to(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator!=(const basic_simd& __x, const basic_simd& __y)
      { return {__detail::__private_init, _Impl::_S_not_equal_to(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator<(const basic_simd& __x, const basic_simd& __y)
      { return {__detail::__private_init, _Impl::_S_less(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator<=(const basic_simd& __x, const basic_simd& __y)
      { return {__detail::__private_init, _Impl::_S_less_equal(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator>(const basic_simd& __x, const basic_simd& __y)
      { return {__detail::__private_init, _Impl::_S_less(__data(__y), __data(__x))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator>=(const basic_simd& __x, const basic_simd& __y)
      { return {__detail::__private_init, _Impl::_S_less_equal(__data(__y), __data(__x))}; }

      // construction from span is simple
      constexpr
      basic_simd(std::span<_Tp, size()> __mem)
      : _Base(__mem.data(), __detail::element_aligned)
      {}

      // ranges typically don't have a static size() function :(
      // but if one does, this ctor is useful
      template <std::ranges::contiguous_range _Rg>
        requires (std::same_as<std::ranges::range_value_t<_Rg>, _Tp>
                    && __detail::__static_range_size<_Rg> == size())
        constexpr
        basic_simd(const _Rg& __range)
        : _Base(std::ranges::data(__range), __detail::element_aligned)
        {}

      template <std::ranges::random_access_range _Rg>
        requires(std::convertible_to<std::ranges::range_value_t<_Rg>, _Tp>)
        constexpr explicit (not std::same_as<std::ranges::range_value_t<_Rg>, _Tp>)
        basic_simd(const _Rg& __range)
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
#define conditional_operator_impl operator?:
#endif

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd
      conditional_operator_impl(const mask_type& __k, const basic_simd& __t, const basic_simd& __f)
      {
        auto __ret = __f;
        _Impl::_S_masked_assign(__data(__k), __data(__ret), __data(__t));
        return __ret;
      }

#ifdef __GXX_CONDITIONAL_IS_OVERLOADABLE__
#undef conditional_operator_impl
#endif

      ///////////////////////
      // P2664::begin

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr reference
      operator[](__detail::_SimdSizeType __i) &
      { return _Base::operator[](__i); }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr value_type
      operator[](__detail::_SimdSizeType __i) const&
      { return _Base::operator[](__i); }

      template <std::integral _Up, typename _Ap>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr simd<_Tp, simd_size_v<_Up, _Ap>>
        operator[](basic_simd<_Up, _Ap> const& __idx) const noexcept
        {
          using _Rp = simd<_Tp, simd_size_v<_Up, _Ap>>;
          const basic_simd& __v = *this;
          return _Rp([&](auto __i) {
                   return __v[__idx[__i]];
                 });
        }

      // P2664::end
      ///////////////////////

      constexpr const auto& _M_data() const
      { return __data(static_cast<const _Base&>(*this)); }

      constexpr auto& _M_data()
      { return __data(static_cast<_Base&>(*this)); }

      friend constexpr const auto& __data(const basic_simd& __x)
      { return __x._M_data(); }

      friend constexpr auto& __data(basic_simd& __x)
      { return __x._M_data(); }
    };

  template <typename _Tp, typename _Abi>
    struct is_simd<basic_simd<_Tp, _Abi>>
    : is_default_constructible<basic_simd<_Tp, _Abi>>
    {};

  template <class _Tp, size_t _Extent>
    basic_simd(std::span<_Tp, _Extent>) -> basic_simd<_Tp, __detail::__deduce_t<_Tp, _Extent>>;

  template <std::ranges::contiguous_range _Rg>
    basic_simd(const _Rg& x)
    -> basic_simd<std::ranges::range_value_t<_Rg>,
                  __detail::__deduce_t<std::ranges::range_value_t<_Rg>,
                                       __detail::__static_range_size<_Rg>>>;

  template <size_t _Bs, typename _Abi>
    basic_simd(std::basic_simd_mask<_Bs, _Abi>)
    -> basic_simd<__detail::__int_with_sizeof_t<_Bs>, _Abi>;

  template <__detail::__vectorizable _Tp, __detail::__simd_type _Simd>
    struct rebind_simd<_Tp, _Simd>
    { using type = simd<_Tp, _Simd::size()>; };

  template <__detail::__vectorizable _Tp, __detail::__mask_type _Mask>
    struct rebind_simd<_Tp, _Mask>
    { using type = simd_mask<_Tp, _Mask::size()>; };

  template <__detail::_SimdSizeType _Np, __detail::__simd_type _Simd>
    struct resize_simd<_Np, _Simd>
    { using type = simd<typename _Simd::value_type, _Np>; };

  // FIXME: resize_simd_t<1, simd_mask<long double, 1>> turns out as a different type
  template <__detail::_SimdSizeType _Np, __detail::__mask_type _Mask>
    struct resize_simd<_Np, _Mask>
    { using type = simd_mask<typename decltype(+_Mask())::value_type, _Np>; };
}

#endif  // PROTOTYPE_SIMD2_H_
// vim: et ts=8 sw=2 tw=100 cc=101
