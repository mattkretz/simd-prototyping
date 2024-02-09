/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_DETAIL_H_
#define PROTOTYPE_DETAIL_H_

#include "fwddecl.h"
#include "constexpr_wrapper.h"

#include <concepts>
#include <cstdint>
#include <limits>
#include <ranges>

namespace std::__detail
{
  template <typename _Tp>
    struct __assert_unreachable
    { static_assert(!is_same_v<_Tp, _Tp>, "this should be unreachable"); };

  template <typename _Tp, typename>
    struct __make_dependent
    { using type = _Tp; };

  template <typename _Tp, typename _Up>
    using __make_dependent_t = typename __make_dependent<_Tp, _Up>::type;

  /**@internal
   * Helper __may_alias<_Tp> that turns _Tp into the type to be used for an
   * aliasing pointer. This adds the __may_alias attribute to _Tp (with compilers
   * that support it).
   */
  template <typename _Tp>
    using __may_alias [[__gnu__::__may_alias__]] = _Tp;

  /**
   * @internal
   * Tag used for private init constructor of simd and simd_mask
   */
  inline constexpr struct _PrivateInit {} __private_init = {};

  template <typename _Tp, typename _Fp>
    static void
    _S_bit_iteration(_Tp __mask, _Fp&& __f)
    {
      static_assert(sizeof(0ULL) >= sizeof(_Tp));
      conditional_t<sizeof(_Tp) <= sizeof(0u), unsigned, unsigned long long> __k;
      if constexpr (is_convertible_v<_Tp, decltype(__k)>)
        __k = __mask;
      else
        __k = __mask.to_ullong();
      while(__k)
        {
          __f(std::__countr_zero(__k));
          __k &= (__k - 1);
        }
    }


#ifdef math_errhandling
  // Determine if math functions must raise floating-point exceptions.
  // math_errhandling may expand to an extern symbol, in which case we must assume fp exceptions
  // need to be considered.
  template <int __me = math_errhandling>
    consteval bool
    __handle_fpexcept_impl(int)
    { return __me & MATH_ERREXCEPT; }

  // Fallback if math_errhandling doesn't work: implement correct exception behavior.
  consteval bool
  __handle_fpexcept_impl(float)
  { return true; }
#endif

  struct _FloatingPointFlags
  {
    std::uint_least64_t _M_handle_fp_exceptions : 1
#if __NO_TRAPPING_MATH__ or __FAST_MATH__
      = 1;
#elif defined math_errhandling
      = __handle_fpexcept_impl(0);
#else
      = 0;
#endif

    std::uint_least64_t _M_fast_math : 1
#if __FAST_MATH__
      = 1;
#else
      = 0;
#endif

    std::uint_least64_t _M_finite_math_only : 1
#if __FINITE_MATH_ONLY__
      = 1;
#else
      = 0;
#endif

    std::uint_least64_t _M_signed_zeros : 1
#if __NO_SIGNED_ZEROS__
      = 0;
#else
      = 1;
#endif

    std::uint_least64_t _M_reciprocal_math : 1
#if __RECIPROCAL_MATH__
      = 1;
#else
      = 0;
#endif

    std::uint_least64_t _M_math_errno : 1
#if __NO_MATH_ERRNO__
      = 0;
#else
      = 1;
#endif

    std::uint_least64_t _M_associative_math : 1
#if __ASSOCIATIVE_MATH__
      = 1;
#else
      = 0;
#endif

    std::uint_least64_t _M_float_eval_method : 2
#if __FLT_EVAL_METHOD__ == 0
      = 0;
#elif __FLT_EVAL_METHOD__ == 1
      = 1;
#elif __FLT_EVAL_METHOD__ == 2
      = 2;
#else
      = 3;
#endif
  };

  static_assert(sizeof(_FloatingPointFlags) == sizeof(std::uint_least64_t));

  struct _MachineFlags;

  template <typename... _Flags>
    struct _BuildFlags : _Flags...
    {};

  /**@internal
   * You must use this type as template argument to function templates that are not declared
   * always_inline (to avoid issues when linking code compiled with different compiler flags).
   */
  using __build_flags = _BuildFlags<_FloatingPointFlags, _MachineFlags>;
}

namespace std
{
  namespace __detail
  {
    template <typename... _Args>
      [[noreturn]] _GLIBCXX_SIMD_ALWAYS_INLINE inline void
      __invoke_ub([[maybe_unused]] const char* __msg, [[maybe_unused]] const _Args&... __args)
      {
#ifdef _GLIBCXX_DEBUG_UB
        __builtin_fprintf(stderr, __msg, __args...);
        __builtin_fprintf(stderr, "\n");
        __builtin_trap();
#else
        __builtin_unreachable();
#endif
      }

    struct _InvalidTraits
    {
      struct _Unusable
      {
        _Unusable() = delete;
        _Unusable(const _Unusable&) = delete;
        _Unusable& operator=(const _Unusable&) = delete;
        ~_Unusable() = delete;
      };

      using _IsValid = false_type;

      static constexpr int _S_size = 0;
      static constexpr int _S_full_size = 0;
      static constexpr bool _S_is_partial = false;

      static constexpr size_t _S_simd_align = 1;
      struct _SimdImpl;
      using _SimdMember = _Unusable;
      struct _SimdCastType;

      static constexpr size_t _S_mask_align = 1;
      struct _MaskImpl;
      using _MaskMember = _Unusable;
      struct _MaskCastType;
    };

    template <typename _Tp, typename _Abi, auto = __build_flags()>
      struct _SimdTraits
      : _InvalidTraits
      {};

    template <typename _Tp, typename _Abi, auto _Flags>
      requires (_Abi::template _IsValid<_Tp>::value)
      struct _SimdTraits<_Tp, _Abi, _Flags>
      : _Abi::template __traits<_Tp>
      {};

    /**
     * Masks need to be different for AVX without AVX2.
     */
    template <size_t _Bs, typename _Abi, auto _Flags = __build_flags()>
      struct _SimdMaskTraits
      : _SimdTraits<__mask_integer_from<_Bs>, _Abi, _Flags>
      {};

    template <typename _Tp>
      concept __arithmetic = integral<_Tp> || floating_point<_Tp>;

    template <typename _Tp>
      concept __vectorizable = __is_vectorizable<_Tp>::value;

    template <_SimdSizeType... _Is>
      using _SimdIndexSequence = std::integer_sequence<_SimdSizeType, _Is...>;

    template <_SimdSizeType _Np>
      using _MakeSimdIndexSequence = std::make_integer_sequence<_SimdSizeType, _Np>;

    template <typename _Rg>
      constexpr inline size_t
      __static_range_size = [] -> size_t {
        using _Tp = std::remove_cvref_t<_Rg>;
        if constexpr (requires { typename std::integral_constant<size_t, _Tp::size()>; })
          return _Tp::size();
        else if constexpr (requires { typename std::integral_constant<size_t, _Tp::extent>; })
          return _Tp::extent;
        else if constexpr (std::extent_v<_Tp> > 0)
          return std::extent_v<_Tp>;
        else if constexpr (requires { typename std::integral_constant<
                                        size_t, std::tuple_size<_Tp>::value>; })
          {
            if constexpr (std::tuple_size_v<_Tp> >= 1
                            && std::same_as<std::tuple_element_t<0, _Tp>,
                                            std::ranges::range_value_t<_Rg>>)
              return std::tuple_size_v<_Tp>;
          }
        return std::dynamic_extent;
      }();

    _GLIBCXX_SIMD_INTRINSIC _SimdSizeType
    __lowest_bit(std::integral auto __bits)
    {
      if constexpr (sizeof(__bits) <= sizeof(int))
        return __builtin_ctz(__bits);
      else if constexpr (sizeof(__bits) <= sizeof(long))
        return __builtin_ctzl(__bits);
      else if constexpr (sizeof(__bits) <= sizeof(long long))
        return __builtin_ctzll(__bits);
      else
        __assert_unreachable<decltype(__bits)>();
    }

    _GLIBCXX_SIMD_INTRINSIC _SimdSizeType
    __highest_bit(std::integral auto __bits)
    {
      if constexpr (sizeof(__bits) <= sizeof(int))
        return sizeof(int) * __CHAR_BIT__ - 1 - __builtin_clz(__bits);
      else if constexpr (sizeof(__bits) <= sizeof(long))
        return sizeof(long) * __CHAR_BIT__ - 1 - __builtin_clzl(__bits);
      else if constexpr (sizeof(__bits) <= sizeof(long long))
        return sizeof(long long) * __CHAR_BIT__ - 1 - __builtin_clzll(__bits);
      else
        __assert_unreachable<decltype(__bits)>();
    }

    template <auto _Value>
      using _Ic = integral_constant<std::remove_const_t<decltype(_Value)>, _Value>;

    template <auto _Value>
      inline constexpr _Ic<_Value> __ic{};

    template <typename _Abi>
      concept __simd_abi_tag
        = not _Abi::template __traits<void>::_IsValid::value
            and requires { typename _Abi::_IsValidAbiTag; };

    template <typename _Abi, typename _Tp>
      concept __valid_abi_tag
        = __simd_abi_tag<_Abi> and _Abi::template __traits<_Tp>::_IsValid::value;

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
      _Ic<(__broadcast_constructible<decltype(declval<_Fp>()(__ic<_Is>)), _Tp> and ...)>
      __simd_broadcast_invokable_impl(_SimdIndexSequence<_Is...>);

    template <typename _Fp, typename _Tp, _SimdSizeType _Np>
      concept __simd_broadcast_invokable = requires
      {
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
  }
}

namespace std::__detail
{
  template <typename _Up, typename _Accessor = _Up,
            typename _ValueType = typename _Up::value_type>
    class _SmartReference
    {
      friend _Accessor;
      _SimdSizeType _M_index;
      _Up& _M_obj;

      _GLIBCXX_SIMD_INTRINSIC constexpr _ValueType
      _M_read() const noexcept
      { return _Accessor::_S_get(_M_obj, _M_index); }

      template <typename _Tp>
        _GLIBCXX_SIMD_INTRINSIC constexpr void
        _M_write(_Tp&& __x) const
        { _Accessor::_S_set(_M_obj, _M_index, static_cast<_Tp&&>(__x)); }

    public:
      _GLIBCXX_SIMD_INTRINSIC constexpr
      _SmartReference(_Up& __o, _SimdSizeType __i) noexcept
      : _M_index(__i), _M_obj(__o) {}

      using value_type = _ValueType;

      _SmartReference(const _SmartReference&) = delete;

      _GLIBCXX_SIMD_INTRINSIC constexpr
      operator value_type() const noexcept
      { return _M_read(); }

      template <__broadcast_constructible<value_type> _Tp>
        _GLIBCXX_SIMD_INTRINSIC constexpr _SmartReference
        operator=(_Tp&& __x) &&
        {
          _M_write(static_cast<_Tp&&>(__x));
          return {_M_obj, _M_index};
        }

#define _GLIBCXX_SIMD_OP_(__op)                                                                    \
      template <__broadcast_constructible<value_type> _Tp>                                         \
        requires __broadcast_constructible<decltype(declval<value_type>() __op declval<_Tp>()),    \
                                           value_type>                                             \
        _GLIBCXX_SIMD_INTRINSIC constexpr _SmartReference                                          \
        operator __op##=(_Tp&& __x) &&                                                             \
        {                                                                                          \
          const value_type& __lhs = _M_read();                                                     \
          _M_write(__lhs __op __x);                                                                \
          return {_M_obj, _M_index};                                                               \
        }

      _GLIBCXX_SIMD_ALL_ARITHMETICS(_GLIBCXX_SIMD_OP_);
      _GLIBCXX_SIMD_ALL_SHIFTS(_GLIBCXX_SIMD_OP_);
      _GLIBCXX_SIMD_ALL_BINARY(_GLIBCXX_SIMD_OP_);
#undef _GLIBCXX_SIMD_OP_

      template <typename _Tp = void,
                typename = decltype(++declval<conditional_t<true, value_type, _Tp>&>())>
        _GLIBCXX_SIMD_INTRINSIC constexpr _SmartReference
        operator++() &&
        {
          value_type __x = _M_read();
          _M_write(++__x);
          return {_M_obj, _M_index};
        }

      template <typename _Tp = void,
                typename = decltype(declval<conditional_t<true, value_type, _Tp>&>()++)>
        _GLIBCXX_SIMD_INTRINSIC constexpr value_type
        operator++(int) &&
        {
          const value_type __r = _M_read();
          value_type __x = __r;
          _M_write(++__x);
          return __r;
        }

      template <typename _Tp = void,
                typename = decltype(--declval<conditional_t<true, value_type, _Tp>&>())>
        _GLIBCXX_SIMD_INTRINSIC constexpr _SmartReference
        operator--() &&
        {
          value_type __x = _M_read();
          _M_write(--__x);
          return {_M_obj, _M_index};
        }

      template <typename _Tp = void,
                typename = decltype(declval<conditional_t<true, value_type, _Tp>&>()--)>
        _GLIBCXX_SIMD_INTRINSIC constexpr value_type
        operator--(int) &&
        {
          const value_type __r = _M_read();
          value_type __x = __r;
          _M_write(--__x);
          return __r;
        }

      _GLIBCXX_SIMD_INTRINSIC friend constexpr void
      swap(_SmartReference&& __a, _SmartReference&& __b) noexcept(
        conjunction<
          is_nothrow_constructible<value_type, _SmartReference&&>,
          is_nothrow_assignable<_SmartReference&&, value_type&&>>::value)
      {
        value_type __tmp = static_cast<_SmartReference&&>(__a);
        static_cast<_SmartReference&&>(__a) = static_cast<value_type>(__b);
        static_cast<_SmartReference&&>(__b) = std::move(__tmp);
      }

      _GLIBCXX_SIMD_INTRINSIC friend constexpr void
      swap(value_type& __a, _SmartReference&& __b) noexcept(
        conjunction<
          is_nothrow_constructible<value_type, value_type&&>,
          is_nothrow_assignable<value_type&, value_type&&>,
          is_nothrow_assignable<_SmartReference&&, value_type&&>>::value)
      {
        value_type __tmp(std::move(__a));
        __a = static_cast<value_type>(__b);
        static_cast<_SmartReference&&>(__b) = std::move(__tmp);
      }

      _GLIBCXX_SIMD_INTRINSIC friend constexpr void
      swap(_SmartReference&& __a, value_type& __b) noexcept(
        conjunction<
          is_nothrow_constructible<value_type, _SmartReference&&>,
          is_nothrow_assignable<value_type&, value_type&&>,
          is_nothrow_assignable<_SmartReference&&, value_type&&>>::value)
      {
        value_type __tmp(__a);
        static_cast<_SmartReference&&>(__a) = std::move(__b);
        __b = std::move(__tmp);
      }
    };
}

namespace std::__detail
{
  ///////////////////////////////////////////////////////////////////////////////////////////////
  /////////////// tools for working with gnu::vector_size types (vector builtins) ///////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Alias for a vector builtin with given value type and total sizeof.
   */
  template <__vectorizable _Tp, size_t _Bytes>
    requires (__has_single_bit(_Bytes))
    using __vec_builtin_type_bytes [[__gnu__::__vector_size__(_Bytes)]] = _Tp;

  /**
   * Alias for a vector builtin with given value type \p _Tp and \p _Width.
   */
  template <__vectorizable _Tp, _SimdSizeType _Width>
    requires (__has_single_bit(_Width))
    using __vec_builtin_type = __vec_builtin_type_bytes<_Tp, sizeof(_Tp) * _Width>;

  /**
   * Constrain to any vector builtin with given value type and optional width.
   */
  template <typename _Tp, typename _ValueType,
            _SimdSizeType _Width = sizeof(_Tp) / sizeof(_ValueType)>
    concept __vec_builtin_of
      = not __arithmetic<_Tp> and __vectorizable<_ValueType>
          and _Width > 1 and sizeof(_Tp) / sizeof(_ValueType) == _Width
          and same_as<__vec_builtin_type_bytes<_ValueType, sizeof(_Tp)>, _Tp>
          and requires(_Tp& __v, _ValueType __x) { __v[0] = __x; };

  static_assert(    __vec_builtin_of<__vec_builtin_type<int, 4>, int>);
  static_assert(not __vec_builtin_of<__vec_builtin_type<int, 4>, char>);
  static_assert(not __vec_builtin_of<int, int>);

  /**
   * Constrain to any vector builtin.
   */
  template <typename _Tp>
    concept __vec_builtin
      = requires(const _Tp& __x) {
        requires __vec_builtin_of<_Tp, remove_cvref_t<decltype(__x[0])>>;
      };

  static_assert(not __vec_builtin<int>);
  static_assert(    __vec_builtin<__vec_builtin_type<int, 4>>);

  template <typename _Tp>
    struct __value_type_of_impl;

  template <typename _Tp>
    concept __has_value_type_member = requires { typename _Tp::value_type; };

  /**
   * Alias for the value type of the given type \p _Tp.
   */
  template <typename _Tp>
    requires __vec_builtin<_Tp> or __arithmetic<_Tp> or __has_value_type_member<_Tp>
    using __value_type_of = typename __value_type_of_impl<_Tp>::type;

  template <__vec_builtin _Tp>
    struct __value_type_of_impl<_Tp>
    { using type = remove_cvref_t<decltype(std::declval<const _Tp>()[0])>; };

  template <__arithmetic _Tp>
    struct __value_type_of_impl<_Tp>
    { using type = _Tp; };

  template <__has_value_type_member _Tp>
    struct __value_type_of_impl<_Tp>
    { using type = typename _Tp::value_type; };

  /**
   * The width (number of value_type elements) of the given vector builtin or arithmetic type.
   */
  template <typename _Tp>
    requires __vec_builtin<_Tp> or __arithmetic<_Tp>
    inline constexpr _SimdSizeType __width_of = sizeof(_Tp) / sizeof(__value_type_of<_Tp>);

  /**
   * Alias for a vector builtin with value type \p _Up and equal width as \p _TV.
   */
  template <__vectorizable _Up, __vec_builtin _TV>
    using __rebind_vec_builtin_t = __vec_builtin_type<_Up, __width_of<_TV>>;

  /**
   * Alias for a vector builtin with value type of \p _TV transformed using \p _Trait and equal
   * width as \p _TV.
   */
  template <template <typename> class _Trait, __vec_builtin _TV>
    using __transform_vec_builtin_t
      = __vec_builtin_type<_Trait<__value_type_of<_TV>>, __width_of<_TV>>;

  /**
   * Alias for a vector mask type matching the given \p _TV type.
   */
  template <__vec_builtin _TV>
    using __mask_vec_from = __transform_vec_builtin_t<__make_signed_int_t, _TV>;

  /**
   * Constrain to vector builtins with given value_type sizeof and optionally vector type sizeof.
   */
  template <typename _Tp, size_t _ValueTypeSize, size_t _VecSize = sizeof(_Tp)>
    concept __vec_builtin_sizeof
      = __vec_builtin<_Tp> and sizeof(_Tp) == _VecSize
          and sizeof(__value_type_of<_Tp>) == _ValueTypeSize;

  static_assert(    __vec_builtin_sizeof<__vec_builtin_type<int, 4>, sizeof(int)>);
  static_assert(not __vec_builtin_sizeof<int, sizeof(int)>);

  using __v2double [[__gnu__::__vector_size__(16)]] = double;
  using __v4double [[__gnu__::__vector_size__(32)]] = double;
  using __v8double [[__gnu__::__vector_size__(64)]] = double;

  using __v4float [[__gnu__::__vector_size__(16)]] = float;
  using __v8float [[__gnu__::__vector_size__(32)]] = float;
  using __v16float [[__gnu__::__vector_size__(64)]] = float;

  using __v16char [[__gnu__::__vector_size__(16)]] = char;
  using __v32char [[__gnu__::__vector_size__(32)]] = char;
  using __v64char [[__gnu__::__vector_size__(64)]] = char;

  using __v16schar [[__gnu__::__vector_size__(16)]] = signed char;
  using __v32schar [[__gnu__::__vector_size__(32)]] = signed char;
  using __v64schar [[__gnu__::__vector_size__(64)]] = signed char;

  using __v16uchar [[__gnu__::__vector_size__(16)]] = unsigned char;
  using __v32uchar [[__gnu__::__vector_size__(32)]] = unsigned char;
  using __v64uchar [[__gnu__::__vector_size__(64)]] = unsigned char;

  using __v8int16 [[__gnu__::__vector_size__(16)]] = int16_t;
  using __v16int16 [[__gnu__::__vector_size__(32)]] = int16_t;
  using __v32int16 [[__gnu__::__vector_size__(64)]] = int16_t;

  using __v8uint16 [[__gnu__::__vector_size__(16)]] = uint16_t;
  using __v16uint16 [[__gnu__::__vector_size__(32)]] = uint16_t;
  using __v32uint16 [[__gnu__::__vector_size__(64)]] = uint16_t;

  using __v4int32 [[__gnu__::__vector_size__(16)]] = int32_t;
  using __v8int32 [[__gnu__::__vector_size__(32)]] = int32_t;
  using __v16int32 [[__gnu__::__vector_size__(64)]] = int32_t;

  using __v4uint32 [[__gnu__::__vector_size__(16)]] = uint32_t;
  using __v8uint32 [[__gnu__::__vector_size__(32)]] = uint32_t;
  using __v16uint32 [[__gnu__::__vector_size__(64)]] = uint32_t;

  using __v2uint64 [[__gnu__::__vector_size__(16)]] = uint64_t;
  using __v4uint64 [[__gnu__::__vector_size__(32)]] = uint64_t;
  using __v8uint64 [[__gnu__::__vector_size__(64)]] = uint64_t;

  using __v2int64 [[__gnu__::__vector_size__(16)]] = int64_t;
  using __v4int64 [[__gnu__::__vector_size__(32)]] = int64_t;
  using __v8int64 [[__gnu__::__vector_size__(64)]] = int64_t;

  using __v2llong [[__gnu__::__vector_size__(16)]] = long long;
  using __v4llong [[__gnu__::__vector_size__(32)]] = long long;
  using __v8llong [[__gnu__::__vector_size__(64)]] = long long;

  using __v2ullong [[__gnu__::__vector_size__(16)]] = unsigned long long;
  using __v4ullong [[__gnu__::__vector_size__(32)]] = unsigned long long;
  using __v8ullong [[__gnu__::__vector_size__(64)]] = unsigned long long;

  /**
   * An object of given type where all bits are 1.
   */
  template <__vec_builtin _V>
    static inline constexpr _V _S_allbits
      = reinterpret_cast<_V>(~__vec_builtin_type_bytes<char, sizeof(_V)>());

  /**
   * An object of given type where only the sign bits are 1.
   */
  template <__vec_builtin _V>
    requires floating_point<__value_type_of<_V>>
    static inline constexpr _V _S_signmask = __xor(_V() + 1, _V() - 1);

  /**
   * An object of given type where only the sign bits are 0 (complement of _S_signmask).
   */
  template <__vec_builtin _V>
    requires floating_point<__value_type_of<_V>>
    static inline constexpr _V _S_absmask = __andnot(_S_signmask<_V>, _S_allbits<_V>);

  /**
   * Returns a permutation of the given vector builtin. _Indices work like for
   * __builtin_shufflevector, except that -1 signifies a 0.
   */
  template <int... _Indices, __vec_builtin _Tp>
    _GLIBCXX_SIMD_INTRINSIC constexpr _Tp
    __vec_permute(_Tp __x)
    {
      static_assert(sizeof...(_Indices) == __width_of<_Tp>);
      return __builtin_shufflevector(__x, _Tp(),
                                     (_Indices == -1 ? __width_of<_Tp> : _Indices)...);
    }

  /**
   * Split \p __x into \p _Total parts and return the part at index \p _Index. Optionally combine
   * multiple parts into the return value (\p _Combine).
   */
  template <int _Index, int _Total, int _Combine = 1, __vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC _GLIBCXX_CONST constexpr
    __vec_builtin_type<__value_type_of<_TV>, __width_of<_TV> / _Total * _Combine>
    __vec_extract_part(_TV __x)
    {
      using _Tp = __value_type_of<_TV>;
      constexpr int __values_per_part = __width_of<_TV> / _Total;
      constexpr int __values_to_skip = _Index * __values_per_part;
      constexpr int __return_size = _Combine * __values_per_part;
      static_assert((_Index + _Combine) * __values_per_part * sizeof(_Tp) <= sizeof(__x),
                    "out of bounds __vec_extract_part");
      return _GLIBCXX_SIMD_INT_PACK(__return_size, _Ind, {
               return __builtin_shufflevector(__x, __x, (__values_to_skip + _Ind)...);
             });
    }

  template <int _Index, int _Total, int _Combine = 1, integral _Tp,
            vir::constexpr_value<int> _Width = decltype(vir::cw<sizeof(_Tp) * __CHAR_BIT__>)>
    _GLIBCXX_SIMD_INTRINSIC _GLIBCXX_CONST constexpr integral auto
    __vec_extract_part(_Tp __x, _Width __width = {})
    {
      constexpr int __values_per_part = __width / _Total;
      constexpr int __values_to_skip = _Index * __values_per_part;
      constexpr int __return_size = __values_per_part * _Combine;
      static_assert((_Index + _Combine) * __values_per_part * sizeof(_Tp) <= sizeof(__x),
                    "out of bounds __vec_extract_part");
      return _GLIBCXX_SIMD_INT_PACK(__return_size, _Ind, {
               return __builtin_shufflevector(__x, __x, (__values_to_skip + _Ind)...);
             });
    }

  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr __vec_builtin_type_bytes<__value_type_of<_TV>, 16>
    __vec_lo128(_TV __x)
    {
      using _Tp = __value_type_of<_TV>;
      constexpr int __new_width = 16 / sizeof(_Tp);
      static_assert(sizeof(_TV) >= 32);
      return _GLIBCXX_SIMD_INT_PACK(__new_width, _Is, {
               return __builtin_shufflevector(__x, __x, _Is...);
             });
    }

  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr __vec_builtin_type_bytes<__value_type_of<_TV>, 16>
    __vec_hi128(_TV __x)
    {
      using _Tp = __value_type_of<_TV>;
      constexpr int __new_width = 16 / sizeof(_Tp);
      static_assert(sizeof(_TV) >= 32);
      return _GLIBCXX_SIMD_INT_PACK(__new_width, _Is, {
               return __builtin_shufflevector(__x, __x, (__width_of<_TV> - __new_width + _Is)...);
             });
    }

  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr __vec_builtin_type_bytes<__value_type_of<_TV>, 32>
    __vec_lo256(_TV __x)
    {
      using _Tp = __value_type_of<_TV>;
      constexpr int __new_width = 32 / sizeof(_Tp);
      static_assert(sizeof(_TV) >= 64);
      return _GLIBCXX_SIMD_INT_PACK(__new_width, _Is, {
               return __builtin_shufflevector(__x, __x, _Is...);
             });
    }

  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr __vec_builtin_type_bytes<__value_type_of<_TV>, 32>
    __vec_hi256(_TV __x)
    {
      using _Tp = __value_type_of<_TV>;
      constexpr int __new_width = 32 / sizeof(_Tp);
      static_assert(sizeof(_TV) >= 64);
      return _GLIBCXX_SIMD_INT_PACK(__new_width, _Is, {
               return __builtin_shufflevector(__x, __x, (__width_of<_TV> - __new_width + _Is)...);
             });
    }

  /**
   * Return vector builtin with all values from \p __a and \p __b.
   */
  template <__vec_builtin _Tp>
    _GLIBCXX_SIMD_INTRINSIC auto
    __vec_concat(_Tp __a, _Tp __b)
    {
      return _GLIBCXX_SIMD_INT_PACK(__width_of<_Tp> * 2, _Is, {
               return __builtin_shufflevector(__a, __b, _Is...);
             });
    }

  template <int _Offset, __vec_builtin _Tp>
    _GLIBCXX_SIMD_INTRINSIC auto
    __vec_concat_from_pack(_Tp __a)
    {
      static_assert(_Offset == 0);
      return __vec_concat(__a, _Tp{});
    }

  template <int _Offset, __vec_builtin _Tp, __vec_builtin... _More>
    _GLIBCXX_SIMD_INTRINSIC auto
    __vec_concat_from_pack(_Tp __a, _Tp __b, _More... __more)
    {
      if constexpr (_Offset == 0)
        return __vec_concat(__a, __b);
      else
        return __vec_concat_from_pack<_Offset - 1>(__more...);
    }

  template <__vec_builtin _Tp, __vec_builtin... _More>
    requires (sizeof...(_More) >= 1)
    _GLIBCXX_SIMD_INTRINSIC auto
    __vec_concat(_Tp __a, _Tp __b, _More... __more)
    {
      static_assert((std::is_same_v<_Tp, _More> and ...));
      return _GLIBCXX_SIMD_INT_PACK((sizeof...(_More) + 1) / 2, _Is, {
               return __vec_concat(__vec_concat(__a, __b),
                                   __vec_concat_from_pack<_Is>(__more...)...);
             });
    }

  /**
   * Convert \p __a to _To.
   * Prefer this function over calling __builtin_convertvector directly so that the library can
   * improve code-gen (until the relevant PRs on GCC get resolved).
   */
  template <__vec_builtin _To, __vec_builtin _From>
    _GLIBCXX_SIMD_INTRINSIC _To
    __vec_convert(_From __a)
    { return __builtin_convertvector(__a, _To); }

  template <__vectorizable _To, __vec_builtin _From>
    _GLIBCXX_SIMD_INTRINSIC __rebind_vec_builtin_t<_To, _From>
    __vec_convert(_From __a)
    { return __builtin_convertvector(__a, __rebind_vec_builtin_t<_To, _From>); }

  template <__vec_builtin _To, __vec_builtin... _From>
    requires (sizeof...(_From) >= 2)
    _GLIBCXX_SIMD_INTRINSIC _To
    __vec_convert(_From... __pack)
    {
      using _T2 = __vec_builtin_type_bytes<__value_type_of<_To>,
                                           sizeof(_To) / std::__bit_ceil(sizeof...(__pack))>;
      return __vec_concat(__vec_convert<_T2>(__pack)...);
    }

  /**
   * Converts __v into array<_To, N>, where N is _NParts if non-zero or otherwise deduced from _To
   * such that N * #elements(_To) <= #elements(__v). Note: this function may return less than all
   * converted elements
   * \tparam _NParts allows to convert fewer or more (only last _To, to be partially filled) than
   *                 all
   * \tparam _Offset where to start, number of elements (not Bytes or Parts)
   */
   template <typename _To, int _NParts = 0, int _Offset = 0, int _FromSize = 0, __vec_builtin _From>
     _GLIBCXX_SIMD_INTRINSIC auto
     __vec_convert_all(_From __v)
     {
       static_assert(_FromSize < __width_of<_From>);
       constexpr int __input_size = _FromSize == 0 ? __width_of<_From> : _FromSize;
       if constexpr (is_arithmetic_v<_To> && _NParts != 1)
         {
           static_assert(_Offset < __width_of<_From>);
           constexpr int _Np = _NParts == 0 ? __input_size - _Offset : _NParts;
           using _Rp = array<_To, _Np>;
           return _GLIBCXX_SIMD_INT_PACK(_Np, _Is, {
                    return _Rp{(static_cast<_To>(__v[_Is + _Offset]))...};
                  });
         }
       else
         {
           static_assert(__vec_builtin<_To>);
           if constexpr (_NParts == 1)
             {
               static_assert(_Offset % __width_of<_To> == 0);
               return array<_To, 1>{
                 __vec_convert<_To>(__vec_extract_part<
                                      _Offset / __width_of<_To>,
                                      __div_roundup(__input_size, __width_of<_To>)>(__v))
               };
             }
           else if constexpr ((__input_size - _Offset) > __width_of<_To>)
             {
               constexpr size_t _NTotal = (__input_size - _Offset) / __width_of<_To>;
               constexpr size_t _Np = _NParts == 0 ? _NTotal : _NParts;
               static_assert(_Np <= _NTotal
                               or (_Np == _NTotal + 1
                                     and (__input_size - _Offset) % __width_of<_To> > 0));
               using _Rp = array<_To, _Np>;
               if constexpr (_Np == 1)
                 return _Rp{__vec_convert<_To>(__vec_extract_part<_Offset, __input_size,
                                                                  __width_of<_To>>(__v))};
               else
                 return _GLIBCXX_SIMD_INT_PACK(_Np, _Is, {
                          return _Rp {
                            __vec_convert<_To>(
                              __vec_extract_part<_Is * __width_of<_To> + _Offset, __input_size,
                                                 __width_of<_To>>(__v))...
                          };
                        });
             }
           else if constexpr (_Offset == 0)
             return array<_To, 1>{__vec_convert<_To>(__v)};
           else
             return array<_To, 1>{__vec_convert<_To>(
                                    __vec_extract_part<_Offset, __input_size,
                                                       __input_size - _Offset>(__v))};
         }
     }

  /**
   * Generator "ctor" for __vec_builtin types.
   */
#define _GLIBCXX_SIMD_VEC_GEN(_Tp, width, pack, code)                                              \
  _GLIBCXX_SIMD_INT_PACK(width, pack, { return _Tp code; })

  template <__vec_builtin _Tp, int _Width = __width_of<_Tp>>
    _GLIBCXX_SIMD_INTRINSIC constexpr _Tp
    __vec_generate(auto&& __gen)
    { return _GLIBCXX_SIMD_VEC_GEN(_Tp, _Width, _Is, {__gen(vir::cw<_Is>)...}); }

  template <int _Width, typename _Tp>
    _GLIBCXX_SIMD_INTRINSIC constexpr __vec_builtin_type<_Tp, __bit_ceil(_Width)>
    __vec_broadcast(_Tp __x)
    {
      using _Rp = __vec_builtin_type<_Tp, __bit_ceil(_Width)>;
      return _GLIBCXX_SIMD_VEC_GEN(_Rp, _Width, __is, {(__is < _Width ? __x : _Tp())...});
    }

  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr _TV
    __vec_xor(_TV __a, _TV __b)
    {
      using _Tp = __value_type_of<_TV>;
      if constexpr (is_floating_point_v<_Tp>)
        {
          using _UV = __vec_builtin_type<__make_unsigned_int_t<_Tp>, __width_of<_TV>>;
          return reinterpret_cast<_TV>(reinterpret_cast<_UV>(__a) ^ reinterpret_cast<_UV>(__b));
        }
      else
        return __a ^ __b;
    }

  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr _TV
    __vec_or(_TV __a, _TV __b)
    {
      using _Tp = __value_type_of<_TV>;
      if constexpr (is_floating_point_v<_Tp>)
        {
          using _UV = __vec_builtin_type<__make_unsigned_int_t<_Tp>, __width_of<_TV>>;
          return reinterpret_cast<_TV>(reinterpret_cast<_UV>(__a) | reinterpret_cast<_UV>(__b));
        }
      else
        return __a | __b;
    }

  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr _TV
    __vec_and(_TV __a, _TV __b)
    {
      using _Tp = __value_type_of<_TV>;
      if constexpr (is_floating_point_v<_Tp>)
        {
          using _UV = __vec_builtin_type<__make_unsigned_int_t<_Tp>, __width_of<_TV>>;
          return reinterpret_cast<_TV>(reinterpret_cast<_UV>(__a) & reinterpret_cast<_UV>(__b));
        }
      else
        return __a & __b;
    }


  //overloaded in x86_detail.h
  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr _TV
    __vec_andnot(_TV __a, _TV __b)
    {
      using _Tp = __value_type_of<_TV>;
      using _UV = __vec_builtin_type<__make_unsigned_int_t<_Tp>, __width_of<_TV>>;
      return reinterpret_cast<_TV>(~reinterpret_cast<_UV>(__a) & reinterpret_cast<_UV>(__b));
    }

  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr _TV
    __vec_not(_TV __a)
    {
      using _UV = __vec_builtin_type<unsigned, sizeof(_TV)>;
      if constexpr (is_floating_point_v<__value_type_of<_TV>>)
        return reinterpret_cast<_TV>(~reinterpret_cast<_UV>(__a));
      else
        return ~__a;
    }

  /**
   * Bit-cast \p __x to a vector type with equal sizeof but value-type \p _Up.
   * Optionally, the width of the return type can be constrained, making the cast ill-formed if it
   * doesn't match.
   */
  template <typename _Up, int _Np = 0, __vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr auto
    __vec_bitcast(_TV __x)
    {
      if constexpr (_Np == 0)
        return reinterpret_cast<__vec_builtin_type_bytes<_Up, sizeof(__x)>>(__x);
      else
        return reinterpret_cast<__vec_builtin_type<_Up, _Np>>(__x);
    }

  /**
   * Bit-cast \p __x to the vector type \p _UV. sizeof(_UV) may be smaller than sizeof(__x).
   */
  template <__vec_builtin _UV, __vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr _UV
    __vec_bitcast_trunc(_TV __x)
    {
      static_assert(sizeof(_UV) <= sizeof(_TV));
      if constexpr (sizeof(_UV) == sizeof(_TV))
        return reinterpret_cast<_UV>(__x);
      else if constexpr (sizeof(_UV) <= 8)
        {
          using _Ip = __make_signed_int_t<_UV>;
          return reinterpret_cast<_UV>(
                   reinterpret_cast<__vec_builtin_type_bytes<_Ip, sizeof(__x)>>(__x)[0]);
        }
      else
        {
          const auto __y
            = reinterpret_cast<__vec_builtin_type_bytes<__value_type_of<_UV>, sizeof(__x)>>(__x);
          return _GLIBCXX_SIMD_INT_PACK(__width_of<_UV>, _Is, {
                   return __builtin_shufflevector(__y, __y, _Is...);
                 });
        }
    }

  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC _TV
    __vec_optimizer_barrier(_TV __x)
    {
      asm("":"+v,x,g"(__x));
      return __x;
    }

  /**
   * Return a type with sizeof 16. If the input type is smaller, add zero-padding to \p __x.
   */
  template <__vec_builtin _TV>
    _GLIBCXX_SIMD_INTRINSIC constexpr auto
    __vec_zero_pad_to_16(_TV __x)
    {
      static_assert(sizeof(_TV) <= 16);
      if constexpr (sizeof(_TV) == 16)
        return __x;
      else
        {
          using _Up = __make_signed_int_t<_TV>;
          __vec_builtin_type_bytes<_Up, 16> __tmp = {__builtin_bit_cast(_Up, __x)};
          return reinterpret_cast<__vec_builtin_type_bytes<__value_type_of<_TV>, 16>>(__tmp);
        }
    }
}

namespace std
{
  template <__detail::__vectorizable _Tp, __detail::__simd_abi_tag _Abi>
    requires requires { _Abi::_S_size; }
    struct simd_size<_Tp, _Abi>
    : integral_constant<__detail::_SimdSizeType, _Abi::_S_size>
    {};
}

#endif  // PROTOTYPE_DETAIL_H_
