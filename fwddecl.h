/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_FWDDECL_H_
#define PROTOTYPE_FWDDECL_H_

//#include <experimental/bits/simd_detail.h>
#include "simd_config.h"

#include <functional>
#include <stdfloat>
#include <type_traits>

namespace std
{
  template <int _Width>
    struct _VecAbi;

  template <int _Width>
    struct _Avx512Abi;

  struct _ScalarAbi;

  namespace __detail
  {
    template <size_t _Np, bool _Sanitized = false>
      struct _BitMask;

    template <size_t _Np>
      using _SanitizedBitMask = _BitMask<_Np, true>;

    template <size_t _Bytes>
      struct __make_unsigned_int;

    template <>
      struct __make_unsigned_int<sizeof(unsigned int)>
      { using type = unsigned int; };

    template <>
      struct __make_unsigned_int<sizeof(unsigned long)
                                   + (sizeof(unsigned long) == sizeof(unsigned int))>
      { using type = unsigned long; };

    template <>
      struct __make_unsigned_int<sizeof(unsigned long long)
                                   + (sizeof(unsigned long long) == sizeof(unsigned long))>
      { using type = unsigned long long; };

    template <>
      struct __make_unsigned_int<sizeof(unsigned short)>
      { using type = unsigned short; };

    template <>
      struct __make_unsigned_int<sizeof(unsigned char)>
      { using type = unsigned char; };

    template <typename _Tp>
      using __make_unsigned_int_t = typename __make_unsigned_int<sizeof(_Tp)>::type;

    template <typename _Tp>
      using __make_signed_int_t = make_signed_t<__make_unsigned_int_t<_Tp>>;

    template <size_t _Bs>
      using __mask_integer_from = make_signed_t<typename __make_unsigned_int<_Bs>::type>;

    template <typename _Tp>
      struct __is_vectorizable
      : bool_constant<false>
      {};

    // TODO
    //template <> struct __is_vectorizable<std::byte> : bool_constant<true> {};

    template <> struct __is_vectorizable<char> : bool_constant<true> {};
    template <> struct __is_vectorizable<wchar_t> : bool_constant<true> {};
    template <> struct __is_vectorizable<char8_t> : bool_constant<true> {};
    template <> struct __is_vectorizable<char16_t> : bool_constant<true> {};
    template <> struct __is_vectorizable<char32_t> : bool_constant<true> {};

    template <> struct __is_vectorizable<  signed char> : bool_constant<true> {};
    template <> struct __is_vectorizable<unsigned char> : bool_constant<true> {};
    template <> struct __is_vectorizable<  signed short> : bool_constant<true> {};
    template <> struct __is_vectorizable<unsigned short> : bool_constant<true> {};
    template <> struct __is_vectorizable<  signed int> : bool_constant<true> {};
    template <> struct __is_vectorizable<unsigned int> : bool_constant<true> {};
    template <> struct __is_vectorizable<  signed long> : bool_constant<true> {};
    template <> struct __is_vectorizable<unsigned long> : bool_constant<true> {};
    template <> struct __is_vectorizable<  signed long long> : bool_constant<true> {};
    template <> struct __is_vectorizable<unsigned long long> : bool_constant<true> {};

    template <> struct __is_vectorizable<float> : bool_constant<true> {};
    template <> struct __is_vectorizable<double> : bool_constant<true> {};
#ifdef __STDCPP_FLOAT16_T__
    //template <> struct __is_vectorizable<std::float16_t> : bool_constant<true> {};
#endif
#ifdef __STDCPP_FLOAT32_T__
    template <> struct __is_vectorizable<std::float32_t> : bool_constant<true> {};
#endif
#ifdef __STDCPP_FLOAT64_T__
    template <> struct __is_vectorizable<std::float64_t> : bool_constant<true> {};
#endif

    template <typename _Tp, typename>
      struct __make_dependent
      { using type = _Tp; };

    template <typename _Tp, typename _Up>
      using __make_dependent_t = typename __make_dependent<_Tp, _Up>::type;

    template <int _Bs, typename _Tp>
      consteval auto
      __native_abi_impl_recursive()
      {
        constexpr int _Width = _Bs / sizeof(_Tp);
        if constexpr (_Avx512Abi<_Width>::template _IsValid<_Tp>::value)
          return _Avx512Abi<_Width>();
        else if constexpr (_VecAbi<_Width>::template _IsValid<_Tp>::value)
          return _VecAbi<_Width>();
        else if constexpr (_Bs > sizeof(_Tp))
          return __native_abi_impl_recursive<_Bs / 2, _Tp>();
        else
          return __make_dependent_t<_ScalarAbi, _Tp>();
      }

    struct _InvalidAbi
    {};

    template <typename _Tp>
      consteval auto
      __native_abi_impl()
      {
        if constexpr (__is_vectorizable<_Tp>::value)
          {
            // __one is used to make _VecAbi a dependent type
            constexpr int __one = sizeof(_Tp) / sizeof(_Tp);
            return __native_abi_impl_recursive<__one * 256, _Tp>();
          }
        else
          return _InvalidAbi();
      }

    template <typename _Tp>
      using _NativeAbi = decltype(__native_abi_impl<_Tp>());

    using _SimdSizeType = int;

    template <typename _Tp, _SimdSizeType _Np>
      struct _DeduceAbi;

    template <typename _Tp, _SimdSizeType _Np>
      using __deduce_t = typename _DeduceAbi<_Tp, _Np>::type;
  }

  template <typename _Abi0, __detail::_SimdSizeType _Np>
    struct _AbiArray;

  template <__detail::_SimdSizeType _Np, typename _Tag>
    struct _AbiCombine;

  template <typename T>
    struct representation_type
    {};

  template <typename T>
    using representation_type_t = typename representation_type<T>::type;

  template <typename _Tp, typename _Abi = __detail::_NativeAbi<_Tp>>
    class basic_simd;

  template <size_t _Bytes,
            typename _Abi = __detail::_NativeAbi<__detail::__mask_integer_from<_Bytes>>>
    class basic_simd_mask;

  template <typename... _Flags>
    struct simd_flags;

  template <typename _Tp>
    struct is_simd
    : bool_constant<false>
    {};

  template <typename _Tp>
    inline constexpr bool is_simd_v = is_simd<_Tp>::value;

  template <typename _Tp>
    struct is_simd_mask
    : bool_constant<false>
    {};

  template <typename _Tp>
    inline constexpr bool is_simd_mask_v = is_simd_mask<_Tp>::value;

  template <typename _Tp, typename _Abi = __detail::_NativeAbi<_Tp>>
    struct simd_size
    {};

  template <typename _Tp, typename _Abi = __detail::_NativeAbi<_Tp>>
    inline constexpr __detail::_SimdSizeType simd_size_v = simd_size<_Tp, _Abi>::value;

  template <typename _Tp, typename _Up = typename _Tp::value_type>
    struct simd_alignment
    {};

  template <typename _Tp, typename _Up = typename _Tp::value_type>
    inline constexpr size_t simd_alignment_v = simd_alignment<_Tp, _Up>::value;

  template <typename _Tp, typename _Vp>
    struct rebind_simd
    {};

  template <typename _Tp, typename _Vp>
    using rebind_simd_t = typename rebind_simd<_Tp, _Vp>::type;

  template <__detail::_SimdSizeType _Np, typename _Vp>
    struct resize_simd
    {};

  template <__detail::_SimdSizeType _Np, typename _Vp>
    using resize_simd_t = typename resize_simd<_Np, _Vp>::type;

  template <typename _Tp, __detail::_SimdSizeType _Np = basic_simd<_Tp>::size()>
    using simd = basic_simd<_Tp, __detail::__deduce_t<_Tp, _Np>>;

  template <typename _Tp, __detail::_SimdSizeType _Np = basic_simd<_Tp>::size()>
    using simd_mask = basic_simd_mask<sizeof(_Tp), __detail::__deduce_t<_Tp, _Np>>;

  // mask_reductions.h
  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    all_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept;

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
  all_of(same_as<bool> auto __x) noexcept;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    any_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept;

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
  any_of(same_as<bool> auto __x) noexcept;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    none_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept;

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
  none_of(same_as<bool> auto __x) noexcept;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_count(const basic_simd_mask<_Bs, _Abi>& __k) noexcept;

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
  reduce_count(same_as<bool> auto __x) noexcept;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_min_index(const basic_simd_mask<_Bs, _Abi>& __k);

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
  reduce_min_index(same_as<bool> auto __x) noexcept;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_max_index(const basic_simd_mask<_Bs, _Abi>& __k);

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
  reduce_max_index(same_as<bool> auto __x) noexcept;

  template <typename _V, typename _Tp, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr auto
    simd_split(const basic_simd<_Tp, _Abi>& __x) noexcept;

  template <typename _M, size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr auto
    simd_split(const basic_simd_mask<_Bs, _Abi>& __x) noexcept;

  template <typename _Tp, typename... _Abis>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
    simd<_Tp, (simd_size_v<_Tp, _Abis> + ...)>
    simd_cat(const basic_simd<_Tp, _Abis>&... __xs) noexcept;

  template <size_t _Bs, typename... _Abis>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
    simd_mask<__detail::__mask_integer_from<_Bs>, (basic_simd_mask<_Bs, _Abis>::size.value + ...)>
    simd_cat(const basic_simd_mask<_Bs, _Abis>&... __xs) noexcept;

  template <typename _Tp, typename _Abi,
            std::invocable<simd<_Tp, 1>, simd<_Tp, 1>> _BinaryOperation = plus<>>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, _BinaryOperation __binary_op = {});

  template <typename _Tp, typename _Abi,
            std::invocable<simd<_Tp, 1>, simd<_Tp, 1>> _BinaryOperation>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           __type_identity_t<_Tp> __identity_element, _BinaryOperation __binary_op);

  template <typename _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           plus<> __binary_op = {}) noexcept;

  template <typename _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           multiplies<> __binary_op) noexcept;

  template <std::integral _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           bit_and<> __binary_op) noexcept;

  template <std::integral _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           bit_or<> __binary_op) noexcept;

  template <std::integral _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           bit_xor<> __binary_op) noexcept;

  template <std::totally_ordered _Tp, typename _Abi>
    constexpr _Tp
    reduce_min(const basic_simd<_Tp, _Abi>& __x) noexcept;

  template <std::totally_ordered _Tp, typename _Abi>
    constexpr _Tp
    reduce_min(const basic_simd<_Tp, _Abi>& __x,
               const typename basic_simd<_Tp, _Abi>::mask_type& __k) noexcept;

  template <std::totally_ordered _Tp, typename _Abi>
    constexpr _Tp
    reduce_max(const basic_simd<_Tp, _Abi>& __x) noexcept;

  template <std::totally_ordered _Tp, typename _Abi>
    constexpr _Tp
    reduce_max(const basic_simd<_Tp, _Abi>& __x,
               const typename basic_simd<_Tp, _Abi>::mask_type& __k) noexcept;
}

#endif  // PROTOTYPE_FWDDECL_H_
