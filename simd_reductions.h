/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_REDUCTIONS_H_
#define PROTOTYPE_SIMD_REDUCTIONS_H_

#include "simd.h"
#include "simd_split.h"

namespace std
{
  namespace __detail
  {
    template <typename _Tp, typename _BinaryOperation>
      inline constexpr auto
      __identity_element_for = [] {
        if constexpr (same_as<_BinaryOperation, plus<>>)
          return _Tp(0);
        else if constexpr (same_as<_BinaryOperation, multiplies<>>)
          return _Tp(1);
        else if constexpr (same_as<_BinaryOperation, bit_and<>>)
          return _Tp(~_Tp());
        else if constexpr (same_as<_BinaryOperation, bit_or<>>)
          return _Tp(0);
        else if constexpr (same_as<_BinaryOperation, bit_xor<>>)
          return _Tp(0);
        else
          return nullptr;
      }();

    template <typename _Tp, typename _Abi, typename _BinaryOperation>
#if P1928
      constexpr std::resize_simd_t<std::simd_size_v<_Tp, _Abi> / 2, basic_simd<_Tp, _Abi>>
#else
      constexpr auto
#endif
      __split_and_invoke_once(const basic_simd<_Tp, _Abi>& __x, _BinaryOperation __binary_op)
      {
        using _V1 = basic_simd<_Tp, _Abi>;
        static_assert(std::__has_single_bit(unsigned(_V1::size.value)));
        using _V2 = std::resize_simd_t<_V1::size.value / 2, _V1>;
        const auto [__x0, __x1] = std::simd_split<_V2>(__x);
        // Mandates: binary_op can be invoked with two arguments of type basic_simd<T, A1>
        // returning basic_simd<T, A1> for every A1 that is an ABI tag type.
        //   -- exploration:
        // returning basic_simd<U, A2> with vectorizable U and simd-size<U, A2> == simd-size<T, A1>
        static_assert(requires {
#if P1928
          { __binary_op(__x0, __x1) } -> same_as<_V2>;
#else
          { __binary_op(__x0, __x1) } -> __simd_type<_V2::size()>;
#endif
        });
        return __binary_op(__x0, __x1);
      }
  }

  // Mandates: binary_op can be invoked with two arguments of type basic_simd<T, A1>
  // returning basic_simd<T, A1> for every A1 that is an ABI tag type.
  //   -- Exploration:
  // Mandates: binary_op can be invoked with two arguments of type basic_simd<T1, A1> and
  // basic_simd<T2, A2> where simd-size
  // returning basic_simd<T, A1> for every A1 that is an ABI tag type.
  template <typename _Tp, typename _Abi,
            std::invocable<simd<_Tp, 1>, simd<_Tp, 1>> _BinaryOperation>
#ifdef P1928
    constexpr _Tp
#else
    constexpr auto
#endif
    reduce(const basic_simd<_Tp, _Abi>& __x, _BinaryOperation __binary_op)
    {
      using _V1 = basic_simd<_Tp, _Abi>;

      if constexpr (requires{__detail::_SimdTraits<_Tp, _Abi>::_SimdImpl::_S_reduce(
                               __x, __binary_op);} and _V1::size.value > 1)
        {
          if (not __builtin_is_constant_evaluated() and not __x._M_is_constprop())
            return __detail::_SimdTraits<_Tp, _Abi>::_SimdImpl::_S_reduce(__x, __binary_op);
        }

      if constexpr (_V1::size.value == 1)
        return _Tp(__x[0]);

      else if constexpr (_V1::size.value == 2 and sizeof(__x) == 16)
        return __binary_op(__x, _V1([&](auto __i) { return __x[__i ^ 1]; }))[0];

      else if constexpr (std::__has_single_bit(_V1::size.value))
        return std::reduce(__detail::__split_and_invoke_once(__x, __binary_op), __binary_op);

      else
        {
          constexpr int __left_size = std::__bit_floor(_V1::size.value);
          constexpr int __right_size = _V1::size.value - __left_size;
          constexpr int __max_size = std::__bit_ceil(_V1::size.value);
          constexpr int __missing = __max_size - _V1::size.value;
          if constexpr (sizeof(_V1) == sizeof(std::resize_simd_t<__max_size, _V1>)
                          and (__missing < __right_size)
                          and not same_as<decltype(__detail::__identity_element_for
                                                     <_Tp, _BinaryOperation>), nullptr_t>)
            {
              using _V2 = std::resize_simd_t<__max_size, _V1>;
              constexpr std::simd<_Tp, __missing> __padding
                = __detail::__identity_element_for<_Tp, _BinaryOperation>;
              const _V2 __y = std::simd_cat(__x, __padding);
              return std::reduce(__detail::__split_and_invoke_once(__y, __binary_op), __binary_op);
            }

          using _V2 = std::resize_simd_t<__left_size, _V1>;
          const auto [__x0, __x1] = std::simd_split<_V2>(__x);
          return std::reduce(
                   std::simd_cat(__detail::__split_and_invoke_once(__x0, __binary_op), __x1),
                   __binary_op);
        }
    }

  template <typename _Tp, typename _Abi,
            std::invocable<simd<_Tp, 1>, simd<_Tp, 1>> _BinaryOperation>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           __type_identity_t<_Tp> __identity_element, _BinaryOperation __binary_op)
    { return reduce(simd_select(__k, __x, __identity_element), __binary_op); }

  template <typename _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           plus<>) noexcept
    { return reduce(simd_select(__k, __x, _Tp())); }

  template <typename _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           multiplies<> __binary_op) noexcept
    { return reduce(simd_select(__k, __x, _Tp(1)), __binary_op); }

  template <std::integral _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           bit_and<> __binary_op) noexcept
    { return reduce(simd_select(__k, __x, _Tp(~_Tp())), __binary_op); }

  template <std::integral _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           bit_or<> __binary_op) noexcept
    { return reduce(simd_select(__k, __x, _Tp()), __binary_op); }

  template <std::integral _Tp, typename _Abi>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, const typename basic_simd<_Tp, _Abi>::mask_type& __k,
           bit_xor<> __binary_op) noexcept
    { return reduce(simd_select(__k, __x, _Tp()), __binary_op); }

  // NaN inputs are precondition violations (_Tp satisfies and models totally_ordered)
  template <std::totally_ordered _Tp, typename _Abi>
    constexpr _Tp
    reduce_min(const basic_simd<_Tp, _Abi>& __x) noexcept
    {
      return reduce(__x, []<simd_totally_ordered _UV>(const _UV& __a, const _UV& __b) {
               return simd_select(__a < __b, __a, __b);
             });
    }

  template <std::totally_ordered _Tp, typename _Abi>
    constexpr _Tp
    reduce_min(const basic_simd<_Tp, _Abi>& __x,
               const typename basic_simd<_Tp, _Abi>::mask_type& __k) noexcept
    {
      return reduce(simd_select(__k, __x, std::__finite_max_v<_Tp>),
                    []<simd_totally_ordered _UV>(const _UV& __a, const _UV& __b) {
                      return simd_select(__a < __b, __a, __b);
                    });
    }

  template <std::totally_ordered _Tp, typename _Abi>
    constexpr _Tp
    reduce_max(const basic_simd<_Tp, _Abi>& __x) noexcept
    {
      return reduce(__x, []<simd_totally_ordered _UV>(const _UV& __a, const _UV& __b) {
               return simd_select(__a < __b, __b, __a);
             });
    }

  template <std::totally_ordered _Tp, typename _Abi>
    constexpr _Tp
    reduce_max(const basic_simd<_Tp, _Abi>& __x,
               const typename basic_simd<_Tp, _Abi>::mask_type& __k) noexcept
    {
      return reduce(simd_select(__k, __x, std::__finite_min_v<_Tp>),
                    []<simd_totally_ordered _UV>(const _UV& __a, const _UV& __b) {
                      return simd_select(__a < __b, __b, __a);
                    });
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Extensions.
////////////////////////////////////////////////////////////////////////////////////////////////////
// reduce, reduce_min, reduce_max have no overloads for scalars and thus can't be used in
// SIMD-generic code. This could fix it:
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std::simd_generic
{
  using std::reduce;
  using std::reduce_min;
  using std::reduce_max;

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
}

#endif  // PROTOTYPE_SIMD_REDUCTIONS_H_
