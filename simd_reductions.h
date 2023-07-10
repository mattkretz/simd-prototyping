/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
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
    constexpr std::resize_simd_t<std::simd_size_v<_Tp, _Abi> / 2, basic_simd<_Tp, _Abi>>
    __split_and_invoke_once(const basic_simd<_Tp, _Abi>& __x, _BinaryOperation __binary_op)
    {
      using _V1 = basic_simd<_Tp, _Abi>;
      static_assert(std::__has_single_bit(unsigned(_V1::size.value)));
      using _V2 = std::resize_simd_t<_V1::size.value / 2, _V1>;
      const auto [__x0, __x1] = std::simd_split<_V2>(__x);
      // Mandates: binary_op can be invoked with two arguments of type basic_simd<_Tp, A1>
      // returning basic_simd<_Tp, A1> for every A1 that is an ABI tag type.
      static_assert(requires {
        { std::invoke(__binary_op, __x0, __x1) } -> same_as<_V2>;
      });
      return std::invoke(__binary_op, __x0, __x1);
    }
  }

  template <typename _Tp, typename _Abi, typename _BinaryOperation = plus<>>
    constexpr _Tp
    reduce(const basic_simd<_Tp, _Abi>& __x, _BinaryOperation __binary_op = {})
    {
      using _V1 = basic_simd<_Tp, _Abi>;
      if constexpr (std::__has_single_bit(_V1::size.value))
        {
          if (not __builtin_is_constant_evaluated() and not __x._M_is_constprop())
            {
#if _GLIBCXX_SIMD_HAVE_SSE
              if constexpr (same_as<_BinaryOperation, plus<>> and _V1::size.value > 1
                              and __detail::__have_ssse3 and is_integral_v<_Tp>)
                {
                  auto __y = __detail::__vector_bitcast<
                               __detail::__int_for_sizeof_t<_Tp>>(__detail::__as_simd_builtin(__x));

                  if constexpr (_V1::size.value == 16 and sizeof(_Tp) == 1)
                    {
                      auto __z = __builtin_bit_cast(__detail::__vint16_128, __y);
                      __z = (__z << 8) + __z;
                      __z = __builtin_ia32_phaddw128(__z, __z);
                      __z = __builtin_ia32_phaddw128(__z, __z);
                      __z = __builtin_ia32_phaddw128(__z, __z);
                      return __z[0] >> 8;
                    }

                  if constexpr (_V1::size.value == 8 and sizeof(_Tp) == 2)
                    {
                      __y = __builtin_ia32_phaddw128(__y, __y);
                      __y = __builtin_ia32_phaddw128(__y, __y);
                      return __builtin_ia32_phaddw128(__y, __y)[0];
                    }

                  if constexpr (_V1::size.value == 4 and sizeof(_Tp) == 2 and sizeof(__x) == 8)
                    {
                      auto __z = __builtin_shufflevector(__y, __y, 0, 1, 2, 3, -1, -1, -1, -1);
                      __z = __builtin_ia32_phaddw128(__z, __z);
                      return __builtin_ia32_phaddw128(__z, __z)[0];
                    }

                  if constexpr (_V1::size.value == 4 and sizeof(_Tp) == 4)
                    {
                      __y = __builtin_ia32_phaddd128(__y, __y);
                      return __builtin_ia32_phaddd128(__y, __y)[0];
                    }
                }
              else if constexpr (same_as<_BinaryOperation, plus<>> and _V1::size.value > 1
                                   and __detail::__have_sse3 and is_floating_point_v<_Tp>)
                {
                  auto __y = __detail::__as_simd_builtin(__x);
                  if constexpr (_V1::size.value == 4 and sizeof(_Tp) == 4)
                    {
                      __y = __builtin_ia32_haddps(__y, __y);
                      return __builtin_ia32_haddps(__y, __y)[0];
                    }

                  if constexpr (_V1::size.value == 2 and sizeof(_Tp) == 8)
                    return __builtin_ia32_haddpd(__y, __y)[0];
                }
#endif
            }

          if constexpr (_V1::size.value == 1)
            return __x[0];

          else if constexpr (_V1::size.value == 2 and sizeof(__x) == 16)
            return __binary_op(__x, _V1([&](auto __i) { return __x[__i ^ 1]; }))[0];

          else
            return std::reduce(__detail::__split_and_invoke_once(__x, __binary_op), __binary_op);
        }
      else
        {
          constexpr int __left_size = std::__bit_floor(_V1::size.value);
          constexpr int __right_size = _V1::size.value - __left_size;
          constexpr int __max_size = std::__bit_ceil(_V1::size.value);
          constexpr int __missing = __max_size - _V1::size.value;
          if constexpr (sizeof(_V1) == sizeof(std::resize_simd_t<__max_size, _V1>)
                          and __missing < __right_size
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
}

#endif  // PROTOTYPE_SIMD_REDUCTIONS_H_
