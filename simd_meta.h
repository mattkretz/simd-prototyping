/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_META_H_
#define PROTOTYPE_SIMD_META_H_

#include "flags.h"
#include "fwddecl.h"

namespace std::__detail
{
  template <typename _Tp>
    struct __assert_unreachable
    { static_assert(!is_same_v<_Tp, _Tp>, "this should be unreachable"); };

  template <typename _To, typename... _Args>
    concept __convertible_from
      = std::constructible_from<_To, _Args...>
          and requires(_To (&__fun)(_To), _Args&&... __args)
        {
          { __fun({static_cast<_Args&&>(__args)...}) } -> std::same_as<_To>;
        };

  template <typename _Tp>
    concept __vectorizable = __is_vectorizable<_Tp>::value;

  template <typename _Tp>
    concept __arithmetic = integral<_Tp> || floating_point<_Tp>;

  template <typename _Abi>
    concept __simd_abi_tag
      = not _Abi::template _IsValid<void>::value
          and requires { typename _Abi::_IsValidAbiTag; };

  template <typename _Abi, typename _Tp>
    concept __valid_abi_tag
      = __simd_abi_tag<_Abi> and _Abi::template _IsValid<_Tp>::value;

  template <typename _Vp, _SimdSizeType _Width = 0>
    concept __simd_type = std::is_simd_v<_Vp> // implies __vectorizable
                            && __simd_abi_tag<typename _Vp::abi_type>
                            && (_Width == 0 || _Vp::size() == _Width);

  template <typename _Vp, _SimdSizeType _Width = 0>
    concept __mask_type = std::is_simd_mask_v<_Vp>
                            && __simd_abi_tag<typename _Vp::abi_type>
                            && (_Width == 0 || _Vp::size() == _Width);

  template <typename _Vp, _SimdSizeType _Width = 0>
    concept __simd_or_mask = __simd_type<_Vp, _Width> or __mask_type<_Vp, _Width>;

  template<class T>
    concept __constexpr_wrapper_like
      = convertible_to<T, decltype(T::value)>
          and equality_comparable_with<T, decltype(T::value)>
          and bool_constant<T() == T::value>::value
          and bool_constant<static_cast<decltype(T::value)>(T()) == T::value>::value;

  template <typename _From, typename _To>
    concept __value_preserving_convertible_to
      = convertible_to<_From, _To>
          and (same_as<_From, _To> or not __arithmetic<_From> or not __arithmetic<_To>
                 or (__vectorizable<_From>
                       and not (is_signed_v<_From> and is_unsigned_v<_To>)
                       and numeric_limits<_From>::digits <= numeric_limits<_To>::digits
                       and numeric_limits<_From>::max() <= numeric_limits<_To>::max()
                       and numeric_limits<_From>::lowest() >= numeric_limits<_To>::lowest()));

  template <typename _From, typename _To>
    concept __non_narrowing_constexpr_conversion
      = __constexpr_wrapper_like<_From> and convertible_to<_From, _To>
          and requires { { _From::value } -> std::convertible_to<_To>; }
          and static_cast<decltype(_From::value)>(_To(_From::value)) == _From::value
          and not (std::unsigned_integral<_To> and _From::value < 0)
          and _From::value <= std::numeric_limits<_To>::max()
          and _From::value >= std::numeric_limits<_To>::lowest();

  template <typename _From, typename _To>
    concept __broadcast_constructible
      = (__value_preserving_convertible_to<remove_cvref_t<_From>, _To>
           and not __constexpr_wrapper_like<remove_cvref_t<_From>>)
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

  template <typename _From, typename _To, typename... _Flags>
    concept __loadstore_convertible_to
      = __vectorizable<_From> and __vectorizable<_To>
          and (__value_preserving_convertible_to<_From, _To>
                 or (std::convertible_to<_From, _To> and (std::same_as<_Flags, _Convert> or ...)));

  template <auto _Value>
    using _Ic = integral_constant<std::remove_const_t<decltype(_Value)>, _Value>;

  template <auto _Value>
    inline constexpr _Ic<_Value> __ic{};

  template <_SimdSizeType... _Is>
    using _SimdIndexSequence = std::integer_sequence<_SimdSizeType, _Is...>;

  template <_SimdSizeType _Np>
    using _MakeSimdIndexSequence = std::make_integer_sequence<_SimdSizeType, _Np>;

  template <typename _Fp, typename _Tp, _SimdSizeType... _Is>
    constexpr
    _Ic<(__broadcast_constructible<decltype(declval<_Fp>()(__ic<_Is>)), _Tp> and ...)>
    __simd_broadcast_invokable_impl(_SimdIndexSequence<_Is...>);

  template <typename _Fp, typename _Tp, _SimdSizeType _Np>
    concept __simd_broadcast_invokable = requires {
      { __simd_broadcast_invokable_impl<_Fp, _Tp>(_MakeSimdIndexSequence<_Np>()) }
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

  template <integral _Tp>
    _GLIBCXX_SIMD_INTRINSIC constexpr _Tp
    __div_roundup(_Tp __a, _Tp __b)
    { return (__a + __b - 1) / __b; }

  template <typename _Tp>
    concept __valid_simd = is_simd_v<_Tp>;

  template <typename _Tp>
    concept __valid_mask = is_simd_mask_v<_Tp>;

  template <typename T>
    concept __boolean_reducable_impl = requires(T&& x)
      {
        { std::all_of(x) } -> std::same_as<bool>;
        { std::none_of(x) } -> std::same_as<bool>;
        { std::any_of(x) } -> std::same_as<bool>;
        { std::reduce_count(x) } -> std::signed_integral;
        { std::reduce_min_index(x) } -> std::signed_integral;
        { std::reduce_max_index(x) } -> std::signed_integral;
      };

  template <typename T>
    concept __boolean_reducable = __boolean_reducable_impl<T> and requires(T&& x)
      {
        { !std::forward<T>(x) } -> __boolean_reducable_impl;
      };

  template <typename T, typename U>
    concept __simd_weakly_equality_comparable_with = requires(const std::remove_reference_t<T>& __t,
                                                              const std::remove_reference_t<U>& __u)
      {
        { __t == __u } -> __boolean_reducable;
        { __t != __u } -> __boolean_reducable;
        { __u == __t } -> __boolean_reducable;
        { __u != __t } -> __boolean_reducable;
      };

  template <typename _Tp, typename _Up>
    concept __simd_partially_ordered_with = requires(const std::remove_reference_t<_Tp>& __t,
                                                     const std::remove_reference_t<_Up>& __u)
      {
        { __t <  __u } -> __boolean_reducable;
        { __t >  __u } -> __boolean_reducable;
        { __t <= __u } -> __boolean_reducable;
        { __t >= __u } -> __boolean_reducable;
        { __u <  __t } -> __boolean_reducable;
        { __u >  __t } -> __boolean_reducable;
        { __u <= __t } -> __boolean_reducable;
        { __u >= __t } -> __boolean_reducable;
      };

  /* TODO
   * Do we need the common_reference checks for simd? The interesting scenarios must be user-defined
   * types that are convertible to simd, no? (operator simd() or derived from simd)
   */
  template <typename _Tp, typename _Up>
    concept __simd_comparison_common_type_with
      = std::common_reference_with<const std::remove_reference_t<_Tp>&,
                                   const std::remove_reference_t<_Up>&>;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Extensions.
////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: To be proposed in some form. Potentially, by splitting into simd and
// simd_generic concepts. But doing this properly requires a split of generic non-member functions
// into different namespaces.
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std
{
  template <typename _Tp>
    concept simd_integral = (__detail::__valid_simd<_Tp> or __detail::__valid_mask<_Tp>)
                              and std::integral<typename _Tp::value_type>;

  template <typename _Tp>
    concept simd_signed_integral
      = simd_integral<_Tp> and std::is_signed_v<typename _Tp::value_type>;

  template <typename _Tp>
    concept simd_unsigned_integral
      = simd_integral<_Tp> and not simd_signed_integral<_Tp>;

  template <typename _Tp>
    concept simd_floating_point
      = __detail::__valid_simd<_Tp> and std::floating_point<typename _Tp::value_type>;

  template <typename _Tp>
    concept simd_arithmetic = simd_integral<_Tp> or simd_floating_point<_Tp>;

  template <typename _Tp>
    concept simd_equality_comparable = __detail::__simd_weakly_equality_comparable_with<_Tp, _Tp>;

  template <typename _Tp, typename _Up>
    concept simd_equality_comparable_with
      = simd_equality_comparable<_Tp> and simd_equality_comparable<_Up>
          and __detail::__simd_comparison_common_type_with<_Tp, _Up>
          and simd_equality_comparable<std::common_reference_t<const std::remove_reference_t<_Tp>&,
                                                               const std::remove_reference_t<_Up>&>>
          and __detail::__simd_weakly_equality_comparable_with<_Tp, _Up>;

  template <typename _Tp>
    concept simd_totally_ordered
      = simd_equality_comparable<_Tp> and __detail::__simd_partially_ordered_with<_Tp, _Tp>;

  template <typename _Tp, typename _Up>
    concept simd_totally_ordered_with
      = simd_totally_ordered<_Tp> and simd_totally_ordered<_Up>
          and simd_equality_comparable_with<_Tp, _Up>
          and simd_totally_ordered<std::common_reference_t<const std::remove_reference_t<_Tp>&,
                                                           const std::remove_reference_t<_Up>&>>
          and __detail::__simd_partially_ordered_with<_Tp, _Up>;

  template <typename _Tp>
    concept simd_regular = semiregular<_Tp> and simd_equality_comparable<_Tp>;
}

namespace std::simd_generic
{
  template <typename _Tp>
    concept integral = std::integral<_Tp> or std::simd_integral<_Tp>;

  template <typename _Tp>
    concept signed_integral = std::signed_integral<_Tp> or std::simd_signed_integral<_Tp>;

  template <typename _Tp>
    concept unsigned_integral = std::unsigned_integral<_Tp> or std::simd_unsigned_integral<_Tp>;

  template <typename _Tp>
    concept floating_point = std::floating_point<_Tp> or std::simd_floating_point<_Tp>;

  template <typename _Tp>
    concept arithmetic = integral<_Tp> or floating_point<_Tp>;

  template <typename _Tp>
    concept regular = std::regular<_Tp> or std::simd_regular<_Tp>;
}

#endif  // PROTOTYPE_SIMD_META_H_
