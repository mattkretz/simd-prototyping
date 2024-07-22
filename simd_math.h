/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_MATH_H_
#define PROTOTYPE_SIMD_MATH_H_

#include "simd.h"
#include <cmath>

#define _GLIBCXX_SIMD_HAVE_VECTORMATH_ABI 1

#if _GLIBCXX_SIMD_HAVE_VECTORMATH_ABI
#if _GLIBCXX_SIMD_HAVE_AVX512F
#define _GLIBCXX_SIMD_VECTORABI_ISA e
#elif _GLIBCXX_SIMD_HAVE_AVX2
#define _GLIBCXX_SIMD_VECTORABI_ISA d
#elif _GLIBCXX_SIMD_HAVE_AVX
#define _GLIBCXX_SIMD_VECTORABI_ISA c
#elif _GLIBCXX_SIMD_HAVE_SSE
#define _GLIBCXX_SIMD_VECTORABI_ISA b
#endif

#define _GLIBCXX_SIMD_VECMATH_NAME_IMPL2(a, b, c, d, e) \
  _ZGV ## a ## N ## b ## c ## d ## e

#define _GLIBCXX_SIMD_VECMATH_NAME_IMPL(a, b, c, d, e) \
  _GLIBCXX_SIMD_VECMATH_NAME_IMPL2(a, b, c, d, e)

#define _GLIBCXX_SIMD_VECMATH_NAME(vlen, vparms, name, suffix) \
  _GLIBCXX_SIMD_VECMATH_NAME_IMPL(_GLIBCXX_SIMD_VECTORABI_ISA, vlen, vparms, name, suffix)

extern "C" {
#define _GLIBCXX_SIMD_VECMATH_DECL_1ARG(name)                                                      \
  extern SIMD_NSPC::__detail::__v16float                                                           \
  _GLIBCXX_SIMD_VECMATH_NAME(16, v, name, f)(SIMD_NSPC::__detail::__v16float);                     \
                                                                                                   \
  extern SIMD_NSPC::__detail::__v8float                                                            \
  _GLIBCXX_SIMD_VECMATH_NAME(8, v, name, f)(SIMD_NSPC::__detail::__v8float);                       \
                                                                                                   \
  extern SIMD_NSPC::__detail::__v4float                                                            \
  _GLIBCXX_SIMD_VECMATH_NAME(4, v, name, f)(SIMD_NSPC::__detail::__v4float);                       \
                                                                                                   \
  extern SIMD_NSPC::__detail::__v8double                                                           \
  _GLIBCXX_SIMD_VECMATH_NAME(8, v, name, )(SIMD_NSPC::__detail::__v8double);                       \
                                                                                                   \
  extern SIMD_NSPC::__detail::__v4double                                                           \
  _GLIBCXX_SIMD_VECMATH_NAME(4, v, name, )(SIMD_NSPC::__detail::__v4double);                       \
                                                                                                   \
  extern SIMD_NSPC::__detail::__v2double                                                           \
  _GLIBCXX_SIMD_VECMATH_NAME(2, v, name, )(SIMD_NSPC::__detail::__v2double)

#define _GLIBCXX_SIMD_VECMATH_DECL_2ARG(name)                                                      \
  extern SIMD_NSPC::__detail::__v16float                                                           \
  _GLIBCXX_SIMD_VECMATH_NAME(16, vv, name, f)(SIMD_NSPC::__detail::__v16float,                     \
                                              SIMD_NSPC::__detail::__v16float);                    \
                                                                                                   \
  extern SIMD_NSPC::__detail::__v8float                                                            \
  _GLIBCXX_SIMD_VECMATH_NAME(8, vv, name, f)(SIMD_NSPC::__detail::__v8float,                       \
                                             SIMD_NSPC::__detail::__v8float);                      \
                                                                                                   \
  extern SIMD_NSPC::__detail::__v4float                                                            \
  _GLIBCXX_SIMD_VECMATH_NAME(4, vv, name, f)(SIMD_NSPC::__detail::__v4float,                       \
                                             SIMD_NSPC::__detail::__v4float);                      \
                                                                                                   \
  extern SIMD_NSPC::__detail::__v8double                                                           \
  _GLIBCXX_SIMD_VECMATH_NAME(8, vv, name, )(SIMD_NSPC::__detail::__v8double,                       \
                                            SIMD_NSPC::__detail::__v8double);                      \
                                                                                                   \
  extern SIMD_NSPC::__detail::__v4double                                                           \
  _GLIBCXX_SIMD_VECMATH_NAME(4, vv, name, )(SIMD_NSPC::__detail::__v4double,                       \
                                            SIMD_NSPC::__detail::__v4double);                      \
                                                                                                   \
  extern SIMD_NSPC::__detail::__v2double                                                           \
  _GLIBCXX_SIMD_VECMATH_NAME(2, vv, name, )(SIMD_NSPC::__detail::__v2double,                       \
                                            SIMD_NSPC::__detail::__v2double)

  _GLIBCXX_SIMD_VECMATH_DECL_1ARG(exp);
  _GLIBCXX_SIMD_VECMATH_DECL_1ARG(sin);
  _GLIBCXX_SIMD_VECMATH_DECL_1ARG(cos);
  _GLIBCXX_SIMD_VECMATH_DECL_1ARG(tan);
  _GLIBCXX_SIMD_VECMATH_DECL_1ARG(log);
  _GLIBCXX_SIMD_VECMATH_DECL_1ARG(log2);
  _GLIBCXX_SIMD_VECMATH_DECL_1ARG(log10);

  _GLIBCXX_SIMD_VECMATH_DECL_2ARG(pow);
}
#endif

namespace SIMD_NSPC
{
  template <SIMD_NSPC::floating_point _Vp, auto = __detail::__build_flags()>
    constexpr _Vp
    floor(const _Vp& __x)
    { return _Vp(_Vp::_Impl::_S_floor(__data(__x))); }
}


#define _GLIBCXX_SIMD_MATH_1ARG(name)                                                              \
namespace SIMD_NSPC                                                                                \
{                                                                                                  \
  namespace __detail                                                                               \
  {                                                                                                \
    template <__vec_builtin _Vp>                                                                   \
      [[gnu::optimize("-O3", "-fno-math-errno")]] _Vp                                              \
      __##name##_loop(_Vp __v)                                                                     \
      {                                                                                            \
        for (int __i = 0; __i < __width_of<_Vp>; ++__i)                                            \
          __v[__i] = std::name(__v[__i]);                                                          \
        return __v;                                                                                \
      }                                                                                            \
  }                                                                                                \
                                                                                                   \
  template <SIMD_NSPC::floating_point _Vp>                                                         \
    _GLIBCXX_ALWAYS_INLINE constexpr _Vp                                                           \
    name(const _Vp& __x)                                                                           \
    {                                                                                              \
      using _Tp [[maybe_unused]] = typename _Vp::value_type;                                       \
      if (__builtin_is_constant_evaluated() or __x._M_is_constprop())                              \
        return _Vp([&] (int __i) { return std::name(__x[__i]); });                                 \
      else if constexpr (sizeof(_Vp) == 64 and sizeof(_Tp) == 4)                                   \
        return _Vp(::_GLIBCXX_SIMD_VECMATH_NAME(16, v, name, f)(__data(__x)));                     \
      else if constexpr (sizeof(_Vp) == 32 and sizeof(_Tp) == 4)                                   \
        return _Vp(::_GLIBCXX_SIMD_VECMATH_NAME(8, v, name, f)(__data(__x)));                      \
      else if constexpr (sizeof(_Vp) == 16 and sizeof(_Tp) == 4)                                   \
        return _Vp(::_GLIBCXX_SIMD_VECMATH_NAME(4, v, name, f)(__data(__x)));                      \
      else if constexpr (sizeof(_Vp) == 64 and sizeof(_Tp) == 8)                                   \
        return _Vp(::_GLIBCXX_SIMD_VECMATH_NAME(8, v, name,)(__data(__x)));                        \
      else if constexpr (sizeof(_Vp) == 32 and sizeof(_Tp) == 8)                                   \
        return _Vp(::_GLIBCXX_SIMD_VECMATH_NAME(4, v, name,)(__data(__x)));                        \
      else if constexpr (sizeof(_Vp) == 16 and sizeof(_Tp) == 8)                                   \
        return _Vp(::_GLIBCXX_SIMD_VECMATH_NAME(2, v, name,)(__data(__x)));                        \
      else                                                                                         \
        return _Vp(__detail::__##name##_loop(__data(__x)));                                        \
    }                                                                                              \
                                                                                                   \
  template <std::floating_point _Tp, __detail::__simd_abi_tag _Abi0, __detail::_SimdSizeType _Np>  \
    requires __detail::__vectorizable<_Tp>                                                         \
    constexpr basic_vec<_Tp, _AbiArray<_Abi0, _Np>>                                                \
    name(const basic_vec<_Tp, _AbiArray<_Abi0, _Np>>& __x)                                         \
    {                                                                                              \
      basic_vec<_Tp, _AbiArray<_Abi0, _Np>> __r;                                                   \
      const auto& __arr = __data(__x);                                                             \
      std::transform(__arr.begin(), __arr.end(), __data(__r).begin(),                              \
                     [] [[gnu::always_inline]] (auto __v) {                                        \
                       return __data(name(basic_vec<_Tp, _Abi0>(__v)));                            \
                     });                                                                           \
      return __r;                                                                                  \
    }                                                                                              \
                                                                                                   \
  template <std::floating_point _Tp, __detail::_SimdSizeType _Np, typename _Tag>                   \
    requires __detail::__vectorizable<_Tp>                                                         \
    constexpr basic_vec<_Tp, _AbiCombine<_Np, _Tag>>                                               \
    name(const basic_vec<_Tp, _AbiCombine<_Np, _Tag>>& __x)                                        \
    {                                                                                              \
      using _Tup = typename _AbiCombine<_Np, _Tag>::template _SimdMember<_Tp>;                     \
      return _Tup::_S_generate_pervec([&] [[gnu::always_inline]] (vir::constexpr_value auto __i) { \
               return __data(name(__data(__x)._M_simd_at(__i)));                                   \
             });                                                                                   \
    }                                                                                              \
}                                                                                                  \
                                                                                                   \
namespace std                                                                                      \
{                                                                                                  \
  using SIMD_NSPC::name;                                                                           \
}

_GLIBCXX_SIMD_MATH_1ARG(exp)

#endif  // PROTOTYPE_SIMD_MATH_H_
