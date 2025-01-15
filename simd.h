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
  namespace __detail
  {
    template <typename _Tp, typename _Rg>
      struct __simd_load_return;

    template <typename _Rg>
      struct __simd_load_return<void, _Rg>
      { using type = simd<ranges::range_value_t<_Rg>>; };

    template <typename _Tp, typename _Abi, typename _Rg>
      struct __simd_load_return<basic_simd<_Tp, _Abi>, _Rg>
      { using type = basic_simd<_Tp, _Abi>; };

    template <typename _Tp, typename _Rg>
      using __simd_load_return_t = typename __simd_load_return<_Tp, _Rg>::type;
  }

  template <typename _Tp = void, ranges::contiguous_range _Rg, typename... _Flags>
    requires (not __detail::__vectorizable<_Tp>)
      and (not __detail::__loadstore_convertible_to<
                 std::ranges::range_value_t<_Rg>,
                 typename __detail::__simd_load_return_t<_Tp, _Rg>::value_type, _Flags...>)
    constexpr void
    load(_Rg&& __range, flags<_Flags...> __flags = {})
      = _GLIBCXX_DELETE_MSG(
          "The converting load is not value-preserving. "
          "Pass 'std::simd::flag_convert' if lossy conversion matches the intent.");

  template <typename _Tp = void, ranges::contiguous_range _Rg, typename... _Flags>
    requires (not __detail::__vectorizable<_Tp>)
      and __detail::__loadstore_convertible_to<
            std::ranges::range_value_t<_Rg>,
            typename __detail::__simd_load_return_t<_Tp, _Rg>::value_type, _Flags...>
    _GLIBCXX_SIMD_INTRINSIC
    constexpr __detail::__simd_load_return_t<_Tp, _Rg>
    load(_Rg&& __range, flags<_Flags...> __flags = {})
    {
      using _RV = __detail::__simd_load_return_t<_Tp, _Rg>;

      constexpr bool __throw_on_out_of_bounds = (is_same_v<_Flags, __detail::_Throw> or ...);
      constexpr bool __allow_out_of_bounds
        = (__throw_on_out_of_bounds or ... or is_same_v<_Flags, __detail::_LoadDefaultInit>);
      static_assert(__detail::__static_range_size<_Rg> >= _RV::size.value
                      or __allow_out_of_bounds
                      or __detail::__static_range_size<_Rg> == dynamic_extent,
                    "Out-of-bounds access: load of %d values out of range of size %d");

      const auto* __ptr = __flags.template _S_adjust_pointer<_RV>(std::ranges::data(__range));
      using _Rp = typename _RV::value_type;
      constexpr _Rp* __type_tag = nullptr;

      const auto __rg_size = std::ranges::size(__range);
      if constexpr (not __allow_out_of_bounds)
        __glibcxx_simd_precondition(std::ranges::size(__range) >= _RV::size(),
                                    "Input range is too small. "
                                    "Consider passing 'std::simd::flag_default_init'.");

      if constexpr (__throw_on_out_of_bounds)
        {
          if (__rg_size < _RV::size())
            throw std::out_of_range("std::simd::load Input range is too small.");
        }
#ifdef __AVX512F__
      if constexpr (__allow_out_of_bounds)
        {
          const typename _RV::mask_type __k([&](unsigned __i) { return __i < __rg_size; });
          _RV __ret {};
          _RV::_Impl::_S_masked_load(__data(__ret), __data(__k), __ptr);
          return __ret;
        }
#endif

      if constexpr (__detail::__static_range_size<_Rg> != dynamic_extent
                      and __detail::__static_range_size<_Rg> >= _RV::size())
        return _RV(__detail::__private_init, _RV::_Impl::_S_load(__ptr, __type_tag));
      else if (__rg_size >= _RV::size())
        return _RV(__detail::__private_init, _RV::_Impl::_S_load(__ptr, __type_tag));
      else if (__builtin_is_constant_evaluated() or __builtin_constant_p(__rg_size))
        return generate<_RV>([&](size_t __i) {
                 return __i < __rg_size ? __range[__i] : _Rp();
               });
      else
        return [&] {
          _RV __ret {};
          const int __bytes_to_read = (_RV::size() - __rg_size) * sizeof(_Rp);
          __builtin_memcpy(&__data(__ret), __ptr, __bytes_to_read);
          return __ret;
        }();
    }

  template <typename _Tp = void, contiguous_iterator _First, sentinel_for<_First> _Last,
            typename... _Flags>
    _GLIBCXX_SIMD_INTRINSIC
    constexpr auto
    load(_First __first, _Last __last, flags<_Flags...> __flags = {})
    { return load<_Tp>(std::span(__first, __last), __flags); }

  template <typename _Tp = void, contiguous_iterator _First, typename... _Flags>
    _GLIBCXX_SIMD_INTRINSIC
    constexpr auto
    load(_First __first, size_t __size, flags<_Flags...> __flags = {})
    { return load<_Tp>(std::span(__first, __size), __flags); }

  // (first, last) and (first, size) overloads are in fwddecl.h
  template <typename _Tp, typename _Abi, ranges::contiguous_range _Rg, typename... _Flags>
    requires std::ranges::output_range<_Rg, _Tp>
    constexpr void
    store(const basic_simd<_Tp, _Abi>& __v, _Rg&& __range, flags<_Flags...> __flags)
    {
      using _TV = basic_simd<_Tp, _Abi>;
      static_assert(destructible<_TV>);
      static_assert(__detail::__loadstore_convertible_to<
                      _Tp, std::ranges::range_value_t<_Rg>, _Flags...>,
                    "The converting store is not value-preserving. "
                    "Pass 'std::simd::flag_convert' if lossy conversion matches the intent.");

      constexpr bool __allow_out_of_bounds
        = (false or ... or is_same_v<_Flags, __detail::_AllowPartialStore>);
      static_assert(__detail::__static_range_size<_Rg> >= _TV::size.value
                      or __allow_out_of_bounds
                      or __detail::__static_range_size<_Rg> == dynamic_extent,
                    "Out-of-bounds access: simd::simd store past the end of the output range");

      auto* __ptr = __flags.template _S_adjust_pointer<_TV>(std::ranges::data(__range));
      constexpr _Tp* __type_tag = nullptr;

      const auto __rg_size = std::ranges::size(__range);
      if constexpr (not __allow_out_of_bounds)
        __glibcxx_simd_precondition(std::ranges::size(__range) >= _TV::size(),
                                    "output range is too small. "
                                    "Consider passing 'std::simd::flag_allow_partial_store'.");

#ifdef __AVX512F__
      if constexpr (__allow_out_of_bounds)
        {
          const typename _TV::mask_type __k([&](unsigned __i) { return __i < __rg_size; });
          _TV::_Impl::_S_masked_store(__data(__v), __ptr, __data(__k));
          return;
        }
#endif

      if constexpr (__detail::__static_range_size<_Rg> != dynamic_extent
                      and __detail::__static_range_size<_Rg> >= _TV::size())
        _TV::_Impl::_S_store(__data(__v), __ptr, __type_tag);
      else if (__rg_size >= _TV::size())
        _TV::_Impl::_S_store(__data(__v), __ptr, __type_tag);
      else if (__builtin_is_constant_evaluated() or __builtin_constant_p(__rg_size))
        {
          for (unsigned __i = 0; __i < __rg_size; ++__i)
            __ptr[__i] = static_cast<std::ranges::range_value_t<_Rg>>(__v[__i]);
        }
      else
        {
          const int __bytes_to_write = (_TV::size() - __rg_size) * sizeof(_Tp);
          __builtin_memcpy(__ptr, &__data(__v), __bytes_to_write);
        }
    }

  template <typename _Tp, typename _Abi, ranges::contiguous_range _Rg, typename... _Flags>
    requires std::ranges::output_range<_Rg, _Tp>
    constexpr void
    store(const basic_simd<_Tp, _Abi>& __v, _Rg&& __range,
          const typename basic_simd<_Tp, _Abi>::mask_type& __k, flags<_Flags...> __flags)
    {
      using _TV = basic_simd<_Tp, _Abi>;
      static_assert(__detail::__loadstore_convertible_to<
                      _Tp, std::ranges::range_value_t<_Rg>, _Flags...>,
                    "The converting store is not value-preserving. "
                    "Pass 'std::simd::flag_convert' if lossy conversion matches the intent.");

      constexpr bool __allow_out_of_bounds
        = (false or ... or is_same_v<_Flags, __detail::_AllowPartialStore>);

      auto* __ptr = __flags.template _S_adjust_pointer<_TV>(std::ranges::data(__range));

      const auto __rg_size = std::ranges::size(__range);
      if constexpr (not __allow_out_of_bounds)
        __glibcxx_simd_precondition(none_of(__k)
                                      or std::ranges::size(__range) > size_t(reduce_max_index(__k)),
                                    "output range is too small. "
                                    "Consider passing 'std::simd::flag_allow_partial_store'.");

      if (__builtin_is_constant_evaluated())
        {
          for (int __i = 0; __i < _TV::size(); ++__i)
            if (__k[__i] and (not __allow_out_of_bounds or size_t(__i) < __rg_size))
              __ptr[__i] = static_cast<std::ranges::range_value_t<_Rg>>(__v[__i]);
        }
      else if (__allow_out_of_bounds && __rg_size < _TV::size())
        {
          using _Ip = __detail::__mask_integer_from<sizeof(_Tp)>;
          constexpr rebind_simd_t<_Ip, _TV> __iota([](_Ip i) { return i; });
          const typename _TV::mask_type __k2 = __iota < _Ip(__rg_size);
          _TV::_Impl::_S_masked_store(__data(__v), __ptr, __data(__k & __k2));
        }
      else
        _TV::_Impl::_S_masked_store(__data(__v), __ptr, __data(__k));
    }

#if not SIMD_DISABLED_HAS_API
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
#endif

  // --------------------------------------------------------------
  // supported
  template <typename _Tp, typename _Abi>
    class basic_simd
    {
#if not SIMD_DISABLED_HAS_API
      static_assert(__detail::__vectorizable<_Tp> and __detail::__valid_abi_tag<_Abi, _Tp>);
#endif

      using _Traits = __detail::_SimdTraits<_Tp, _Abi>;

      static constexpr _Tp* _S_type_tag = nullptr;

    public:
      using _MemberType = typename _Traits::_SimdMember;

      alignas(_Traits::_S_simd_align) _MemberType _M_data;

      using _Impl = typename _Traits::_SimdImpl;

      using reference = __detail::_SmartReference<_MemberType, _Impl, _Tp>;

      using value_type = _Tp;

      using abi_type = _Abi;

      using mask_type = std::basic_simd_mask<sizeof(_Tp), _Abi>;

      static constexpr auto size = __detail::__ic<_Traits::_S_size>;

#if SIMD_IS_A_RANGE
      using iterator = __simd_iterator<basic_simd>;
      using const_iterator = __simd_iterator<const basic_simd>;

      //static_assert(std::random_access_iterator<iterator>);
      //static_assert(std::sentinel_for<std::default_sentinel_t, iterator>);

      constexpr iterator
      begin()
      { return iterator(*this, 0); }

      constexpr const_iterator
      begin() const
      { return const_iterator(*this, 0); }

      constexpr const_iterator
      cbegin() const
      { return const_iterator(*this, 0); }

      constexpr std::default_sentinel_t
      end() const
      { return {}; }

      constexpr std::default_sentinel_t
      cend() const
      { return {}; }
#endif

      constexpr
      basic_simd() = default;

      // ABI-specific conversions
      template <typename _Up>
        requires requires { _Traits::template _S_simd_conversion<_Up>(_M_data); }
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _GLIBCXX_SIMD_IMPLDEF_CONV_EXPLICIT
        operator _Up() const
        { return _Traits::template _S_simd_conversion<_Up>(_M_data); }

      template <typename _Up>
        requires (_Traits::template _S_is_simd_ctor_arg<_Up>)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _GLIBCXX_SIMD_IMPLDEF_CONV_EXPLICIT
        basic_simd(_Up __x)
        : _M_data(_Traits::_S_simd_construction(__x))
        {}

      // implicit broadcast constructor
#if SIMD_BROADCAST_CONDITIONAL_EXPLICIT
      template <typename _Up>
        requires constructible_from<value_type, _Up>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        explicit(not __detail::__broadcast_constructible<_Up, value_type>)
#else
      template <__detail::__broadcast_constructible<value_type> _Up>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
#endif
        basic_simd(_Up&& __x) noexcept
        : _M_data(_Impl::_S_broadcast(value_type(static_cast<_Up&&>(__x))))
        {}

      template <__detail::__value_preserving_convertible_to<value_type> _U0,
                same_as<_U0> _U1, same_as<_U0>... _Us>
        requires (size.value == 2 + sizeof...(_Us))
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_simd(_U0 __x0, _U1 __x1, _Us... __xs)
        : basic_simd(array<value_type, size.value>{__x0, __x1, __xs...})
        {}

      // type conversion constructor
      template <typename _Up, typename _UAbi>
        requires(__simd_size_v<_Up, _UAbi> == size() and std::constructible_from<_Tp, _Up>)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        explicit(not __detail::__value_preserving_convertible_to<_Up, value_type>
                   || __detail::__higher_rank_than<_Up, value_type>)
        basic_simd(const basic_simd<_Up, _UAbi>& __x) noexcept
        : _M_data(__detail::_SimdConverter<_Up, _UAbi, _Tp, _Abi>()(__data(__x)))
        {}

      // generator constructor
      template <__detail::__simd_generator_invokable<value_type, size()> _Fp>
        constexpr explicit
        basic_simd(_Fp&& __gen) noexcept
        : _M_data(_Impl::template _S_generator<value_type>(static_cast<_Fp&&>(__gen)))
        {}

      template <__detail::__almost_simd_generator_invokable<value_type, size()> _Fp>
        constexpr explicit
        basic_simd(_Fp&& )
          = _GLIBCXX_DELETE_MSG("Invalid return type of the generator function: "
                                "Requires value-preserving conversion or implicitly "
                                "convertible user-defined type.");

      // ranges typically don't have a static size
      // but if one does, this ctor is useful (std::array, C-array, span of static extent)
      //
      // Note that simd::simd is not a match, because it's not a contiguous range. Thus, if the
      // constraint were to be relaxed to a random-access range, I'd expect ambiguities with the
      // conversion constructor.
      template <__detail::__static_sized_range<size.value> _Rg, typename... _Flags>
        constexpr // implicit!
        basic_simd(_Rg&& __range, flags<_Flags...> __flags = {})
        : _M_data(_Impl::_S_load(__flags.template _S_adjust_pointer<basic_simd>(
                                   std::ranges::data(__range)), _S_type_tag))
        {
          static_assert(__detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                             value_type, _Flags...>);
        }

#if RANGES_TO_SIMD
      // optimize the contiguous_range case
      template <std::ranges::contiguous_range _Rg, typename... _Flags>
        requires std::ranges::sized_range<_Rg>
        constexpr explicit
        basic_simd(std::from_range_t, _Rg&& __range, flags<_Flags...> __flags = {})
        : _M_data(__data(std::load<basic_simd>(__range, __flags)))
        {
          static_assert(__detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                             value_type, _Flags...>);
          __glibcxx_simd_precondition(std::ranges::size(__range) <= unsigned(size),
                                      "Input range is too large. "
                                      "Consider using std::views::take(N) or something similar "
                                      "for reducing the size of the input.");
        }

      // support non-contiguous_range as well
      template <std::ranges::input_range _Rg, typename... _Flags>
        requires __detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                      value_type, _Flags...>
        constexpr explicit
        basic_simd(std::from_range_t, _Rg&& __range, flags<_Flags...> __flags = {})
        : _M_data(_Impl::template _S_generator<value_type>(
                    [&__range, __it = std::ranges::begin(__range)] (int __i) mutable {
                      __glibcxx_simd_precondition(__it != std::ranges::end(__range),
                                                  "Input range is too small.");
                      auto __r = static_cast<value_type>(*__it++);
                      __glibcxx_simd_precondition(__i + 1 < size
                                                    or __it == std::ranges::end(__range),
                                      "Input range is too large. "
                                      "Consider using std::views::take(N) or something similar "
                                      "for reducing the size of the input.");
                      return __r;
                    }))
        {}

      // and give a better error message when the user might have expected `ranges::to` to work
      template <std::ranges::range _Rg, typename... _Flags>
        basic_simd(std::from_range_t, _Rg&&, flags<_Flags...> = {})
        : _M_data{}
        {
          static_assert(false, "'ranges::to<basic_simd>()' requires a value-preserving conversion. "
                               "Call 'ranges::to<basic_simd>(simd::flag_convert)' to allow all "
                               "implicit conversions.");
        }
#endif

      // private init
      _GLIBCXX_SIMD_INTRINSIC constexpr
      basic_simd(__detail::_PrivateInit, const _MemberType& __init)
      : _M_data(__init)
      {}

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
      {
        __glibcxx_simd_precondition(is_unsigned_v<value_type> or all_of(__y >= value_type()),
                                    "negative shift is undefined behavior");
        __glibcxx_simd_precondition(
          all_of(__y < value_type(std::max(sizeof(int), sizeof(value_type)) * __CHAR_BIT__)),
          "too large shift invokes undefined behavior");
        return {__detail::__private_init, _Impl::_S_bit_shift_right(__data(__x), __data(__y))};
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator<<(const basic_simd& __x, const basic_simd& __y)
      requires requires (value_type __a) { __a << __a; }
      {
        __glibcxx_simd_precondition(is_unsigned_v<value_type> or all_of(__y >= value_type()),
                                    "negative shift is undefined behavior");
        __glibcxx_simd_precondition(
          all_of(__y < value_type(std::max(sizeof(int), sizeof(value_type)) * __CHAR_BIT__)),
          "too large shift invokes undefined behavior");
        return {__detail::__private_init, _Impl::_S_bit_shift_left(__data(__x), __data(__y))};
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator>>(const basic_simd& __x, int __y)
      requires requires (value_type __a, int __b) { __a >> __b; }
      {
        __glibcxx_simd_precondition(__y >= 0, "negative shift is undefined behavior");
        __glibcxx_simd_precondition(
          __y < int(std::max(sizeof(int), sizeof(value_type)) * __CHAR_BIT__),
          "too large shift invokes undefined behavior");
        return {__detail::__private_init, _Impl::_S_bit_shift_right(__data(__x), __y)};
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr friend basic_simd
      operator<<(const basic_simd& __x, int __y)
      requires requires (value_type __a, int __b) { __a << __b; }
      {
        __glibcxx_simd_precondition(__y >= 0, "negative shift is undefined behavior");
        __glibcxx_simd_precondition(
          __y < int(std::max(sizeof(int), sizeof(value_type)) * __CHAR_BIT__),
          "too large shift invokes undefined behavior");
        return {__detail::__private_init, _Impl::_S_bit_shift_left(__data(__x), __y)};
      }

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
      _M_to_array() const noexcept
      {
        std::array<_Tp, size()> __r = {};
        std::store(*this, __r);
        return __r;
      }

        //explicit
      operator std::array<_Tp, size()>() const noexcept
      { return _M_to_array(); }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_simd
      select_impl(const mask_type& __k, const basic_simd& __t, const basic_simd& __f)
      {
        auto __ret = __f;
        _Impl::_S_masked_assign(__data(__k), __ret._M_data, __t._M_data);
        return __ret;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr value_type
      operator[](__detail::_SimdSizeType __i) const
      {
        __glibcxx_simd_precondition(__i >= 0, "error: negative index");
        __glibcxx_simd_precondition(__i < size.value, "error: index out of bounds");
        return _Impl::_S_get(_M_data, __i);
      }

#if SIMD_HAS_SUBSCRIPT_GATHER
      template <std::integral _Up, typename _Ap>
        constexpr
        resize_simd_t<__simd_size_v<_Up, _Ap>, basic_simd>
        operator[](basic_simd<_Up, _Ap> const& __idx) const
        {
          __glibcxx_simd_precondition(is_unsigned_v<_Up> or all_of(__idx >= 0), "out-of-bounds");
          __glibcxx_simd_precondition(all_of(__idx < _Up(size)), "out-of-bounds");
          using _Rp = resize_simd_t<__simd_size_v<_Up, _Ap>, basic_simd>;
          return _Rp(__detail::__private_init,
                     _Rp::_Impl::template _S_generator<value_type>([&](int __i) {
                       return _Impl::_S_get(_M_data, __idx[__i]);
                     }));
        }
#endif

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

  template <__detail::__static_sized_range _Rg, typename... _Other>
    basic_simd(_Rg&& __r, _Other...)
      -> basic_simd<ranges::range_value_t<_Rg>,
                   __detail::__deduce_t<ranges::range_value_t<_Rg>,
#if 0 // PR117849
                                        ranges::size(__r)>>;
#else
                                        decltype(std::span(__r))::extent>>;
#endif

#if RANGES_TO_SIMD
    template <std::ranges::input_range _Rg>
    basic_simd(std::from_range_t, _Rg&& x)
    -> basic_simd<std::ranges::range_value_t<_Rg>>;

  template <std::ranges::input_range _Rg, typename... _Flags>
    basic_simd(std::from_range_t, _Rg&& x, flags<_Flags...>)
    -> basic_simd<std::ranges::range_value_t<_Rg>>;
#endif

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

  template <same_as<void> = void, typename _Fp>
    requires __detail::__simd_generator_invokable<
               _Fp, decltype(declval<_Fp&&>()(__detail::__ic<0>)),
               simd<decltype(declval<_Fp&&>()(__detail::__ic<0>))>::size()>
    constexpr simd<decltype(declval<_Fp&&>()(__detail::__ic<0>))>
    generate(_Fp&& __gen)
    {
      using _Tp = decltype(declval<_Fp&&>()(__detail::__ic<0>));
      using _Vp = simd<_Tp>;
      return _Vp(__detail::__private_init,
                 _Vp::_Impl::template _S_generator<_Tp>(static_cast<_Fp&&>(__gen)));
    }

  template <__detail::__simd_or_mask _Vp = void,
            __detail::__simd_generator_invokable<typename _Vp::value_type, _Vp::size()> _Fp>
    constexpr _Vp
    generate(_Fp&& __gen)
    {
      if constexpr (__detail::__simd_type<_Vp>)
        return _Vp(__detail::__private_init,
                   _Vp::_Impl::template _S_generator<typename _Vp::value_type>(
                     static_cast<_Fp&&>(__gen)));
      else
        return _Vp(static_cast<_Fp&&>(__gen));
    }
}

#endif  // PROTOTYPE_SIMD2_H_
// vim: et ts=8 sw=2 tw=100 cc=101
