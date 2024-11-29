/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */
#ifndef PROTOTYPE_SIMD_GENERIC_H_
#define PROTOTYPE_SIMD_GENERIC_H_

#include "simd_config.h"
#include "detail.h"
#include "simd_meta.h"

namespace std::simd_generic::scalar
{
  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
  all_of(same_as<bool> auto __x) noexcept
  { return __x; }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
  any_of(same_as<bool> auto __x) noexcept
  { return __x; }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr bool
  none_of(same_as<bool> auto __x) noexcept
  { return not __x; }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
  reduce_count(same_as<bool> auto __x) noexcept
  { return static_cast<__detail::_SimdSizeType>(__x); }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
  reduce_min_index(same_as<bool> auto __x) noexcept
  {
    __glibcxx_simd_precondition(__x, "any_of(x) must be true");
    return 0;
  }

  _GLIBCXX_SIMD_ALWAYS_INLINE constexpr __detail::_SimdSizeType
  reduce_max_index(same_as<bool> auto __x) noexcept
  {
    __glibcxx_simd_precondition(__x, "any_of(x) must be true");
    return 0;
  }

  template <__detail::__vectorizable _Tp, std::invocable<_Tp, _Tp> _BinaryOperation>
    constexpr _Tp
    reduce(const _Tp& __x, _BinaryOperation)
    { return __x; }

  template <__detail::__vectorizable _Tp, std::invocable<_Tp, _Tp> _BinaryOperation>
    constexpr _Tp
    reduce(const _Tp& __x, bool __k, __type_identity_t<_Tp> __identity_element, _BinaryOperation)
    { return __k ? __x : __identity_element; }

  template <__detail::__vectorizable _Tp>
    constexpr _Tp
    reduce(const _Tp& __x, bool __k, plus<>) noexcept
    { return __k ? __x : _Tp(); }

  template <__detail::__vectorizable _Tp>
    constexpr _Tp
    reduce(const _Tp& __x, bool __k, multiplies<>) noexcept
    { return __k ? __x : _Tp(1); }

  template <__detail::__vectorizable _Tp>
    requires std::integral<_Tp>
    constexpr _Tp
    reduce(const _Tp& __x, bool __k, bit_and<>) noexcept
    { return __k ? __x : _Tp(~_Tp()); }

  template <__detail::__vectorizable _Tp>
    requires std::integral<_Tp>
    constexpr _Tp
    reduce(const _Tp& __x, bool __k, bit_or<>) noexcept
    { return __k ? __x : _Tp(); }

  template <__detail::__vectorizable _Tp>
    requires std::integral<_Tp>
    constexpr _Tp
    reduce(const _Tp& __x, bool __k, bit_xor<>) noexcept
    { return __k ? __x : _Tp(); }

  template <__detail::__vectorizable _Tp>
    requires std::totally_ordered<_Tp>
    constexpr _Tp
    reduce_min(_Tp __x) noexcept
    { return __x; }

  template <__detail::__vectorizable _Tp>
    requires std::totally_ordered<_Tp>
    constexpr _Tp
    reduce_min(_Tp __x, bool __k) noexcept
    { return __k ? __x : std::__finite_max_v<_Tp>; }

  template <__detail::__vectorizable _Tp>
    requires std::totally_ordered<_Tp>
    constexpr _Tp
    reduce_max(_Tp __x) noexcept
    { return __x; }

  template <__detail::__vectorizable _Tp>
    requires std::totally_ordered<_Tp>
    constexpr _Tp
    reduce_max(_Tp __x, bool __k) noexcept
    { return __k ? __x : std::__finite_min_v<_Tp>; }

  template <__detail::__vectorizable _Tp, ranges::contiguous_range _Rg, typename... _Flags>
    constexpr _Tp
    load(_Rg&& __range, flags<_Flags...> __flags = {})
    {
      static_assert(__detail::__loadstore_convertible_to<std::ranges::range_value_t<_Rg>,
                                                         _Tp, _Flags...>,
                    "The converting load is not value-preserving. "
                    "Pass 'std::simd::flag_convert' if lossy conversion matches the intent.");

      constexpr bool __allow_out_of_bounds
        = (false or ... or is_same_v<_Flags, __detail::_LoadDefaultInit>);
      static_assert(__detail::__static_range_size<_Rg> > 0
                      or __allow_out_of_bounds
                      or __detail::__static_range_size<_Rg> == dynamic_extent,
                    "Out-of-bounds access: load of 1 value out of range of size 0");

      const auto* __ptr = __flags.template _S_adjust_pointer<_Tp>(std::ranges::data(__range));

      const auto __rg_size = std::ranges::size(__range);
      const bool __out_of_bounds = __rg_size == 0;
      __glibcxx_simd_precondition(__allow_out_of_bounds or not __out_of_bounds,
                                  "Out-of-bounds access: load of 1 value out of range of size 0");

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

  template <__detail::__vectorizable _Tp, contiguous_iterator _First, sentinel_for<_First> _Last,
            typename... _Flags>
    constexpr auto
    load(_First __first, _Last __last, flags<_Flags...> __flags = {})
    { return load<_Tp>(std::span(__first, __last), __flags); }

  template <__detail::__vectorizable _Tp, contiguous_iterator _First, typename... _Flags>
    constexpr auto
    load(_First __first, size_t __size, flags<_Flags...> __flags = {})
    { return load<_Tp>(std::span(__first, __size), __flags); }

  template <__detail::__vectorizable _Tp, ranges::contiguous_range _Rg, typename... _Flags>
    constexpr void
    store(const _Tp& __x, _Rg&& __range, flags<_Flags...> __flags = {})
    {
      static_assert(__detail::__loadstore_convertible_to<
                      _Tp, std::ranges::range_value_t<_Rg>, _Flags...>,
                    "The converting store is not value-preserving. "
                    "Pass 'std::simd::flag_convert' if lossy conversion matches the intent.");

      constexpr bool __allow_out_of_bounds
        = (false or ... or is_same_v<_Flags, __detail::_LoadDefaultInit>);
      static_assert(__detail::__static_range_size<_Rg> > 0
                      or __allow_out_of_bounds
                      or __detail::__static_range_size<_Rg> == dynamic_extent,
                    "Out-of-bounds access: store into an empty output range");

      const auto* __ptr = __flags.template _S_adjust_pointer<_Tp>(std::ranges::data(__range));

      const auto __rg_size = std::ranges::size(__range);
      const bool __out_of_bounds = __rg_size == 0;
      // "Out-of-bounds access: store of 1 value into a range of size 0"
      __glibcxx_simd_precondition(__allow_out_of_bounds or not __out_of_bounds,
                                  "output range is too small. "
                                  "Consider passing 'std::simd::flag_allow_partial_store'.");

      if constexpr (__detail::__static_range_size<_Rg> != dynamic_extent
                      and __detail::__static_range_size<_Rg> > 0)
        *__ptr = static_cast<_Tp>(__x);
      else if (__rg_size > 0)
        *__ptr = static_cast<_Tp>(__x);
#ifdef UB_LOADS
      else if (not __allow_out_of_bounds)
        __detail::__invoke_ub("Out-of-bounds access: load of 1 value out of range of size 0");
#endif
    }

  template <__detail::__vectorizable _Tp, __detail::__simd_generator_invokable<_Tp, 1> _Fp>
    constexpr _Tp
    generate(_Fp&& __gen)
    { return _Tp(__gen(__detail::__ic<0>)); }
}

namespace std::simd_generic
{
  template <typename _Tp, typename _Up = __detail::__value_type_of<_Tp>>
    struct simd_alignment : std::simd_alignment<_Tp, _Up>
    {};

  template <__detail::__vectorizable _Tp, typename _Up>
    struct simd_alignment<_Tp, _Up>
    : integral_constant<size_t, alignof(_Up)>
    {};

  template <typename _Tp, typename _Up = __detail::__value_type_of<_Tp>>
    inline constexpr size_t simd_alignment_v = simd_alignment<_Tp, _Up>::value;


  template <typename _Tp, typename _Vp>
    struct rebind_simd : std::rebind_simd<_Tp, _Vp>
    {};

  template <__detail::__vectorizable _Tp, __detail::__vectorizable _Up>
    struct rebind_simd<_Tp, _Up>
    { using type = _Tp; };

  template <__detail::__vectorizable _Tp>
    struct rebind_simd<_Tp, bool>
    { using type = bool; };

  template <typename _Tp, typename _Vp>
    using rebind_simd_t = typename rebind_simd<_Tp, _Vp>::type;


  template <__detail::_SimdSizeType _Np, typename _Vp>
    struct resize_simd : std::resize_simd<_Np, _Vp>
    {};

  template <__detail::_SimdSizeType _Np, __detail::__vectorizable _Tp>
    struct resize_simd<_Np, _Tp>
    { using type = std::simd<_Tp, _Np>; };

  template <__detail::__vectorizable _Tp>
    struct resize_simd<1, _Tp>
    { using type = _Tp; };

  template <>
    struct resize_simd<1, bool>
    { using type = bool; };

  template <__detail::_SimdSizeType _Np, typename _Vp>
    using resize_simd_t = typename resize_simd<_Np, _Vp>::type;


  template <typename _Tp>
    concept integral = std::integral<_Tp> or std::integral<_Tp>;

  template <typename _Tp>
    concept signed_integral = std::signed_integral<_Tp> or std::signed_integral<_Tp>;

  template <typename _Tp>
    concept unsigned_integral = std::unsigned_integral<_Tp> or std::unsigned_integral<_Tp>;

  template <typename _Tp>
    concept floating_point = std::floating_point<_Tp> or std::floating_point<_Tp>;

  template <typename _Tp>
    concept arithmetic = integral<_Tp> or floating_point<_Tp>;

  template <typename _Tp>
    concept regular = std::regular<_Tp> or std::regular<_Tp>;
}

#endif  // PROTOTYPE_SIMD_GENERIC_H_
