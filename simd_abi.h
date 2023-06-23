/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_ABI_H_
#define PROTOTYPE_SIMD_ABI_H_

#include <experimental/simd>

namespace std
{
  namespace __detail
  {
    template <typename _Tp>
      using _NativeAbi = std::experimental::parallelism_v2::simd_abi::native<_Tp>;
  }

  template <typename _Tp, typename _Abi = __detail::_NativeAbi<_Tp>>
    class basic_simd;

  template <size_t _Bytes, typename _Abi>
    class basic_simd_mask;

  namespace __detail
  {
    using namespace std::experimental::parallelism_v2;
    using namespace std::experimental::parallelism_v2::__proposed;

    using _SimdSizeType = int;

    template <_SimdSizeType _Np, typename = experimental::__detail::__odr_helper>
      struct _MaskImplAbiCombine;

    template <_SimdSizeType _Np>
      struct _AbiCombine
      {
        template <typename _Tp>
          static constexpr _SimdSizeType _S_size = _Np;

        template <typename _Tp>
          static constexpr _SimdSizeType _S_full_size = _Np;

        struct _IsValidAbiTag
        : public __bool_constant<(_Np > 1)>
        {};

        template <typename _Tp>
          static constexpr _SimdSizeType _S_max_size
            = std::max(_SimdSizeType(64),
                       _SimdSizeType(4 * experimental::simd_size_v<_Tp, _NativeAbi<_Tp>>));

        template <typename _Tp>
          struct _IsValidSizeFor
          : __bool_constant<(_Np <= _S_max_size<_Tp>)>
          {};

        template <typename _Tp>
          struct _IsValid
          : conjunction<_IsValidAbiTag, __is_vectorizable<_Tp>, _IsValidSizeFor<_Tp>>
          {};

        template <typename _Tp>
          static constexpr bool _S_is_valid_v = _IsValid<_Tp>::value;

        _GLIBCXX_SIMD_INTRINSIC static constexpr _SanitizedBitMask<_Np>
        _S_masked(_BitMask<_Np> __x)
        { return __x._M_sanitized(); }

        _GLIBCXX_SIMD_INTRINSIC static constexpr _SanitizedBitMask<_Np>
        _S_masked(_SanitizedBitMask<_Np> __x)
        { return __x; }

        using _CommonImpl = _CommonImplFixedSize;

        using _SimdImpl = _SimdImplFixedSize<_Np>;

        using _MaskImpl = _MaskImplAbiCombine<_Np>;

        template <typename _Tp>
          struct __traits
          : _InvalidTraits
          {};

        template <typename _Tp>
          requires _S_is_valid_v<_Tp>
          struct __traits<_Tp>
          {
            using _IsValid = true_type;

            using _SimdImpl = _SimdImplFixedSize<_Np>;

            using _MaskImpl = _MaskImplAbiCombine<_Np>;

            using _SimdMember = __fixed_size_storage_t<_Tp, _Np>;

            using _MaskMember = _SanitizedBitMask<_Np>;

            static constexpr size_t _S_simd_align = std::__bit_ceil(_Np * sizeof(_Tp));

            static constexpr size_t _S_mask_align = alignof(_MaskMember);

            // base class for simd, providing extra conversions
            struct _SimdBase
            {
              _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
              operator const _SimdMember &() const
              { return static_cast<const std::basic_simd<_Tp, _AbiCombine>*>(this)->_M_data(); }

              _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
              operator array<_Tp, _Np>() const
              {
                array<_Tp, _Np> __r;
                static_cast<const std::basic_simd<_Tp, _AbiCombine>*>(this)->copy_to(__r.begin());
                return __r;
              }
            };

            // empty. The bitset interface suffices
            struct _MaskBase {};

            struct _SimdCastType
            {
              _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
              _SimdCastType(const array<_Tp, _Np>& __a)
              { __builtin_memcpy(&_M_data, __a.data(), _Np * sizeof(_Tp)); }

              _GLIBCXX_SIMD_ALWAYS_INLINE constexpr
              _SimdCastType(const _SimdMember& __dd)
              : _M_data(__dd)
              {}

              _GLIBCXX_SIMD_ALWAYS_INLINE constexpr explicit
              operator const _SimdMember &() const { return _M_data; }

            private:
              _SimdMember _M_data;
            };

            class _MaskCastType
            {
              _MaskCastType() = delete;
            };
          };
      };

    template <_SimdSizeType _Np, typename>
      struct _MaskImplAbiCombine
      {
        static_assert(
          sizeof(_ULLong) * __CHAR_BIT__ >= _Np,
          "The fixed_size implementation relies on one _ULLong being able to store "
          "all boolean elements."); // required in load & store

        using _Abi = _AbiCombine<_Np>;

        using _MaskMember = _SanitizedBitMask<_Np>;

        template <typename _Tp>
          using _FirstAbi = typename __fixed_size_storage_t<_Tp, _Np>::_FirstAbi;

        template <typename _Tp>
          using _TypeTag = _Tp*;

        template <typename>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
          _S_broadcast(bool __x)
          { return __x ? ~_MaskMember() : _MaskMember(); }

        template <typename>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
          _S_load(const bool* __mem)
          {
            using _Ip = __int_for_sizeof_t<bool>;
            // the following load uses element_aligned and relies on __mem already
            // carrying alignment information from when this load function was
            // called.
            const std::basic_simd<_Ip, _Abi> __bools(
                    reinterpret_cast<const __may_alias<_Ip>*>(__mem));
            return __data(__bools != 0);
          }

        template <bool _Sanitized>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SanitizedBitMask<_Np>
          _S_to_bits(_BitMask<_Np, _Sanitized> __x)
          { return __x; }

        template <typename _Tp, size_t _Bs, typename _UAbi>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
          _S_convert(std::basic_simd_mask<_Bs, _UAbi> __x)
          { return _UAbi::_MaskImpl::_S_to_bits(__data(__x)).template _M_extract<0, _Np>(); }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
          _S_from_bitmask(_MaskMember __bits, _TypeTag<_Tp>) noexcept
          { return __bits; }

        static constexpr _MaskMember
        _S_load(const bool* __mem) noexcept
        {
          // TODO: _UChar is not necessarily the best type to use here. For smaller
          // _Np _UShort, _UInt, _ULLong, float, and double can be more efficient.
          _ULLong __r = 0;
          using _Vs = __fixed_size_storage_t<_UChar, _Np>;
          __for_each(_Vs{}, [&](auto __meta, auto) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                       __r |= __meta._S_mask_to_shifted_ullong(
                                __meta._S_mask_impl._S_load(&__mem[__meta._S_offset],
                                                            _SizeConstant<__meta._S_size()>()));
                     });
          return __r;
        }

        static constexpr _MaskMember
        _S_masked_load(_MaskMember __merge, _MaskMember __mask, const bool* __mem) noexcept
        {
          _BitOps::_S_bit_iteration(__mask.to_ullong(),
                                    [&](auto __i) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                                      __merge.set(__i, __mem[__i]);
                                    });
          return __merge;
        }

        static constexpr void
        _S_store(const _MaskMember __bitmask, bool* __mem) noexcept
        { _FirstAbi<_UChar>::_CommonImpl::_S_store_bool_array(__bitmask, __mem); }

        static constexpr void
        _S_masked_store(const _MaskMember __v, bool* __mem, const _MaskMember __k) noexcept
        {
          _BitOps::_S_bit_iteration(
            __k, [&](auto __i) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA { __mem[__i] = __v[__i]; });
        }

        _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
        _S_logical_and(const _MaskMember& __x, const _MaskMember& __y) noexcept
        { return __x & __y; }

        _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
        _S_logical_or(const _MaskMember& __x, const _MaskMember& __y) noexcept
        { return __x | __y; }

        _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
        _S_bit_not(const _MaskMember& __x) noexcept
        { return ~__x; }

        _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
        _S_bit_and(const _MaskMember& __x, const _MaskMember& __y) noexcept
        { return __x & __y; }

        _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
        _S_bit_or(const _MaskMember& __x, const _MaskMember& __y) noexcept
        { return __x | __y; }

        _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
        _S_bit_xor(const _MaskMember& __x, const _MaskMember& __y) noexcept
        { return __x ^ __y; }

        _GLIBCXX_SIMD_INTRINSIC static constexpr void
        _S_set(_MaskMember& __k, _SimdSizeType __i, bool __x) noexcept
        { __k.set(__i, __x); }

        _GLIBCXX_SIMD_INTRINSIC static constexpr void
        _S_masked_assign(const _MaskMember __k, _MaskMember& __lhs, const _MaskMember __rhs)
        { __lhs = (__lhs & ~__k) | (__rhs & __k); }

        // Optimization for the case where the RHS is a scalar.
        _GLIBCXX_SIMD_INTRINSIC static constexpr void
        _S_masked_assign(const _MaskMember __k, _MaskMember& __lhs, const bool __rhs)
        {
          if (__rhs)
            __lhs |= __k;
          else
            __lhs &= ~__k;
        }

        template <size_t _Bs>
          _GLIBCXX_SIMD_INTRINSIC static constexpr bool
          _S_all_of(basic_simd_mask<_Bs, _Abi> __k)
          { return __data(__k).all(); }

        template <size_t _Bs>
          _GLIBCXX_SIMD_INTRINSIC static constexpr bool
          _S_any_of(basic_simd_mask<_Bs, _Abi> __k)
          { return __data(__k).any(); }

        template <size_t _Bs>
          _GLIBCXX_SIMD_INTRINSIC static constexpr bool
          _S_none_of(basic_simd_mask<_Bs, _Abi> __k)
          { return __data(__k).none(); }

        template <size_t _Bs>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdSizeType
          _S_popcount(basic_simd_mask<_Bs, _Abi> __k)
          { return __data(__k).count(); }

        template <size_t _Bs>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdSizeType
          _S_find_first_set(basic_simd_mask<_Bs, _Abi> __k)
          { return std::__countr_zero(__data(__k).to_ullong()); }

        template <size_t _Bs>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdSizeType
          _S_find_last_set(basic_simd_mask<_Bs, _Abi> __k)
          { return std::__bit_width(__data(__k).to_ullong()) - 1; }
      };

    template <typename _Tp, _SimdSizeType _Np>
      struct _DeduceAbi
      {};

    // try all native ABIs (including scalar) first
    template <typename _Tp, _SimdSizeType _Np>
      requires (_AllNativeAbis::template _S_has_valid_abi<_Tp, _Np>)
      struct _DeduceAbi<_Tp, _Np>
      { using type = _AllNativeAbis::_FirstValidAbi<_Tp, _Np>; };

    // fall back to fixed_size only if scalar and native ABIs don't match
    template <typename _Tp, _SimdSizeType _Np>
      requires (not _AllNativeAbis::template _S_has_valid_abi<_Tp, _Np>
                  and _AbiCombine<_Np>::template _S_is_valid_v<_Tp>)
      struct _DeduceAbi<_Tp, _Np>
      { using type = _AbiCombine<_Np>; };

    template <typename _Tp, _SimdSizeType _Np>
      using __deduce_t = typename _DeduceAbi<_Tp, _Np>::type;
  }
}

#endif  // PROTOTYPE_SIMD_ABI_H_

// vim: et
