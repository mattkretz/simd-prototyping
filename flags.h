/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_FLAGS_H_
#define PROTOTYPE_FLAGS_H_

#include "simd_config.h"

#include <bit>
#include <concepts>

namespace std
{
  struct element_aligned_tag
  {
    template <typename _Tp, typename _Up = typename _Tp::value_type>
      static constexpr size_t _S_alignment = alignof(_Up);

    template <typename _Tp, typename _Up>
      _GLIBCXX_SIMD_INTRINSIC static constexpr _Up*
      _S_apply(_Up* __ptr)
      { return __ptr; }
  };

  struct vector_aligned_tag
  {
    template <typename _Tp, typename _Up = typename _Tp::value_type>
      static constexpr size_t _S_alignment
      = std::__bit_ceil(sizeof(_Up) * _Tp::size());

    template <typename _Tp, typename _Up>
      _GLIBCXX_SIMD_INTRINSIC static constexpr _Up*
      _S_apply(_Up* __ptr)
      { return static_cast<_Up*>(__builtin_assume_aligned(__ptr, _S_alignment<_Tp, _Up>)); }
  };

  template <size_t _Np>
    struct overaligned_tag
    {
      template <typename _Tp, typename _Up = typename _Tp::value_type>
        static constexpr size_t _S_alignment = _Np;

      template <typename _Tp, typename _Up>
        _GLIBCXX_SIMD_INTRINSIC static constexpr _Up*
        _S_apply(_Up* __ptr)
        { return static_cast<_Up*>(__builtin_assume_aligned(__ptr, _Np)); }
    };

  inline constexpr element_aligned_tag element_aligned = {};

  inline constexpr vector_aligned_tag vector_aligned = {};

  template <size_t _Np>
    inline constexpr overaligned_tag<_Np> overaligned = {};

  namespace __detail
  {
    struct _LoadStoreTag
    {
      _GLIBCXX_SIMD_ALWAYS_INLINE
      static constexpr auto
      __adjust_pointer(auto __ptr, std::size_t)
      { return __ptr; }
    };

    struct _Convert : _LoadStoreTag
    {
    };

    struct _Aligned : _LoadStoreTag
    {
      _GLIBCXX_SIMD_ALWAYS_INLINE
      static constexpr auto
      __adjust_pointer(auto __ptr, std::size_t __simd_alignment)
      { return __builtin_assume_aligned(__ptr, __simd_alignment); }
    };

    template <std::size_t _Np> struct _Overaligned : _LoadStoreTag
    {
      _GLIBCXX_SIMD_ALWAYS_INLINE
      static constexpr auto
      __adjust_pointer(auto __ptr, std::size_t)
      { return __builtin_assume_aligned(__ptr, _Np); }
    };

    struct _Streaming : _LoadStoreTag
    {
    };

    template <int _L1, int _L2 /*, exclusive vs. shared*/>
      struct _Prefetch : _LoadStoreTag
      {
        _GLIBCXX_SIMD_ALWAYS_INLINE
        static constexpr auto
        __adjust_pointer(auto __ptr, std::size_t)
        {
          // one read: 0, 0
          // L1: 0, 1
          // L2: 0, 2
          // L3: 0, 3
          // (exclusive cache line) for writing: 1, 0 / 1, 1
/*          constexpr int __write = 1;
          constexpr int __level = 0-3;
          __builtin_prefetch(__ptr, __write, __level)
          _mm_prefetch(reinterpret_cast<char const*>(__ptr), _MM_HINT_T0);
          _mm_prefetch(reinterpret_cast<char const*>(__ptr), _MM_HINT_T1);
          _mm_prefetch(reinterpret_cast<char const*>(__ptr), _MM_HINT_T2);
          _mm_prefetch(reinterpret_cast<char const*>(__ptr), _MM_HINT_ET0);
          _mm_prefetch(reinterpret_cast<char const*>(__ptr), _MM_HINT_ET1);
          _mm_prefetch(reinterpret_cast<char const*>(__ptr), _MM_HINT_NTA);*/
          return __ptr;
        }
      };

    template <typename _Tp>
      struct _IsLoadstoreTag
      : std::false_type {};

    template <>
      struct _IsLoadstoreTag<_Convert>
      : std::true_type {};

    template <>
      struct _IsLoadstoreTag<_Aligned>
      : std::true_type {};

    template <>
      struct _IsLoadstoreTag<_Streaming>
      : std::true_type {};

    template <std::size_t _Np>
      requires(std::has_single_bit(_Np))
      struct _IsLoadstoreTag<_Overaligned<_Np>>
      : std::true_type {};

    template <int _L1, int _L2>
      struct _IsLoadstoreTag<_Prefetch<_L1, _L2>>
      : std::true_type {};

  } // namespace __detail

  template <typename... _Flags>
    requires(__detail::_IsLoadstoreTag<_Flags>::value and ...)
    struct simd_flags
    {
      friend consteval bool
      operator==(simd_flags, simd_flags)
      { return true; }

      template <typename... _Other>
        friend consteval bool
        operator==(simd_flags __x, simd_flags<_Other...> __y)
        { return std::same_as<simd_flags<>, decltype(__x ^ __y)>; }

      template <typename... _Other>
        consteval bool
        test(simd_flags<_Other...> __x) const noexcept
        { return __x == (*this & __x); }

      friend consteval auto
      operator|(simd_flags, simd_flags<>)
      { return simd_flags{}; }

      template <typename _T0, typename... _More>
        friend consteval auto
        operator|(simd_flags, simd_flags<_T0, _More...>)
        {
          if constexpr ((std::same_as<_Flags, _T0> or ...))
            return simd_flags<_Flags...>{} | simd_flags<_More...>{};
          else
            return simd_flags<_Flags..., _T0>{} | simd_flags<_More...>{};
        }

      friend consteval auto
      operator&(simd_flags, simd_flags<>)
      { return simd_flags<>{}; }

      template <typename _T0, typename... _More>
        friend consteval auto
        operator&(simd_flags, simd_flags<_T0, _More...>)
        {
          if constexpr ((std::same_as<_Flags, _T0> or ...))
            return simd_flags<_T0>{} | (simd_flags{} & simd_flags<_More...>{});
          else
            return simd_flags{} & simd_flags<_More...>{};
        }

      friend consteval auto
      operator^(simd_flags, simd_flags<>)
      { return simd_flags{}; }

      template <typename _T0, typename... _More>
        friend consteval auto
        operator^(simd_flags, simd_flags<_T0, _More...>)
        {
          if constexpr ((std::same_as<_Flags, _T0> or ...))
            {
              constexpr auto __removed
                = (std::conditional_t<std::same_as<_Flags, _T0>, simd_flags<>,
                                      simd_flags<_Flags>>{} | ...);
              return __removed ^ simd_flags<_More...>{};
            }
          else
            return (simd_flags{} | simd_flags<_T0>{}) ^ simd_flags<_More...>{};
        }

      static constexpr void
      __adjust_pointer(auto __ptr, std::size_t __simd_alignment)
      { ((__ptr = _Flags::__adjust_pointer(__ptr, __simd_alignment)), ...); }
    };

  template <typename _Tp>
    struct is_loadstore_flag
    : std::false_type {};

  template <typename... _Flags>
    struct is_loadstore_flag<simd_flags<_Flags...>>
    : std::bool_constant<(__detail::_IsLoadstoreTag<_Flags>::value and ...)> {};

  template <typename _Tp>
    inline constexpr bool is_loadstore_flag_v = is_loadstore_flag<_Tp>::value;

  // loadstore flags
  inline constexpr simd_flags<> simd_flag_default;

  inline constexpr simd_flags<__detail::_Convert> simd_flag_convert;

  inline constexpr simd_flags<__detail::_Aligned> simd_flag_aligned;

  template <std::size_t _Np>
    requires(std::has_single_bit(_Np))
    inline constexpr simd_flags<__detail::_Overaligned<_Np>> simd_flag_overaligned;

  inline constexpr simd_flags<__detail::_Streaming> simd_flag_streaming;

  template <int _L1, int _L2>
    inline constexpr simd_flags<__detail::_Prefetch<_L1, _L2>> simd_flag_prefetch;

} // namespace std

#endif  // PROTOTYPE_FLAGS_H_
