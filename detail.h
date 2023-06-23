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
      using _Ic = integral_constant<std::remove_const_t<decltype(_Value)>, _Value>;

    template <auto _Value>
      inline constexpr _Ic<_Value> __ic{};

    using namespace std::experimental::parallelism_v2;
    using namespace std::experimental::parallelism_v2::__proposed;

    template <typename _Tp>
      concept __arithmetic = integral<_Tp> || floating_point<_Tp>;

    template <typename _Tp>
      concept __vectorizable
        = __arithmetic<_Tp> and not same_as<_Tp, bool> and not same_as<_Tp, long double>;

    template <typename _Abi>
      concept __simd_abi_tag = std::is_abi_tag_v<_Abi>;

    template <typename _Vp, _SimdSizeType _Width = 0>
      concept __simd_type = std::is_simd_v<_Vp>
                              && __vectorizable<typename _Vp::value_type>
                              && __simd_abi_tag<typename _Vp::abi_type>
                              && (_Width == 0 || _Vp::size() == _Width);

    template <typename _Vp, _SimdSizeType _Width = 0>
      concept __mask_type = std::is_simd_mask_v<_Vp>
                              && __simd_abi_tag<typename _Vp::abi_type>
                              && (_Width == 0 || _Vp::size() == _Width);

    template <typename _Vp, _SimdSizeType _Width = 0>
      concept __simd_or_mask = __simd_type<_Vp, _Width> or __mask_type<_Vp, _Width>;

    template <typename _From, typename _To>
      concept __value_preserving_convertible_to
        = convertible_to<_From, _To>
            && (not __arithmetic<_From> || not __arithmetic<_To>
                  || (__vectorizable<_From>
                        && not (is_signed_v<_From> && is_unsigned_v<_To>)
                        && numeric_limits<_From>::digits <= numeric_limits<_To>::digits
                        && numeric_limits<_From>::max() <= numeric_limits<_To>::max()
                        && numeric_limits<_From>::lowest() >= numeric_limits<_To>::lowest()));

    template <typename _From, typename _To>
      concept __non_narrowing_constexpr_conversion
        = convertible_to<_From, _To>
            and requires { { _From::value } -> std::convertible_to<_To>; }
            and static_cast<decltype(_From::value)>(_To(_From::value)) == _From::value
            and not (std::unsigned_integral<_To> and _From::value < 0)
            and _From::value <= std::numeric_limits<_To>::max()
            and _From::value >= std::numeric_limits<_To>::lowest();

    template <typename _From, typename _To>
      concept __broadcast_constructible
        = __value_preserving_convertible_to<remove_cvref_t<_From>, _To>
            or __non_narrowing_constexpr_conversion<remove_cvref_t<_From>, _To>;

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

    // std::common_type but without integral promotions
    template <typename _T0, typename _T1>
      struct __nopromot_common_type : std::common_type<_T0, _T1>
      {};

    template <typename _Tp>
      struct __nopromot_common_type<_Tp, _Tp>
      { using type = _Tp; };

    template <typename _T0, typename _T1>
      requires __higher_integer_rank_than<int, _T0> and __higher_integer_rank_than<int, _T1>
        and (std::is_signed_v<_T0> == std::is_signed_v<_T1>)
      struct __nopromot_common_type<_T0, _T1>
      : std::conditional<__higher_integer_rank_than<_T0, _T1>, _T0, _T1>
      {};

    template <typename _T0, typename _T1>
      requires __higher_integer_rank_than<int, _T0> and __higher_integer_rank_than<int, _T1>
        and (std::is_signed_v<_T0> != std::is_signed_v<_T1>)
      struct __nopromot_common_type<_T0, _T1>
      {
        using _Up = std::conditional_t<std::is_signed_v<_T0>, _T1, _T0>;
        using _Sp = std::conditional_t<std::is_signed_v<_T0>, _T0, _T1>;
        using type = std::conditional_t<(sizeof(_Up) >= sizeof(_Sp)), _Up, _Sp>;
      };

    template <typename _T0, typename _T1>
      using __nopromot_common_type_t = typename __nopromot_common_type<_T0, _T1>::type;

    template <typename _Fp, typename _Tp, _SimdSizeType... _Is>
      constexpr
      _Ic<(__broadcast_constructible<invoke_result_t<_Fp, _Ic<_Is>>, _Tp> && ...)>
      __simd_broadcast_invokable_impl(integer_sequence<_SimdSizeType, _Is...>);

    template <typename _Fp, typename _Tp, _SimdSizeType _Np>
      concept __simd_broadcast_invokable = requires
      {
        { __simd_broadcast_invokable_impl<_Fp, _Tp>(make_integer_sequence<_SimdSizeType, _Np>()) }
          -> same_as<_Ic<true>>;
      };

    template <typename _Fp>
      concept __index_permutation_function_nosize = requires(_Fp const& __f)
      {
        { __f(0) } -> std::integral;
      };

    template <typename _Fp, typename _Simd>
      concept __index_permutation_function_size = requires(_Fp const& __f)
      {
        { __f(0, _Simd::size) } -> std::integral;
      };

    template <typename _Fp, typename _Simd>
      concept __index_permutation_function
        = __index_permutation_function_size<_Fp, _Simd> or __index_permutation_function_nosize<_Fp>;
  }
}

#endif  // PROTOTYPE_DETAIL_H_
