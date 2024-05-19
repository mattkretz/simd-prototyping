/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD2_H_
#define PROTOTYPE_SIMD2_H_

#include "detail.h"
#include "simd_mask.h"
#include "flags.h"
#include "simd_iterator.h"
#include <span>
#include <iterator>

namespace std
{
  // not supported:
  // - deleted: dctor, dtor, cctor, cassign
  // - no members except value_type, abi_type, and mask_type
  template <typename _Tp, typename _Abi>
    requires (__detail::_SimdTraits<_Tp, _Abi>::_S_size == 0)
    class basic_simd<_Tp, _Abi>
    {
    public:
      using value_type = _Tp;

      using abi_type = _Abi;

      using mask_type = std::basic_simd_mask<
                          sizeof(conditional_t<is_void_v<_Tp>, int, _Tp>), _Abi>;

      basic_simd() = delete;

      ~basic_simd() = delete;

      basic_simd(const basic_simd&) = delete;

      basic_simd& operator=(const basic_simd&) = delete;
    };

  // --------------------------------------------------------------
  // supported
  template <typename _Tp, typename _Abi>
    class basic_simd
    {
      static_assert(__detail::__vectorizable<_Tp> and __detail::__valid_abi_tag<_Abi, _Tp>);

      using _Traits = __detail::_SimdTraits<_Tp, _Abi>;

      using _MemberType = typename _Traits::_SimdMember;

      static constexpr _Tp* _S_type_tag = nullptr;

      alignas(_Traits::_S_simd_align) _MemberType _M_data;

    public:
      using _Impl = typename _Traits::_SimdImpl;

      using reference = __detail::_SmartReference<_MemberType, _Impl, _Tp>;

      using value_type = _Tp;

      using abi_type = _Abi;

      using mask_type = std::basic_simd_mask<sizeof(_Tp), _Abi>;

      static constexpr auto size = __detail::__ic<_Traits::_S_size>;

      using iterator = __simd_iterator<_Tp, _Abi>;

      //static_assert(std::random_access_iterator<iterator>);
      //static_assert(std::sentinel_for<__simd_iterator_sentinel, iterator>);

      constexpr iterator
      begin() const
      { return iterator(*this, 0); }

      constexpr __simd_iterator_sentinel
      end() const
      { return {}; }

      constexpr
      basic_simd() = default;

      // ABI-specific conversions
      template <typename _Up>
        requires requires { _Traits::template _S_simd_conversion<_Up>(_M_data); }
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        operator _Up() const
        { return _Traits::template _S_simd_conversion<_Up>(_M_data); }

      template <typename _Up>
        requires (_Traits::template _S_is_simd_ctor_arg<_Up>)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_simd(_Up __x)
        : _M_data(_Traits::_S_simd_construction(__x))
        {}

      // implicit broadcast constructor
      template <__detail::__broadcast_constructible<value_type> _Up>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_simd(_Up&& __x) noexcept
        : _M_data(_Impl::_S_broadcast(static_cast<_Tp>(__x)))
        {}

      // type conversion constructor
      template <typename _Up, typename _UAbi>
        requires(simd_size_v<_Up, _UAbi> == size() and std::constructible_from<_Tp, _Up>)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        explicit(not __detail::__value_preserving_convertible_to<_Up, value_type>
                   || __detail::__higher_rank_than<_Up, value_type>)
        basic_simd(const basic_simd<_Up, _UAbi>& __x) noexcept
        : _M_data(__detail::_SimdConverter<_Up, _UAbi, _Tp, _Abi>()(__data(__x)))
        {}

      // generator constructor
      template <__detail::__simd_broadcast_invokable<value_type, size()> _Fp>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_simd(_Fp&& __gen) noexcept
        : _M_data(_Impl::template _S_generator<value_type>(static_cast<_Fp&&>(__gen)))
        {}

      // ranges typically don't have a static size
      // but if one does, this ctor is useful
      template <std::ranges::contiguous_range _Rg, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                      value_type, _Flags...>
          and (__detail::__static_range_size<_Rg> == size.value)
        constexpr // implicit!
        basic_simd(_Rg&& __range, simd_flags<_Flags...> __flags = {})
        : _M_data(_Impl::_S_load(__flags.template _S_adjust_pointer<basic_simd>(
                                   std::ranges::data(__range)), _S_type_tag))
        {}

      template <std::ranges::contiguous_range _Rg, typename... _Flags>
        requires std::ranges::output_range<_Rg, value_type>
          and __detail::__loadstore_convertible_to<
                value_type, std::ranges::range_value_t<_Rg>, _Flags...>
          and (__detail::__static_range_size<_Rg> == std::dynamic_extent
                 or __detail::__static_range_size<_Rg> == size.value)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_Rg&& __range, simd_flags<_Flags...> __f = {})
        {
          const auto* __ptr = __f.template _S_adjust_pointer<basic_simd>(std::ranges::data(__range));
          if constexpr (__detail::__static_range_size<_Rg> == std::dynamic_extent)
            {
              if(std::ranges::size(__range) >= size())
                _Impl::_S_store(_M_data, __ptr, _S_type_tag);
              else
                __builtin_memcpy(__ptr, &_M_data, size() - std::ranges::size(__range));
            }
          else
            _Impl::_S_store(_M_data, __ptr, _S_type_tag);
        }


      // end exploration
      ////////////////////////////////////////////////////////////////////////////////////////////

      // private init
      _GLIBCXX_SIMD_INTRINSIC constexpr
      basic_simd(__detail::_PrivateInit, const _MemberType& __init)
      : _M_data(__init)
      {}

      // stores
      template <std::contiguous_iterator _It, typename... _Flags>
        requires std::output_iterator<_It, _Tp>
          and __detail::__loadstore_convertible_to<value_type, std::iter_value_t<_It>, _Flags...>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_It __first, simd_flags<_Flags...> __flags = {}) const
        {
          _Impl::_S_store(_M_data, __flags.template _S_adjust_pointer<basic_simd>(
                                     std::to_address(__first)), _S_type_tag);
        }

      template <std::contiguous_iterator _It, typename... _Flags>
        requires std::output_iterator<_It, _Tp>
          and __detail::__loadstore_convertible_to<value_type, std::iter_value_t<_It>, _Flags...>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_It __first, const mask_type& __k, simd_flags<_Flags...> __flags = {}) const
        {
          _Impl::_S_masked_store(
            __data(*this), __flags.template _S_adjust_pointer<basic_simd>(std::to_address(__first)),
            __data(__k));
        }

      // unary operators (for any _Tp)
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr mask_type
      operator!() const
      requires requires(value_type __a) { {!__a} -> same_as<bool>; }
      { return {__detail::__private_init, _Impl::_S_negate(__data(*this))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd
      operator+() const
      requires requires(value_type __a) { +__a; }
      { return *this; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd
      operator-() const
      requires requires(value_type __a) { -__a; }
      { return {__detail::__private_init, _Impl::_S_unary_minus(__data(*this))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd
      operator~() const
      requires requires(value_type __a) { ~__a; }
      { return {__detail::__private_init, _Impl::_S_complement(__data(*this))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd&
      operator++()
      requires requires(value_type __a) { ++__a; }
      {
        _Impl::_S_increment(_M_data);
        return *this;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd
      operator++(int)
      requires requires(value_type __a) { __a++; }
      {
        basic_simd __r = *this;
        _Impl::_S_increment(_M_data);
        return __r;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd&
      operator--()
      requires requires(value_type __a) { --__a; }
      {
        _Impl::_S_decrement(_M_data);
        return *this;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd
      operator--(int)
      requires requires(value_type __a) { __a--; }
      {
        basic_simd __r = *this;
        _Impl::_S_decrement(_M_data);
        return __r;
      }

      // compound assignment [basic_simd.cassign]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator+=(basic_simd& __lhs, const basic_simd& __x)
      requires requires(value_type __a, value_type __b) { __a + __b; }
      { return __lhs = __lhs + __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator-=(basic_simd& __lhs, const basic_simd& __x)
      requires requires(value_type __a, value_type __b) { __a - __b; }
      { return __lhs = __lhs - __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator*=(basic_simd& __lhs, const basic_simd& __x)
      requires requires(value_type __a, value_type __b) { __a * __b; }
      { return __lhs = __lhs * __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator/=(basic_simd& __lhs, const basic_simd& __x)
      requires requires(value_type __a, value_type __b) { __a / __b; }
      { return __lhs = __lhs / __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator%=(basic_simd& __lhs, const basic_simd& __x)
      requires requires(value_type __a, value_type __b) { __a % __b; }
      { return __lhs = __lhs % __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator&=(basic_simd& __lhs, const basic_simd& __x)
      requires requires(value_type __a, value_type __b) { __a & __b; }
      { return __lhs = __lhs & __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator|=(basic_simd& __lhs, const basic_simd& __x)
      requires requires(value_type __a, value_type __b) { __a | __b; }
      { return __lhs = __lhs | __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator^=(basic_simd& __lhs, const basic_simd& __x)
      requires requires(value_type __a, value_type __b) { __a ^ __b; }
      { return __lhs = __lhs ^ __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator<<=(basic_simd& __lhs, const basic_simd& __x)
      requires requires(value_type __a, value_type __b) { __a << __b; }
      { return __lhs = __lhs << __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator>>=(basic_simd& __lhs, const basic_simd& __x)
      requires requires(value_type __a, value_type __b) { __a >> __b; }
      { return __lhs = __lhs >> __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator<<=(basic_simd& __lhs, int __x)
      requires requires(value_type __a, int __b) { __a << __b; }
      { return __lhs = __lhs << __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator>>=(basic_simd& __lhs, int __x)
      requires requires(value_type __a, int __b) { __a >> __b; }
      { return __lhs = __lhs >> __x; }

      // binary operators [basic_simd.binary]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator+(const basic_simd& __x, const basic_simd& __y)
      requires requires(value_type __a) { __a + __a; }
      { return {__detail::__private_init, _Impl::_S_plus(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator-(const basic_simd& __x, const basic_simd& __y)
      requires requires(value_type __a) { __a - __a; }
      { return {__detail::__private_init, _Impl::_S_minus(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator*(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a * __a; }
      { return {__detail::__private_init, _Impl::_S_multiplies(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator/(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a / __a; }
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

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator>>(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a >> __a; }
      { return {__detail::__private_init, _Impl::_S_bit_shift_right(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator<<(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a << __a; }
      { return {__detail::__private_init, _Impl::_S_bit_shift_left(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator>>(const basic_simd& __x, int __y)
      requires requires (value_type __a, int __b) { __a >> __b; }
      { return {__detail::__private_init, _Impl::_S_bit_shift_right(__data(__x), __y)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator<<(const basic_simd& __x, int __y)
      requires requires (value_type __a, int __b) { __a << __b; }
      { return {__detail::__private_init, _Impl::_S_bit_shift_left(__data(__x), __y)}; }

      // compares [basic_simd.comparison]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator==(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a == __a; }
      { return {__detail::__private_init, _Impl::_S_equal_to(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator!=(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a != __a; }
      { return {__detail::__private_init, _Impl::_S_not_equal_to(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator<(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a < __a; }
      { return {__detail::__private_init, _Impl::_S_less(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator<=(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a <= __a; }
      { return {__detail::__private_init, _Impl::_S_less_equal(__data(__x), __data(__y))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator>(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a > __a; }
      { return {__detail::__private_init, _Impl::_S_less(__data(__y), __data(__x))}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type
      operator>=(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a >= __a; }
      { return {__detail::__private_init, _Impl::_S_less_equal(__data(__y), __data(__x))}; }

      constexpr std::array<_Tp, size()>
      to_array() const noexcept
      {
        std::array<_Tp, size()> __r = {};
        this->copy_to(__r.data(), simd_flag_default);
        return __r;
      }

      explicit
      operator std::array<_Tp, size()>() const noexcept
      { return to_array(); }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd
      simd_select_impl(const mask_type& __k, const basic_simd& __t, const basic_simd& __f)
      {
        auto __ret = __f;
        _Impl::_S_masked_assign(__data(__k), __ret._M_data, __t._M_data);
        return __ret;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr reference
      operator[](__detail::_SimdSizeType __i) &
      {
        if (__i >= size.value or __i < 0)
          __detail::__invoke_ub("Subscript %d is out of range [0, %d]", __i, size() - 1);
        return {_M_data, __i};
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr value_type
      operator[](__detail::_SimdSizeType __i) const&
      {
        if (__i >= size.value or __i < 0)
          __detail::__invoke_ub("Subscript %d is out of range [0, %d]", __i, size() - 1);
        return _Impl::_S_get(_M_data, __i);
      }

      ///////////////////////
      // P2664::begin

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

      friend constexpr const auto& __data(const basic_simd& __x)
      { return __x._M_data; }

      friend constexpr auto& __data(basic_simd& __x)
      { return __x._M_data; }

      _GLIBCXX_SIMD_INTRINSIC constexpr bool
      _M_is_constprop() const
      {
        if constexpr (requires {_Impl::_S_is_constprop(_M_data);})
          return _Impl::_S_is_constprop(_M_data);
        else if constexpr (requires {_M_data._M_is_constprop();})
          return _M_data._M_is_constprop();
        else
          return __builtin_constant_p(_M_data);
      }
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
    -> basic_simd<__detail::__mask_integer_from<_Bs>,
                  __detail::__simd_abi_for_mask_t<_Bs, _Abi>>;

  template <__detail::__vectorizable _Tp, __detail::__simd_type _Simd>
    struct rebind_simd<_Tp, _Simd>
    { using type = simd<_Tp, _Simd::size()>; };

  template <__detail::__vectorizable _Tp, __detail::__mask_type _Mask>
    struct rebind_simd<_Tp, _Mask>
    { using type = simd_mask<_Tp, _Mask::size()>; };

  template <__detail::_SimdSizeType _Np, __detail::__simd_type _Simd>
    struct resize_simd<_Np, _Simd>
    { using type = simd<typename _Simd::value_type, _Np>; };

  template <__detail::_SimdSizeType _Np, __detail::__mask_type _Mask>
    struct resize_simd<_Np, _Mask>
    { using type = simd_mask<typename decltype(+_Mask())::value_type, _Np>; };

  template <typename _Tp, typename _Abi, __detail::__vectorizable _Up>
    struct simd_alignment<basic_simd<_Tp, _Abi>, _Up>
    : std::integral_constant<size_t, alignof(rebind_simd_t<_Up, basic_simd<_Tp, _Abi>>)>
    {};

  template <size_t _Bs, typename _Abi>
    struct simd_alignment<basic_simd_mask<_Bs, _Abi>, bool>
    : std::integral_constant<size_t, alignof(simd<__detail::__make_unsigned_int_t<bool>,
                                                  basic_simd_mask<_Bs, _Abi>::size()>)>
    {};

  template <typename _Tp, typename _Rg>
    struct __simd_load_return;

  template <typename _Rg>
    struct __simd_load_return<void, _Rg>
    { using type = simd<ranges::range_value_t<_Rg>>; };

  template <__detail::__simd_type _Vp, typename _Rg>
    struct __simd_load_return<_Vp, _Rg>
    { using type = _Vp; };

  /*  template <__detail::__vectorizable _Tp, typename _Rg>
    struct __simd_load_return<_Tp, _Rg>
    { using type = simd<_Tp>; };*/

  template <typename _Tp, typename _Rg>
    using __simd_load_return_t = typename __simd_load_return<_Tp, _Rg>::type;

  [[__gnu__::__error__("simd_load reads beyond the end of the given range")]]
  void
  __error_simd_load_out_of_bounds();

  template <typename _Tp = void, ranges::contiguous_range _Rg, typename... _Flags>
    requires (not __detail::__vectorizable<_Tp>)
    constexpr __simd_load_return_t<_Tp, _Rg>
    simd_load(_Rg&& __range, simd_flags<_Flags...> __flags = {})
    {
      using _RV = __simd_load_return_t<_Tp, _Rg>;
      static_assert(__detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                         typename _RV::value_type, _Flags...>,
                    "The converting load is not value-preserving. "
                    "Pass 'std::simd_flag_convert' if lossy conversion matches the intent.");

      constexpr bool __allow_out_of_bounds
        = (false or ... or is_same_v<_Flags, __detail::_LoadDefaultInit>);
      static_assert(__detail::__static_range_size<_Rg> >= _RV::size.value
                      or __allow_out_of_bounds
                      or __detail::__static_range_size<_Rg> == dynamic_extent,
                    "Out-of-bounds access: load of %d values out of range of size %d");

      const auto* __ptr = __flags.template _S_adjust_pointer<_RV>(std::ranges::data(__range));
      using _Rp = typename _RV::value_type;
      constexpr _Rp* __type_tag = nullptr;

      const auto __rg_size = std::ranges::size(__range);
      const bool __out_of_bounds = __rg_size < _RV::size();
      if (not __allow_out_of_bounds and __builtin_constant_p(__out_of_bounds) and __out_of_bounds)
        __error_simd_load_out_of_bounds();

      if constexpr (__detail::__static_range_size<_Rg> != dynamic_extent
                      and __detail::__static_range_size<_Rg> >= _RV::size())
        return _RV(_RV::_Impl::_S_load(__ptr, __type_tag));
      else if (__rg_size >= _RV::size())
        return _RV(_RV::_Impl::_S_load(__ptr, __type_tag));
      else if (__allow_out_of_bounds and (__builtin_is_constant_evaluated()
                                            or __builtin_constant_p(__rg_size)))
        return _RV([&](size_t __i) {
                 return __i < __rg_size ? __range[__i] : _Rp();
               });
      else if (__allow_out_of_bounds)
        {
          _RV __ret {};
          __builtin_memcpy(&__data(__ret), __ptr, _RV::size() - __rg_size);
          return __ret;
        }
      else
        {
          _RV __ret;
#ifdef UB_LOADS
          __detail::__invoke_ub("Out-of-bounds access: load of %d values out of range of size %d",
                                _RV::size(), __rg_size);
#else
          // otherwise we get EB
          __builtin_memcpy(&__data(__ret), __ptr, _RV::size() - __rg_size);
#endif
          return __ret;
        }
    }

  template <typename _Tp = void, contiguous_iterator _First, sentinel_for<_First> _Last,
            typename... _Flags>
    constexpr auto
    simd_load(_First __first, _Last __last, simd_flags<_Flags...> __flags = {})
    { return simd_load<_Tp>(std::span(__first, __last), __flags); }

  template <typename _Tp = void, contiguous_iterator _First, typename... _Flags>
    constexpr auto
    simd_load(_First __first, size_t __size, simd_flags<_Flags...> __flags = {})
    { return simd_load<_Tp>(std::span(__first, __size), __flags); }

  // simd-generic loads
  template <__detail::__vectorizable _Tp, ranges::contiguous_range _Rg, typename... _Flags>
    constexpr _Tp
    simd_load(_Rg&& __range, simd_flags<_Flags...> __flags = {})
    {
      static_assert(__detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                         _Tp, _Flags...>,
                    "The converting load is not value-preserving. "
                    "Pass 'std::simd_flag_convert' if lossy conversion matches the intent.");

      constexpr bool __allow_out_of_bounds
        = (false or ... or is_same_v<_Flags, __detail::_LoadDefaultInit>);
      static_assert(__detail::__static_range_size<_Rg> > 0
                      or __allow_out_of_bounds
                      or __detail::__static_range_size<_Rg> == dynamic_extent,
                    "Out-of-bounds access: load of 1 value out of range of size 0");

      const auto* __ptr = __flags.template _S_adjust_pointer<_Tp>(std::ranges::data(__range));

      const auto __rg_size = std::ranges::size(__range);
      const bool __out_of_bounds = __rg_size == 0;
      if (not __allow_out_of_bounds and __builtin_constant_p(__out_of_bounds) and __out_of_bounds)
        __error_simd_load_out_of_bounds();

      if constexpr (__detail::__static_range_size<_Rg> != dynamic_extent
                      and __detail::__static_range_size<_Rg> > 0)
        return *__ptr;
      else if (__rg_size > 0)
        return *__ptr;
      else if (__allow_out_of_bounds)
        return _Tp();
      else
        {
          _Tp __ret;
#ifdef UB_LOADS
          __detail::__invoke_ub("Out-of-bounds access: load of 1 value out of range of size 0");
#else
          // otherwise we get EB
#endif
          return __ret;
        }
    }
}

#endif  // PROTOTYPE_SIMD2_H_
// vim: et ts=8 sw=2 tw=100 cc=101
