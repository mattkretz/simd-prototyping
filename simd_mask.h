/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_MASK2_H_
#define PROTOTYPE_SIMD_MASK2_H_

#include "detail.h"
#include "simd_abi.h"
#include "simd_iterator.h"

#include <concepts>
#include <climits>

namespace std
{
  namespace __detail
  {
    // Deducing the right ABI tag for basic_mask -> basic_simd is tricky for the AVX w/o AVX2
    // case, where basic_simd<T, Abi> might be unusable and we therefore need to deduce another ABI
    // tag.
    template <size_t _Bytes, typename _Abi>
      struct __simd_abi_for_mask
      { using type = __deduce_t<__mask_integer_from<_Bytes>, _Abi::_S_size>; };

    template <size_t _Bytes, typename _Abi>
      requires (std::destructible<std::basic_simd<__mask_integer_from<_Bytes>, _Abi>>
                  or not __simd_abi_tag<_Abi>)
      struct __simd_abi_for_mask<_Bytes, _Abi>
      { using type = _Abi; };

    template <size_t _Bytes, typename _Abi>
      using __simd_abi_for_mask_t = typename __simd_abi_for_mask<_Bytes, _Abi>::type;
  }

#if not SIMD_DISABLED_HAS_API
  // not supported:
  // - deleted: dctor, dtor, cctor, cassign
  // - no members except value_type and abi_type
  template <size_t _Bytes, typename _Abi>
    requires (__detail::_SimdMaskTraits<_Bytes, _Abi>::_S_size == 0)
    class basic_mask<_Bytes, _Abi>
    {
    public:
      using value_type = bool;

      using abi_type = _Abi;

      basic_mask() = delete;

      ~basic_mask() = delete;

      basic_mask(const basic_mask&) = delete;

      basic_mask& operator=(const basic_mask&) = delete;
    };
#endif

  template <size_t _Bytes, typename _Abi>
    class basic_mask
    {
      using _Tp = __detail::__mask_integer_from<_Bytes>;

      using _Traits = __detail::_SimdMaskTraits<_Bytes, _Abi>;

      using _MemberType = typename _Traits::_MaskMember;

      using _SimdType = std::basic_simd<_Tp, __detail::__simd_abi_for_mask_t<_Bytes, _Abi>>;

    public:
      // the only non-static data member
      alignas(_Traits::_S_mask_align) _MemberType _M_data;

      using _Impl = typename _Traits::_MaskImpl;

      static constexpr bool _S_is_bitmask = sizeof(_MemberType) < _Traits::_S_size;

      // really public:

      using value_type = bool;

      using reference = __detail::_SmartReference<_MemberType, _Impl, value_type>;

      using abi_type = _Abi;

      static constexpr auto size = __detail::__ic<_Traits::_S_size>;

#if SIMD_IS_A_RANGE
      using iterator = __simd_iterator<basic_mask>;
      using const_iterator = __simd_iterator<const basic_mask>;

      static_assert(std::random_access_iterator<iterator>);
      static_assert(std::sentinel_for<std::default_sentinel_t, iterator>);

      constexpr iterator
      begin()
      { return iterator(*this, 0); }

      constexpr const_iterator
      begin() const
      { return const_iterator(*this, 0); }

      constexpr std::default_sentinel_t
      end() const
      { return {}; }
#endif

      constexpr
      basic_mask() noexcept = default;

      // suggested extension [simd.mask.overview] p4
      // ABI-specific conversions
      template <typename _Up>
        requires requires { _Traits::template _S_mask_conversion<_Up>(_M_data); }
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        operator _Up() const
        { return _Traits::template _S_mask_conversion<_Up>(_M_data); }

      template <typename _Up>
        requires (_Traits::template _S_is_mask_ctor_arg<_Up>)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_mask(_Up __x)
        : _M_data(_Traits::_S_mask_construction(__x))
        {}

      // private init (implementation detail)
      _GLIBCXX_SIMD_INTRINSIC constexpr
      basic_mask(__detail::_PrivateInit, const _MemberType& __init) noexcept
      : _M_data(__init)
      {}

      // TODO bitset conversions

      // broadcast ctor
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
      basic_mask(value_type __x) noexcept
      : _M_data(_Impl::template _S_broadcast<_Tp>(__x))
      {}

      // conversion ctor
      template <size_t _UBytes, class _UAbi>
        requires(size() == basic_mask<_UBytes, _UAbi>::size.value)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        // TODO: new paper? implicit conversion if *only* the ABI tag differs:
        // equal size and equal sizeof (_Bytes), but different ABI tag
        // implicit conversion in one direction, but not the other (avoid interconvertible types)
        explicit(_UBytes != _Bytes or _Traits::template _S_explicit_mask_conversion<_UAbi>)
        basic_mask(const basic_mask<_UBytes, _UAbi>& __x) noexcept
        : basic_mask(__detail::__private_init, _Impl::template _S_convert<_Tp>(__x))
        {}

      // generator ctor
      template <__detail::__simd_generator_invokable<value_type, size.value> _Fp>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
        basic_mask(_Fp&& __gen) noexcept
        : _M_data([&]<__detail::_SimdSizeType... _Is>(__detail::_SimdIndexSequence<_Is...>) {
            if constexpr (requires {_Impl::template _S_mask_generator<_Tp>(__gen);})
              return _Impl::template _S_mask_generator<_Tp>(__gen);
            else if constexpr (size.value == 1)
              return bool(__gen(__detail::__ic<0>));
            else
              {
                static_assert(not _S_is_bitmask);
                return _MemberType { _Tp(-bool(__gen(__detail::__ic<_Is>)))... };
              }
          }(__detail::_MakeSimdIndexSequence<size()>()))
        {}

      // load ctor
      template <typename _It, typename... _Flags>
        requires std::same_as<std::iter_value_t<_It>, bool> and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_mask(_It __first, flags<_Flags...> __f = {})
        { copy_from(__first, __f); }

      // masked load ctor
      template <typename _It, typename... _Flags>
        requires std::same_as<std::iter_value_t<_It>, bool>
          and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        basic_mask(_It __first, const basic_mask& __k, flags<_Flags...> __f = {})
        : _M_data {}
        {
          const auto* __ptr = __f.template _S_adjust_pointer<basic_mask>(
                                std::addressof(*__first));
          _M_data = _Impl::_S_masked_load(_M_data, __k._M_data, __ptr);
        }

      // loads [simd.mask.copy]
      template <typename _It, typename... _Flags>
        requires std::same_as<std::iter_value_t<_It>, bool> and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(_It __first, flags<_Flags...> __f = {})
        {
          const auto* __ptr = __f.template _S_adjust_pointer<basic_mask>(
                                std::addressof(*__first));
          if (__builtin_is_constant_evaluated())
            {
              _M_data = [&]<__detail::_SimdSizeType... _Is> [[__gnu__::__always_inline__]]
                          (__detail::_SimdIndexSequence<_Is...>) {
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

      template <typename _It, typename... _Flags>
        requires std::same_as<std::iter_value_t<_It>, bool> and std::contiguous_iterator<_It>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_from(_It __first, const basic_mask& __k, flags<_Flags...> __f = {})
        {
          const auto* __ptr = __f.template _S_adjust_pointer<basic_mask>(
                                std::addressof(*__first));
          _M_data = _Impl::_S_masked_load(_M_data, __k._M_data, __ptr);
        }

      // stores [simd.mask.copy]
      template <typename _It, typename... _Flags>
        requires std::same_as<std::iter_value_t<_It>, bool>
          and std::contiguous_iterator<_It>
          and std::indirectly_writable<_It, value_type>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_It __first, flags<_Flags...> __f = {}) const
        {
          const auto* __ptr = __f.template _S_adjust_pointer<basic_mask>(
                                std::addressof(*__first));
          _Impl::_S_store(_M_data, __ptr);
        }

      template <typename _It, typename... _Flags>
        requires std::same_as<std::iter_value_t<_It>, bool>
          and std::contiguous_iterator<_It>
          and std::indirectly_writable<_It, value_type>
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr void
        copy_to(_It __first, const basic_mask& __k, flags<_Flags...> __f = {}) const
        {
          const auto* __ptr = __f.template _S_adjust_pointer<basic_mask>(
                                std::addressof(*__first));
          _Impl::_S_masked_store(_M_data, __ptr, __k._M_data);
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
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
        resize_simd_t<__simd_size_v<_Up, _Ap>, basic_mask>
        operator[](basic_simd<_Up, _Ap> const& __idx) const
        {
          __glibcxx_simd_precondition(is_unsigned_v<_Up> or all_of(__idx >= 0), "out-of-bounds");
          __glibcxx_simd_precondition(all_of(__idx < _Up(size)), "out-of-bounds");
          using _Rp = resize_simd_t<__simd_size_v<_Up, _Ap>, basic_mask>;
          return _Rp([&](int __i) {
                   return _Impl::_S_get(_M_data, __idx[__i]);
                 }));
        }
#endif

      // [simd.mask.unary]
      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr basic_mask
      operator!() const noexcept
      { return {__detail::__private_init, _Impl::_S_bit_not(_M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _SimdType
      operator+() const noexcept
      { return operator _SimdType(); }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _SimdType
      operator-() const noexcept
      {
        if constexpr (__detail::__vec_builtin<_MemberType> and sizeof(_M_data) == sizeof(_SimdType))
          return std::bit_cast<_SimdType>(_M_data);
        else
          {
            _SimdType __r = {};
            _SimdType::_Impl::_S_masked_assign(_M_data, __data(__r), _Tp(-1));
            return __r;
          }
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE constexpr _SimdType
      operator~() const noexcept
      {
        if constexpr (sizeof(basic_mask) == sizeof(_SimdType))
          return std::bit_cast<_SimdType>(*this) - _Tp(1);
        else
          {
            _SimdType __r = _Tp(-1);
            _SimdType::_Impl::_S_masked_assign(_M_data, __data(__r), _Tp(-2));
            return __r;
          }
      }

      // [simd.mask.conv]
      template <typename _Up, typename _UAbi>
        requires (__simd_size_v<_Up, _UAbi> == size.value)
        _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit(sizeof(_Up) != _Bytes)
        operator basic_simd<_Up, _UAbi>() const noexcept
        {
          using _Rp = basic_simd<_Up, _UAbi>;
          _Rp __r {};
          _Rp::_Impl::_S_masked_assign(__data(*this), __data(__r), 1);
          return __r;
        }

      // [simd.mask.binary]
      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator&&(const basic_mask& __x, const basic_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_logical_and(__x._M_data, __y._M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator||(const basic_mask& __x, const basic_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_logical_or(__x._M_data, __y._M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator&(const basic_mask& __x, const basic_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_bit_and(__x._M_data, __y._M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator|(const basic_mask& __x, const basic_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_bit_or(__x._M_data, __y._M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator^(const basic_mask& __x, const basic_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_bit_xor(__x._M_data, __y._M_data)}; }

      // [simd.mask.cassign]
      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask&
      operator&=(basic_mask& __x, const basic_mask& __y) noexcept
      {
        __x._M_data = _Impl::_S_bit_and(__x._M_data, __y._M_data);
        return __x;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask&
      operator|=(basic_mask& __x, const basic_mask& __y) noexcept
      {
        __x._M_data = _Impl::_S_bit_or(__x._M_data, __y._M_data);
        return __x;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask&
      operator^=(basic_mask& __x, const basic_mask& __y) noexcept
      {
        __x._M_data = _Impl::_S_bit_xor(__x._M_data, __y._M_data);
        return __x;
      }

      // [simd.mask.comparison]
      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator==(const basic_mask& __x, const basic_mask& __y) noexcept
      {
        return {__detail::__private_init,
                _Impl::_S_bit_not(_Impl::_S_bit_xor(__x._M_data, __y._M_data))};
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator!=(const basic_mask& __x, const basic_mask& __y) noexcept
      { return {__detail::__private_init, _Impl::_S_bit_xor(__x._M_data, __y._M_data)}; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator>=(const basic_mask& __x, const basic_mask& __y) noexcept
      { return __x || !__y; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator<=(const basic_mask& __x, const basic_mask& __y) noexcept
      { return !__x || __y; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator>(const basic_mask& __x, const basic_mask& __y) noexcept
      { return __x && !__y; }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      operator<(const basic_mask& __x, const basic_mask& __y) noexcept
      { return !__x && __y; }

      // [simd.mask.cond]
      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      select_impl(const basic_mask& __k, const basic_mask& __t,
                       const basic_mask& __f) noexcept
      {
        basic_mask __ret = __f;
        _Impl::_S_masked_assign(__k._M_data, __ret._M_data, __t._M_data);
        return __ret;
      }

      _GLIBCXX_SIMD_ALWAYS_INLINE friend constexpr basic_mask
      select_impl(const basic_mask& __k, same_as<bool> auto __t,
                       same_as<bool> auto __f) noexcept
      {
        if (__t == __f)
          return basic_mask(__t);
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
        select_impl(const basic_mask& __k, const _T0& __t, const _T1& __f) noexcept
        {
          using _Rp = __detail::__nopromot_common_type_t<_T0, _T1>;
          using _RV = simd<_Rp, size.value>;
          _RV __ret = __f;
          _RV::_Impl::_S_masked_assign(__data(__k), __data(__ret), _Rp(__t));
          return __ret;
        }

      _GLIBCXX_SIMD_INTRINSIC friend constexpr const auto&
      __data(const basic_mask& __x)
      { return __x._M_data; }

      _GLIBCXX_SIMD_INTRINSIC friend constexpr auto&
      __data(basic_mask& __x)
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

  template <size_t _Bs, typename _Abi>
    struct is_mask<basic_mask<_Bs, _Abi>>
    : is_default_constructible<basic_mask<_Bs, _Abi>>
    {};

  template <typename _Tp, typename _Up>
    constexpr auto
    select(bool __c, const _Tp& __x0, const _Up& __x1) noexcept
    -> remove_cvref_t<decltype(__c ? __x0 : __x1)>
    { return __c ? __x0 : __x1; }

  template <size_t _Np, typename _A0>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr auto
    select(const basic_mask<_Np, _A0>& __k, const auto& __x0, const auto& __x1) noexcept
    -> decltype(select_impl(__k, __x0, __x1))
    { return select_impl(__k, __x0, __x1); }
}

#endif  // PROTOTYPE_SIMD_MASK2_H_
// vim: et ts=8 sw=2 tw=100 cc=101
