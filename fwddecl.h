/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_FWDDECL_H_
#define PROTOTYPE_FWDDECL_H_

#include "simd_abi.h"

#include <type_traits>

namespace std
{
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

  template <typename _Tp, typename _Abi>
    struct simd_size : std::experimental::simd_size<_Tp, _Abi>
    {};

  template <typename _Tp, typename _Abi>
    inline constexpr size_t simd_size_v = simd_size<_Tp, _Abi>::value;

  template <typename _Tp, typename _Vp>
    struct rebind_simd : std::experimental::rebind_simd<_Tp, _Vp>
    {};

  template <typename _Tp, typename _Vp>
    using rebind_simd_t = typename rebind_simd<_Tp, _Vp>::type;

  template <int _Np, typename _Vp>
    struct resize_simd : std::experimental::resize_simd<_Np, _Vp>
    {};

  template <int _Np, typename _Vp>
    using resize_simd_t = typename resize_simd<_Np, _Vp>::type;

  template <typename _Tp, typename _Abi = simd_abi::native<_Tp>>
    class simd;

  template <typename _Tp, int _Np>
    using fixed_size_simd = simd<_Tp, simd_abi::fixed_size<_Np>>;

  template <typename _Tp, typename _Abi = simd_abi::native<_Tp>>
    class simd_mask;

  template <typename _Tp, int _Np>
    using fixed_size_simd_mask = simd_mask<_Tp, simd_abi::fixed_size<_Np>>;
}

#endif  // PROTOTYPE_FWDDECL_H_
