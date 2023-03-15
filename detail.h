/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_DETAIL_H_
#define PROTOTYPE_DETAIL_H_

#include "fwddecl.h"

#include <ranges>
#include <concepts>
#include <experimental/simd>
#include <limits>

namespace std
{
  namespace __detail
  {
    template <typename _Tp, std::size_t _Np>
      using __deduced_simd = std::simd<_Tp, std::simd_abi::deduce_t<_Tp, _Np>>;

    template <typename _Tp, std::size_t _Np>
      using __deduced_simd_mask = std::simd_mask<_Tp, std::simd_abi::deduce_t<_Tp, _Np>>;

    template <size_t _Bytes>
      struct __make_unsigned;

    template <>
      struct __make_unsigned<sizeof(unsigned int)>
      { using type = unsigned int; };

    template <>
      struct __make_unsigned<sizeof(unsigned long)
                               + (sizeof(unsigned long) == sizeof(unsigned int))>
      { using type = unsigned long; };

    template <>
      struct __make_unsigned<sizeof(unsigned long long)
                               + (sizeof(unsigned long long) == sizeof(unsigned long))>
      { using type = unsigned long long; };

    template <>
      struct __make_unsigned<sizeof(unsigned short)>
      { using type = unsigned short; };

    template <>
      struct __make_unsigned<sizeof(unsigned char)>
      { using type = unsigned char; };

    template <typename _Tp>
      using __make_unsigned_t = typename __make_unsigned<sizeof(_Tp)>::type;

    template <std::ranges::sized_range _Tp>
      constexpr inline size_t
      __static_range_size = []() {
        if constexpr (requires { {_Tp::size()} -> std::integral; })
          return _Tp::size();
        else if constexpr (requires { {_Tp::extent} -> std::integral; })
          return _Tp::extent;
        else if constexpr (requires { {std::tuple_size_v<_Tp>} -> std::integral; })
          return std::tuple_size_v<_Tp>;
        else
          return std::dynamic_extent;
      }();

    template <auto _Value>
      using _Cnst = integral_constant<std::remove_const_t<decltype(_Value)>, _Value>;

    template <auto _Value>
      inline constexpr _Cnst<_Value> __cnst{};

    using namespace std::experimental::parallelism_v2;
    using namespace std::experimental::parallelism_v2::__proposed;

    template <typename _Tp>
      concept __arithmetic = integral<_Tp> || floating_point<_Tp>;

    template <typename _Tp>
      concept __vectorizable = __arithmetic<_Tp> && not same_as<_Tp, bool>;

    template <typename _Abi>
      concept __simd_abi_tag = std::is_abi_tag_v<_Abi>;

    template <typename _Vp, std::size_t _Width = 0>
      concept __simd_type = std::is_simd_v<_Vp>
                              && __vectorizable<typename _Vp::value_type>
                              && __simd_abi_tag<typename _Vp::abi_type>
                              && (_Width == 0 || _Vp::size() == _Width);

    template <typename _Vp, std::size_t _Width = 0>
      concept __mask_type = std::is_simd_mask_v<_Vp>
                              && __simd_abi_tag<typename _Vp::abi_type>
                              && (_Width == 0 || _Vp::size() == _Width);

    template <typename _From, typename _To>
      concept __value_preserving_convertible_to
        = convertible_to<_From, _To>
            && (not __arithmetic<_From> || not __arithmetic<_To>
                  || (not (is_signed_v<_From> && is_unsigned_v<_To>)
                        && numeric_limits<_From>::digits <= numeric_limits<_To>::digits
                        && numeric_limits<_From>::max() <= numeric_limits<_To>::max()
                        && numeric_limits<_From>::lowest() >= numeric_limits<_To>::lowest()));

    template <typename _From, typename _To>
      concept __value_preserving_or_int
        = __value_preserving_convertible_to<remove_cvref_t<_From>, _To>
            || (__arithmetic<_To> && same_as<remove_cvref_t<_From>, int>)
            || (unsigned_integral<_To> && same_as<remove_cvref_t<_From>, unsigned>);

    // __higher_floating_point_rank_than<_Tp, U> (_Tp has higher or equal floating point rank than U)
    template <typename _From, typename _To>
      concept __higher_floating_point_rank_than
        = floating_point<_From> && floating_point<_To>
            && same_as<common_type_t<_From, _To>, _From>;

    // __higher_integer_rank_than<_Tp, U> (_Tp has higher or equal integer rank than U)
    template <typename _From, typename _To>
      concept __higher_integer_rank_than
        = integral<_From> && integral<_To>
            && (sizeof(_From) > sizeof(_To) || same_as<common_type_t<_From, _To>, _From>);

    template <typename _From, typename _To>
      concept __higher_rank_than
        = __higher_floating_point_rank_than<_From, _To> || __higher_integer_rank_than<_From, _To>;

    template <typename _Fp, typename _Tp, std::size_t... _Is>
      constexpr
      _Cnst<(__value_preserving_or_int<invoke_result_t<_Fp, _Cnst<_Is>>, _Tp>
                         && ...)>
      __simd_broadcast_invokable_impl(index_sequence<_Is...>);

    template <typename _Fp, typename _Tp, std::size_t _Np>
      concept __simd_broadcast_invokable = requires
      {
        { __simd_broadcast_invokable_impl<_Fp, _Tp>(make_index_sequence<_Np>()) }
          -> same_as<_Cnst<true>>;
      };

    template <typename _Fp>
      concept __index_permutation_function = requires(_Fp const& __f)
      {
        { __f(0) } -> std::integral;
      };
  }
}

#endif  // PROTOTYPE_DETAIL_H_
