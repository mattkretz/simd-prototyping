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
#include "udt_simd.h"
#include <span>
#include <iterator>

namespace std
{
  // not supported:
  // - deleted: dctor, dtor, cctor, cassign
  // - no members except value_type, abi_type, and mask_type
  template <typename _Tp, typename _Abi>
    requires (__detail::_SimdTraits<_Tp, _Abi>::_S_size == 0)
      and (not __detail::__indirectly_vectorizable<_Tp>
             or __detail::_SimdTraits<std::representation_type_t<_Tp>, _Abi>::_S_size == 0)
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

      // load constructor
      template <typename _It, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::iter_value_t<_It>, value_type, _Flags...>
          and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr // TODO explicit
        basic_simd(_It __first, simd_flags<_Flags...> __flags = {})
        : _M_data(_Impl::_S_load(__flags.template _S_adjust_pointer<basic_simd>(
                                   std::to_address(__first)), _S_type_tag))
        {}

      template <typename _It, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::iter_value_t<_It>, value_type, _Flags...>
          and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr // TODO explicit
        basic_simd(_It __first, const mask_type& __k, simd_flags<_Flags...> __flags = {})
        : _M_data(_Impl::_S_masked_load(_MemberType(), __data(__k),
                                        __flags.template _S_adjust_pointer<basic_simd>(
                                          std::to_address(__first))))
        {}

      ////////////////////////////////////////////////////////////////////////////////////////////
      // begin exploration

      // construction from span of static extent is simple
      //   basic_simd(std::span<_Tp, size()> __mem)
      //
      // but let's add conversions and flags
      template <typename _Up, typename... _Flags>
        requires __detail::__loadstore_convertible_to<_Up, value_type, _Flags...>
        constexpr // TODO explicit?
        basic_simd(std::span<_Up, size()> __mem, simd_flags<_Flags...> __flags = {})
        : basic_simd(__mem.begin(), __flags)
        {}

      // ranges typically don't have a static size() function :(
      // but if one does, this ctor is useful
      template <std::ranges::contiguous_range _Rg, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                      value_type, _Flags...>
          and (__detail::__static_range_size<_Rg> == size.value)
        constexpr // TODO explicit?
        basic_simd(_Rg&& __range, simd_flags<_Flags...> __flags = {})
        : basic_simd(std::ranges::begin(__range), __flags)
        {}

      template <std::ranges::contiguous_range _Rg, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                      value_type, _Flags...>
          and (__detail::__static_range_size<_Rg> == std::dynamic_extent)
        constexpr // TODO explicit?
        basic_simd(_Rg&& __range, simd_flags<_Flags...> __f = {})
        {
          const auto* __ptr = __f.template _S_adjust_pointer<basic_simd>(std::ranges::data(__range));
          if(std::ranges::size(__range) >= size())
            _M_data = _Impl::_S_load(__ptr, _S_type_tag);
          else
            {
              *this = _Tp();
              __builtin_memcpy(&_M_data, __ptr, size() - std::ranges::size(__range));
            }
        }

      /* This constructor makes loads from C-arrays ambiguous because they are contiguous iterators
       * (decay to pointer) as well as contiguous ranges.
       *
      template <std::ranges::random_access_range _Rg>
        requires(std::convertible_to<std::ranges::range_value_t<_Rg>, _Tp>)
        constexpr explicit (not std::same_as<std::ranges::range_value_t<_Rg>, _Tp>)
        basic_simd(const _Rg& __range)
        : basic_simd([&__range](auto __i) -> _Tp { return __range[__i]; })
        {}*/

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

      // loads and stores
      template <std::contiguous_iterator _It, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::iter_value_t<_It>, value_type, _Flags...>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(_It __first, simd_flags<_Flags...> __flags = {})
        {
          _M_data = _Impl::_S_load(__flags.template _S_adjust_pointer<basic_simd>(
                                     std::to_address(__first)), _S_type_tag);
        }

      template <std::contiguous_iterator _It, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::iter_value_t<_It>, value_type, _Flags...>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(_It __first, const mask_type& __k, simd_flags<_Flags...> __flags = {})
        {
          _M_data = _Impl::_S_masked_load(_M_data, __data(__k),
                                          __flags.template _S_adjust_pointer<basic_simd>(
                                            std::to_address(__first)));
        }

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

  template <__detail::__indirectly_vectorizable _Tp, typename _Abi>
    class basic_simd<_Tp, _Abi>
    {
      using _Rep = std::representation_type_t<_Tp>;

      using _RepSimd = basic_simd<_Rep, _Abi>;

      _RepSimd _M_data;

    public:
      //using reference = __detail::_SmartReference<_MemberType, _Impl, _Tp>;

      using value_type = _Tp;

      using abi_type = _Abi;

      using mask_type = typename _RepSimd::mask_type;

      static constexpr auto size = _RepSimd::size;

      using iterator = __simd_iterator<_Tp, _Abi>;

      constexpr iterator
      begin() const
      { return iterator(*this, 0); }

      constexpr __simd_iterator_sentinel
      end() const
      { return {}; }

      constexpr explicit(not __detail::__convertible_from<_Tp>)
      basic_simd() requires std::is_default_constructible_v<_Tp> = default;

      // broadcast constructor
      // constructs a scalar _Tp from __args, turns it into the scalar representation and then
      // broadcasts into _M_data
      template <typename... _Args>
        requires (sizeof...(_Args) >= 1) and std::constructible_from<_Tp, _Args&&...>
        constexpr explicit(not __detail::__convertible_from<_Tp, _Args&&...>)
        basic_simd(_Args&&... __args)
        : _M_data(std::to_representation(_Tp(static_cast<_Args&&>(__args)...)))
        {}

      // type conversion constructor
      // either per-element _Up -> _Tp conversion or CPO optimization
      template <typename _Up, typename _UAbi>
        requires (basic_simd<_Up, _UAbi>::size() == size())
          and std::constructible_from<_Tp, _Up>
        constexpr
        explicit(not std::convertible_to<_Up, _Tp>)
        basic_simd(const basic_simd<_Up, _UAbi>& __x)
        : _M_data(std::construct_representation_from<_Tp>(__x))
        {}

      // generator constructor
      template <__detail::__simd_broadcast_invokable<value_type, size()> _Fp>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_simd(_Fp&& __gen) noexcept
        : _M_data([&](auto __i) { return std::to_representation(__gen(__i)); })
        {}

      // load constructor
      template <typename _It, typename... _Flags>
        requires std::same_as<std::iter_value_t<_It>, value_type>
          and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_simd(_It __first, simd_flags<_Flags...> __flags = {})
        { copy_from(__first, __flags); }

      template <typename _It, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::iter_value_t<_It>, value_type, _Flags...>
          and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_simd(_It __first, const mask_type& __k, simd_flags<_Flags...> __flags = {})
        : _M_data(_RepSimd(
                    [&](int __i) {
                      const auto* __ptr
                        = __flags.template _S_adjust_pointer<basic_simd>(std::to_address(__first));
                      return __k[__i] ? std::to_representation(__ptr[__i])
                                      : std::to_representation(value_type());
                    }))
        {}

#if 0
      ////////////////////////////////////////////////////////////////////////////////////////////
      // begin exploration

      // construction from span of static extent is simple
      //   basic_simd(std::span<_Tp, size()> __mem)
      //
      // but let's add conversions and flags
      template <typename _Up, typename... _Flags>
        requires __detail::__loadstore_convertible_to<_Up, value_type, _Flags...>
        constexpr // TODO explicit?
        basic_simd(std::span<_Up, size()> __mem, simd_flags<_Flags...> __flags = {})
        : basic_simd(__mem.begin(), __flags)
        {}

      // ranges typically don't have a static size() function :(
      // but if one does, this ctor is useful
      template <std::ranges::contiguous_range _Rg, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                      value_type, _Flags...>
          and (__detail::__static_range_size<_Rg> == size.value)
        constexpr // TODO explicit?
        basic_simd(_Rg&& __range, simd_flags<_Flags...> __flags = {})
        : basic_simd(std::ranges::begin(__range), __flags)
        {}

      template <std::ranges::contiguous_range _Rg, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                 value_type, _Flags...>
          and (__detail::__static_range_size<_Rg> == std::dynamic_extent)
        constexpr // TODO explicit?
        basic_simd(_Rg&& __range, simd_flags<_Flags...> __f = {})
        {
          const auto* __ptr = __f.template _S_adjust_pointer<basic_simd>(std::ranges::data(__range));
          if(std::ranges::size(__range) >= size())
            _M_data = _Impl::_S_load(__ptr, _S_type_tag);
          else
            {
              *this = _Tp();
              __builtin_memcpy(&_M_data, __ptr, size() - std::ranges::size(__range));
            }
        }

      /* This constructor makes loads from C-arrays ambiguous because they are contiguous iterators
       * (decay to pointer) as well as contiguous ranges.
       *
      template <std::ranges::random_access_range _Rg>
        requires(std::convertible_to<std::ranges::range_value_t<_Rg>, _Tp>)
        constexpr explicit (not std::same_as<std::ranges::range_value_t<_Rg>, _Tp>)
        basic_simd(const _Rg& __range)
        : basic_simd([&__range](auto __i) -> _Tp { return __range[__i]; })
        {}*/

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
#endif

      // private init
      _GLIBCXX_SIMD_INTRINSIC constexpr
      basic_simd(__detail::_PrivateInit, const _RepSimd& __init)
      : _M_data(__init)
      {}

      // loads and stores
      template <std::contiguous_iterator _It, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::iter_value_t<_It>, value_type, _Flags...>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(_It __first, simd_flags<_Flags...> __flags = {})
        {
          const auto* __ptr
            = __flags.template _S_adjust_pointer<basic_simd>(std::to_address(__first));
          static_assert(sizeof(_M_data) == sizeof(value_type) * size());
          if (__builtin_is_constant_evaluated())
            _M_data = _RepSimd([&](int __i) {
                        return std::to_representation(__ptr[__i]);
                      });
          else
            __builtin_memcpy(&_M_data, __ptr, sizeof(_M_data));
        }

      template <std::contiguous_iterator _It, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::iter_value_t<_It>, value_type, _Flags...>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(_It __first, const mask_type& __k, simd_flags<_Flags...> __flags = {})
        {
          _M_data = _RepSimd(
                      [&](int __i) {
                        const auto* __ptr
                          = __flags.template _S_adjust_pointer<basic_simd>(std::to_address(__first));
                        return __k[__i] ? std::to_representation(__ptr[__i])
                                        : std::to_representation(value_type());
                      });
        }

      template <std::contiguous_iterator _It, typename... _Flags>
        requires std::output_iterator<_It, _Tp>
          and __detail::__loadstore_convertible_to<value_type, std::iter_value_t<_It>, _Flags...>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_It __first, simd_flags<_Flags...> __flags = {}) const
        {
          auto* __ptr = __flags.template _S_adjust_pointer<basic_simd>(std::to_address(__first));
          static_assert(sizeof(_M_data) == sizeof(value_type) * size());
          if (__builtin_is_constant_evaluated())
            {
              for (int __i = 0; __i < size(); ++__i)
                __ptr[__i] = std::from_representation<_Tp>(_M_data[__i]);
            }
          else
            __builtin_memcpy(__ptr, &_M_data, sizeof(_M_data));
        }

      template <std::contiguous_iterator _It, typename... _Flags>
        requires std::output_iterator<_It, _Tp>
          and __detail::__loadstore_convertible_to<value_type, std::iter_value_t<_It>, _Flags...>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_It __first, const mask_type& __k, simd_flags<_Flags...> __flags = {}) const
        {
          // TODO
        }

      // unary operators (for any _Tp)
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr mask_type
      operator!() const
      requires requires(value_type __a) { !__a; }
      {
        return mask_type([&](int __i) {
                 return !from_representation<value_type>(_M_data[__i]);
               });
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr auto
      operator+() const
      requires requires(value_type __a) { {+__a} -> __detail::__any_vectorizable; }
      {
        using _RT = std::remove_cvref_t<decltype(+declval<value_type>())>;
        return simd<_RT, size()>([&](int __i) {
                 return +from_representation<value_type>(_M_data[__i]);
               });
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr auto
      operator-() const
      requires requires(value_type __a) { {-__a} -> __detail::__any_vectorizable; }
      {
        using _RT = std::remove_cvref_t<decltype(-declval<value_type>())>;
        return simd<_RT, size()>([&](int __i) {
                 return -from_representation<value_type>(_M_data[__i]);
               });
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr auto
      operator~() const
      requires requires(value_type __a) { {~__a} -> __detail::__any_vectorizable; }
      {
        using _RT = std::remove_cvref_t<decltype(~declval<value_type>())>;
        return simd<_RT, size()>([&](int __i) {
                 return ~from_representation<value_type>(_M_data[__i]);
               });
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd&
      operator++()
      requires requires(value_type __a) { {++__a} -> same_as<value_type&>; }
      {
        for (int __i = 0; __i < size(); ++__i)
          _M_data[__i] = ++from_representation<value_type>(_M_data[__i]);
        return *this;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd
      operator++(int)
      requires requires(value_type __a) { {__a++} -> same_as<value_type>; }
      {
        basic_simd __r = *this;
        ++*this;
        return __r;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd&
      operator--()
      requires requires(value_type __a) { {--__a} -> same_as<value_type&>; }
      {
        for (int __i = 0; __i < size(); ++__i)
          _M_data[__i] = --from_representation<value_type>(_M_data[__i]);
        return *this;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_simd
      operator--(int)
      requires requires(value_type __a) { {__a--} -> same_as<value_type>; }
      {
        basic_simd __r = *this;
        --*this;
        return __r;
      }

      // compound assignment [basic_simd.cassign]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator+=(basic_simd& __lhs, const auto& __x)
      requires requires { {__lhs + __x} -> convertible_to<basic_simd>; }
      { return __lhs = __lhs + __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator-=(basic_simd& __lhs, const auto& __x)
      requires requires { {__lhs - __x} -> convertible_to<basic_simd>; }
      { return __lhs = __lhs - __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator*=(basic_simd& __lhs, const auto& __x)
      requires requires { {__lhs * __x} -> convertible_to<basic_simd>; }
      { return __lhs = __lhs * __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator/=(basic_simd& __lhs, const auto& __x)
      requires requires { {__lhs / __x} -> convertible_to<basic_simd>; }
      { return __lhs = __lhs / __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator%=(basic_simd& __lhs, const auto& __x)
      requires requires { {__lhs % __x} -> convertible_to<basic_simd>; }
      { return __lhs = __lhs % __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator&=(basic_simd& __lhs, const auto& __x)
      requires requires { {__lhs & __x} -> convertible_to<basic_simd>; }
      { return __lhs = __lhs & __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator|=(basic_simd& __lhs, const auto& __x)
      requires requires { {__lhs | __x} -> convertible_to<basic_simd>; }
      { return __lhs = __lhs | __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator^=(basic_simd& __lhs, const auto& __x)
      requires requires { {__lhs ^ __x} -> convertible_to<basic_simd>; }
      { return __lhs = __lhs ^ __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator<<=(basic_simd& __lhs, const auto& __x)
      requires requires { {__lhs << __x} -> convertible_to<basic_simd>; }
      { return __lhs = __lhs << __x; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd&
      operator>>=(basic_simd& __lhs, const auto& __x)
      requires requires { {__lhs >> __x} -> convertible_to<basic_simd>; }
      { return __lhs = __lhs >> __x; }

      // binary operators [basic_simd.binary]
#define _GLIBCXX_SIMD_BINARY_OP(op)                                                                \
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend auto                                            \
      operator op(const basic_simd& __x, const basic_simd& __y)                                    \
      requires requires(value_type __a) { {__a op __a} -> __detail::__any_vectorizable; }          \
      {                                                                                            \
        using _RT = std::remove_cvref_t<decltype(__x[0] op __y[0])>;                               \
        return simd<_RT, size()>([&](int __i) { return __x[__i] op __y[__i]; });                   \
      }                                                                                            \
                                                                                                   \
      template <__detail::__simd_type _Rhs>                                                        \
        requires (not convertible_to<_Rhs, basic_simd> and size() == _Rhs::size())                 \
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend auto                                          \
        operator op(const basic_simd& __x, const _Rhs& __y)                                        \
        requires requires(value_type __a, typename _Rhs::value_type __b)                           \
          { {__a op __b} -> __detail::__any_vectorizable; }                                        \
        {                                                                                          \
          using _RT = std::remove_cvref_t<decltype(__x[0] op __y[0])>;                             \
          return simd<_RT, size()>([&](int __i) { return __x[__i] op __y[__i]; });                 \
        }                                                                                          \
                                                                                                   \
      template <typename _Rhs>                                                                     \
        requires (not __detail::__simd_type<_Rhs> and not convertible_to<_Rhs, basic_simd>)        \
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend auto                                          \
        operator op(const basic_simd& __x, const _Rhs& __y)                                        \
        requires requires(value_type __a)                                                          \
          { {__a op __y} -> __detail::__any_vectorizable; }                                        \
        {                                                                                          \
          using _RT = std::remove_cvref_t<decltype(__x[0] op __y)>;                                \
          return simd<_RT, size()>([&](int __i) { return __x[__i] op __y; });                      \
        }                                                                                          \
                                                                                                   \
      template <typename _Lhs>                                                                     \
        requires (not __detail::__simd_type<_Lhs> and not convertible_to<_Lhs, basic_simd>)        \
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend auto                                          \
        operator op(const _Lhs& __x, const basic_simd& __y)                                        \
        requires requires(value_type __a)                                                          \
          { {__x op __a} -> __detail::__any_vectorizable; }                                        \
        {                                                                                          \
          using _RT = std::remove_cvref_t<decltype(__x op __y[0])>;                                \
          return simd<_RT, size()>([&](int __i) { return __x op __y[__i]; });                      \
        }

      _GLIBCXX_SIMD_ALL_BINARY(_GLIBCXX_SIMD_BINARY_OP);
      _GLIBCXX_SIMD_ALL_SHIFTS(_GLIBCXX_SIMD_BINARY_OP);
      _GLIBCXX_SIMD_ALL_ARITHMETICS(_GLIBCXX_SIMD_BINARY_OP);
#undef _GLIBCXX_SIMD_BINARY_OP

      // compares [basic_simd.comparison]
#define _GLIBCXX_SIMD_COMPARE_OP(op)                                                               \
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend mask_type                                       \
      operator op(const basic_simd& __x, const basic_simd& __y)                                    \
      requires requires (value_type __a) { {__a op __a} -> same_as<bool>; }                        \
      { return mask_type([&](int __i) { return __x[__i] op __y[__i]; }); }

      _GLIBCXX_SIMD_ALL_COMPARES(_GLIBCXX_SIMD_COMPARE_OP);
#undef _GLIBCXX_SIMD_COMPARE_OP

      constexpr std::array<value_type, size()>
      to_array() const noexcept
      {
        std::array<value_type, size()> __r = {};
        this->copy_to(__r.data(), simd_flag_default);
        return __r;
      }

      explicit
      operator std::array<_Tp, size()>() const noexcept
      { return to_array(); }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd
      simd_select_impl(const mask_type& __k, const basic_simd& __t, const basic_simd& __f)
      { return {__detail::__private_init, simd_select_impl(__k, __t._M_data, __f._M_data)}; }

#if 0
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr reference
      operator[](__detail::_SimdSizeType __i) &
      {
        if (__i >= size.value or __i < 0)
          __detail::__invoke_ub("Subscript %d is out of range [0, %d]", __i, size() - 1);
        return {_M_data, __i};
      }
#endif

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr value_type
      operator[](__detail::_SimdSizeType __i) const&
      { return std::from_representation<value_type>(_M_data[__i]); }

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
      { return __data(__x._M_data); }

      friend constexpr auto& __data(basic_simd& __x)
      { return __data(__x._M_data); }

      _GLIBCXX_SIMD_INTRINSIC constexpr bool
      _M_is_constprop() const
      { return _M_data._M_is_constprop(); }
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
}

#endif  // PROTOTYPE_SIMD2_H_
// vim: et ts=8 sw=2 tw=100 cc=101
