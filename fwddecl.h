/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_FWDDECL_H_
#define PROTOTYPE_FWDDECL_H_

#include <experimental/simd>

#include <type_traits>

namespace std
{
  namespace __pv2
  {
    using namespace std::experimental::parallelism_v2;
    using namespace std::experimental::parallelism_v2::__proposed;
  }

  namespace __detail
  {
    template <typename _Tp>
      using _NativeAbi = std::experimental::parallelism_v2::simd_abi::native<_Tp>;

    using _SimdSizeType = int;

    template <typename _Tp, _SimdSizeType _Np>
      struct _DeduceAbi
      {};

    template <typename _Tp, _SimdSizeType _Np>
      using __deduce_t = typename _DeduceAbi<_Tp, _Np>::type;
  }

  template <typename _Tp, typename _Abi = __detail::_NativeAbi<_Tp>>
    class basic_simd;

  template <size_t _Bytes, typename _Abi>
    class basic_simd_mask;

  using element_aligned_tag = std::experimental::element_aligned_tag;

  using vector_aligned_tag = std::experimental::vector_aligned_tag;

  template <size_t _Np>
    using overaligned_tag = std::experimental::overaligned_tag<_Np>;

  inline constexpr element_aligned_tag element_aligned = {};

  inline constexpr vector_aligned_tag vector_aligned = {};

  template <size_t _Np>
    inline constexpr overaligned_tag<_Np> overaligned = {};

  template <typename _Tp>
    struct is_abi_tag : std::experimental::is_abi_tag<_Tp>
    {};

  template <typename _Tp>
    inline constexpr bool is_abi_tag_v = is_abi_tag<_Tp>::value;

  template <typename _Tp>
    struct is_simd : std::experimental::is_simd<_Tp>
    {};

  template <typename _Tp>
    inline constexpr bool is_simd_v = is_simd<_Tp>::value;

  template <typename _Tp>
    struct is_simd_mask : std::experimental::is_simd_mask<_Tp>
    {};

  template <typename _Tp>
    inline constexpr bool is_simd_mask_v = is_simd_mask<_Tp>::value;

/*  template <typename _Tp>
    struct is_simd_flag_type : std::experimental::is_simd_flag_type<_Tp>
    {};

  template <typename _Tp>
    inline constexpr bool is_simd_flag_type_v = is_simd_flag_type<_Tp>::value;*/

  template <typename _Tp, typename _Abi = __detail::_NativeAbi<_Tp>>
    struct simd_size : std::experimental::simd_size<_Tp, _Abi>
    {};

  template <typename _Tp, typename _Abi = __detail::_NativeAbi<_Tp>>
    inline constexpr __detail::_SimdSizeType simd_size_v = simd_size<_Tp, _Abi>::value;

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

  template <typename _Tp, __detail::_SimdSizeType _Np>
    using simd_mask = basic_simd_mask<sizeof(_Tp), __detail::__deduce_t<_Tp, _Np>>;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    all_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    any_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
    none_of(const basic_simd_mask<_Bs, _Abi>& __k) noexcept;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_count(const basic_simd_mask<_Bs, _Abi>& __k) noexcept;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_min_index(const basic_simd_mask<_Bs, _Abi>& __k) noexcept;

  template <size_t _Bs, typename _Abi>
    _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
    reduce_max_index(const basic_simd_mask<_Bs, _Abi>& __k) noexcept;

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
    simd_mask<__pv2::__int_with_sizeof_t<_Bs>, (basic_simd_mask<_Bs, _Abis>::size.value + ...)>
    simd_cat(const basic_simd_mask<_Bs, _Abis>&... __xs) noexcept;

  template <typename _Tp, typename _Abi,
            std::invocable<simd<_Tp, 1>, simd<_Tp, 1>> _BinaryOperation = plus<>>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, _BinaryOperation __binary_op = {});

  template <typename _Tp, typename _Abi,
            std::invocable<simd<_Tp, 1>, simd<_Tp, 1>> _BinaryOperation>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           _Tp __identity_element, _BinaryOperation __binary_op);

  template <typename _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           plus<> __binary_op = {});

  template <typename _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           multiplies<> __binary_op);

  template <typename _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           bit_and<> __binary_op);

  template <typename _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           bit_or<> __binary_op);

  template <typename _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           bit_xor<> __binary_op);
}

#endif  // PROTOTYPE_FWDDECL_H_
