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
  namespace __detail
  {
    /* If we add our own common_type we can make 'hypot(holder<simd<float>>, float)' work.

    template <typename...>
      struct common_type;

    template <typename... _Ts>
      using common_type_t = typename common_type<_Ts...>::type;

    template <typename _T0>
      struct common_type<_T0>
      { using type = _T0; };

    template <typename _T0>
      struct common_type<_T0, _T0>
      { using type = _T0; };

    template <typename _T0>
      struct common_type<_T0, _T0, _T0>
      { using type = _T0; };

    template <typename _T0, typename _T1>
      requires requires(_T0 __a, _T1 __b) { select(__a == __b, __a, __b); }
      struct common_type<_T0, _T1>
      { using type = decltype(select(std::declval<_T0>() == std::declval<_T1>(),
                                     std::declval<_T0>(), std::declval<_T1>())); };

    template <typename _T0, typename _T1, typename... _Ts>
      requires requires { typename common_type<typename common_type<_T0, _T1>::type,
                                               typename common_type<_Ts...>::type>::type; }
      struct common_type<_T0, _T1, _Ts...>
      : common_type<typename common_type<_T0, _T1>::type, typename common_type<_Ts...>::type>
      {};
    */

    template <typename _Tp>
      concept __can_deduce_simd = requires(const _Tp& __x) {
        basic_vec(std::declval<_Tp>());
      };

    template <typename _Tp>
      struct __deduced_simd
      { using type = void; };

    template <typename _Tp>
      requires requires(const _Tp& __x) { { __x + __x } -> __valid_simd; }
        and (not __can_deduce_simd<_Tp>)
      struct __deduced_simd<_Tp>
      { using type = decltype(std::declval<const _Tp&>() + std::declval<const _Tp&>()); };

    template <__can_deduce_simd _Tp>
      struct __deduced_simd<_Tp>
      { using type = decltype(basic_vec(std::declval<const _Tp&>())); };

    template <typename _Tp>
      using __deduced_simd_t = typename __deduced_simd<_Tp>::type;

    template <typename... _Ts>
      concept __math_floating_point
        = (SIMD_NSPC::floating_point<__deduced_simd_t<_Ts>> or ...);

    template <typename... _Ts>
      struct __deduced_common_simd;

    template <typename... _Ts>
      requires __math_floating_point<_Ts...>
      using __deduced_common_simd_t = typename __deduced_common_simd<_Ts...>::type;

    template <typename _T0>
      requires __math_floating_point<_T0>
      struct __deduced_common_simd<_T0>
      { using type = __deduced_simd_t<_T0>; };

    template <typename _T0>
      requires (not __math_floating_point<_T0>)
      struct __deduced_common_simd<_T0>
      { using type = _T0; };

    template <typename _T0, typename _T1>
      requires __math_floating_point<_T0, _T1>
      struct __deduced_common_simd<_T0, _T1>
      : std::common_type<typename __deduced_common_simd<_T0>::type,
                         typename __deduced_common_simd<_T1>::type>
      {};

    template <typename _T0, typename _T1, typename _T2, typename... _Ts>
      requires requires { typename __deduced_common_simd<_T0, _T1>::type; }
      struct __deduced_common_simd<_T0, _T1, _T2, _Ts...>
      : std::common_type<typename __deduced_common_simd<_T0, _T1>::type, _T2, _Ts...>
      {};

    template <typename _T0, typename _T1, typename _T2, typename... _Ts>
      requires (not requires { typename __deduced_common_simd<_T0, _T1>::type; })
      struct __deduced_common_simd<_T0, _T1, _T2, _Ts...>
      : std::common_type<typename __deduced_common_simd<_T2, _Ts...>::type, _T0, _T1>
      {};

    template <typename _To, typename _From>
      constexpr
      conditional_t<convertible_to<_From&, _To&> and convertible_to<const _From&, const _To&>,
                    const _To&, _To>
      __cast_if_needed(const _From& __x)
      { return __x; }
  }

  template <__detail::__math_floating_point _V0, auto = __detail::__build_flags()>
    constexpr __detail::__deduced_simd_t<_V0>
    floor(const _V0& __xx)
    {
      using _Vp = __detail::__deduced_simd_t<_V0>;
      const _Vp& __x = __xx;
      return _Vp::_Impl::_S_floor(__data(__x));
    }

  template <typename _V0, typename _V1, auto = __detail::__build_flags()>
    requires __detail::__math_floating_point<_V0, _V1>
    constexpr __detail::__deduced_common_simd_t<_V0, _V1>
    hypot(const _V0& __xx, const _V1& __yy)
    {
      using _Vp = __detail::__deduced_common_simd_t<_V0, _V1>;
      const _Vp& __x = __xx;
      const _Vp& __y = __yy;
      return _Vp::_Impl::_S_hypot(__data(__x), __data(__y));
    }

  template <typename _V0, typename _V1, typename _V2, auto = __detail::__build_flags()>
    requires __detail::__math_floating_point<_V0, _V1, _V2>
    constexpr __detail::__deduced_common_simd_t<_V0, _V1, _V2>
    hypot(const _V0& __xx, const _V1& __yy, const _V2& __zz)
    {
      using _Vp = __detail::__deduced_common_simd_t<_V0, _V1, _V2>;
      const _Vp& __x = __xx;
      const _Vp& __y = __yy;
      const _Vp& __z = __zz;
      return _Vp::_Impl::_S_hypot(__data(__x), __data(__y), __data(__z));
    }
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
  template <SIMD_NSPC::__detail::__math_floating_point _Up>                                        \
    _GLIBCXX_ALWAYS_INLINE constexpr _Up                                                           \
    name(const _Up& __xx)                                                                          \
    {                                                                                              \
      using _Vp = __detail::__deduced_simd_t<_Up>;                                                   \
      const _Vp& __x = __detail::__cast_if_needed<_Vp>(__xx);                                      \
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
