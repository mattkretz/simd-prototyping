/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_ABI_H_
#define PROTOTYPE_SIMD_ABI_H_

#include <cstdint>
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

    template <typename _Tp>
      concept __arithmetic = integral<_Tp> || floating_point<_Tp>;

    template <typename _Tp>
      concept __vectorizable
        = __arithmetic<_Tp> and not same_as<_Tp, bool> and not same_as<_Tp, long double>;

    template <typename _Abi0, size_t... _Is>
      struct _SimdImplArray;

    template <typename _Abi0, size_t... _Is>
      struct _MaskImplArray;

    template <typename _Abi0, _SimdSizeType _Np>
      struct _AbiArray
      {
        static constexpr _SimdSizeType _S_abiarray_size = _Np;

        template <typename _Tp>
          static constexpr _SimdSizeType _S_size = _Np * _Abi0::template _S_size<_Tp>;

        template <typename _Tp>
          static constexpr _SimdSizeType _S_full_size = _Np * _Abi0::template _S_size<_Tp>;

        template <typename _Tp>
          static constexpr bool _S_is_partial = false;

        template <typename _Tp>
          static constexpr _SimdSizeType _S_max_size
            = std::max(_SimdSizeType(64), _SimdSizeType(8 * _Abi0::template _S_size<_Tp>));

        struct _IsValidAbiTag
        : __bool_constant<_Abi0::_IsValidAbiTag::value and (_Np > 1)>
        {};

        template <typename _Tp>
          struct _IsValidSizeFor
          : __bool_constant<(_S_size<_Tp> <= _S_max_size<_Tp>)
                              and not _Abi0::template _S_is_partial<_Tp>
                              and _Abi0::template _IsValidSizeFor<_Tp>::value>
          {};

        template <typename _Tp>
          struct _IsValid
          : conjunction<_IsValidAbiTag, __is_vectorizable<_Tp>, _IsValidSizeFor<_Tp>>
          {};

        template <typename _Tp>
          static constexpr bool _S_is_valid_v = _IsValid<_Tp>::value;

        using _SimdImpl = decltype([]<size_t... _Is>(index_sequence<_Is...>)
                                     -> _SimdImplArray<_Abi0, _Is...> {
                                       return {};
                                     }(make_index_sequence<_Np>()));

        using _MaskImpl = decltype([]<size_t... _Is>(index_sequence<_Is...>)
                                     -> _MaskImplArray<_Abi0, _Is...> {
                                       return {};
                                     }(make_index_sequence<_Np>()));

        template <typename _Tp>
          struct __traits
          : _InvalidTraits
          {};

        template <typename _Tp>
          requires _S_is_valid_v<_Tp>
          struct __traits<_Tp>
          {
            using _IsValid = true_type;

            using _SimdImpl = decltype([]<size_t... _Is>(index_sequence<_Is...>)
                                         -> _SimdImplArray<_Abi0, _Is...> {
                                           return {};
                                         }(make_index_sequence<_Np>()));

            using _MaskImpl = decltype([]<size_t... _Is>(index_sequence<_Is...>)
                                         -> _MaskImplArray<_Abi0, _Is...> {
                                           return {};
                                         }(make_index_sequence<_Np>()));

            using _SimdMember
              = std::array<typename _Abi0::template __traits<_Tp>::_SimdMember, _Np>;

            using _MaskMember
              = std::array<typename _Abi0::template __traits<_Tp>::_MaskMember, _Np>;

            static constexpr size_t _S_simd_align = alignof(_SimdMember);

            static constexpr size_t _S_mask_align = alignof(_MaskMember);

            static constexpr bool _S_is_partial = false;

            struct _SimdBase
            {};

            struct _MaskBase
            {};

            class _MaskCastType
            {};

            class _SimdCastType
            {};
          };
      };

    template <typename _Abi0, size_t... _Is>
      struct _SimdImplArray
      {
        static constexpr _SimdSizeType _Np = sizeof...(_Is);

        using abi_type = _AbiArray<_Abi0, _Np>;

        template <typename _Tp>
          using _TypeTag = _Tp*;

        template <typename _Tp>
          using _SimdMember = typename abi_type::template __traits<_Tp>::_SimdMember;

        template <typename _Tp>
          using _ValueTypeOf = typename _VectorTraits<typename _Tp::value_type>::value_type;

        template <typename _Tp>
          using _MaskMember = typename abi_type::template __traits<
                                conditional_t<__vectorizable<_Tp>, _Tp,
                                              _ValueTypeOf<_Tp>>>::_MaskMember;

        template <typename _Tp>
          static constexpr _SimdSizeType _S_size = abi_type::template _S_size<_Tp>;

        template <typename _Tp>
          static constexpr _SimdSizeType _S_chunk_size = _Abi0::template _S_size<_Tp>;

        using _Impl0 = typename _Abi0::_SimdImpl;

        using _MaskImpl = typename abi_type::_MaskImpl;

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdMember<_Tp>
          _S_broadcast(_Tp __x) noexcept
          { return {((void)_Is, _Impl0::_S_broadcast(__x))...}; }

        template <typename _Fp, typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdMember<_Tp>
          _S_generator(_Fp&& __gen, _TypeTag<_Tp> __tag)
          {
            return {_Impl0::_S_generator([&](auto __i) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                      return invoke(__gen, std::integral_constant<
                                             _SimdSizeType, __i + _Is * _S_chunk_size<_Tp>>());
                    }, __tag)...};
          }

        template <typename _Tp, typename _Up>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdMember<_Tp>
          _S_load(const _Up* __mem, _TypeTag<_Tp> __tag) noexcept
          { return {_Impl0::_S_load(__mem + _Is * _S_chunk_size<_Tp>, __tag)...}; }

        template <typename _Tp, typename _Up>
          static constexpr inline _Tp
          _S_masked_load(_Tp const& __merge, _MaskMember<_Tp> const& __k,
                         const _Up* __mem) noexcept
          {
            _SimdMember<_Tp> __ret = __merge;
            (_Impl0::_S_masked_load(__ret[_Is], __k[_Is], __mem + _Is * _S_chunk_size<_Tp>), ...);
            return __ret;
          }

        template <typename _Tp, typename _Up>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_store(_SimdMember<_Tp> const& __v, _Up* __mem, _TypeTag<_Tp> __tag) noexcept
          { (_Impl0::_S_store(__v[_Is], __mem + _Is * _S_chunk_size<_Tp>, __tag), ...); }

        template <typename _Tp, typename _Up>
          static constexpr inline void
          _S_masked_store(const _Tp __v, _Up* __mem, const _MaskMember<_Tp> __k) noexcept
          { (_Impl0::_S_masked_store(__v[_Is], __mem + _Is * _S_chunk_size<_Tp>, __k[_Is]), ...); }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_complement(_Tp __x) noexcept
          { return {_Impl0::_S_complement(__x[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_unary_minus(_Tp __x) noexcept
          { return {_Impl0::_S_unary_minus(__x[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_plus(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_plus(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_minus(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_minus(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_multiplies(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_multiplies(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_divides(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_divides(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_modulus(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_modulus(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_bit_and(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_bit_and(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_bit_or(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_bit_or(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_bit_xor(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_bit_xor(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_bit_shift_left(_Tp __x, _Tp __y) noexcept
          { return {_Impl0::_S_bit_shift_left(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_bit_shift_right(_Tp __x, _Tp __y) noexcept
          { return {_Impl0::_S_bit_shift_right(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_bit_shift_left(_Tp __x, int __y) noexcept
          { return {_Impl0::_S_bit_shift_left(__x[_Is], __y)...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_bit_shift_right(_Tp __x, int __y) noexcept
          { return {_Impl0::_S_bit_shift_right(__x[_Is], __y)...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_equal_to(_Tp __x, _Tp __y) noexcept
          { return {_Impl0::_S_equal_to(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_not_equal_to(_Tp __x, _Tp __y) noexcept
          { return __x._M_data != __y._M_data; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_less(_Tp __x, _Tp __y) noexcept
          { return __x._M_data < __y._M_data; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_less_equal(_Tp __x, _Tp __y) noexcept
          { return __x._M_data <= __y._M_data; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_negate(_Tp __x) noexcept
          { return !__x._M_data; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_increment(_SimdMember<_Tp>& __x)
          { (_Impl0::_S_increment(__x[_Is]), ...); }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_decrement(_SimdMember<_Tp>& __x)
          { (_Impl0::_S_decrement(__x[_Is]), ...); }

        template <typename _Tp, typename _Up>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_set(_SimdMember<_Tp>& __v, int __i, _Up&& __x) noexcept
          {
            ((__i >= _Is * _S_chunk_size<_Tp> and __i < (_Is + 1) * _S_chunk_size<_Tp>
              ? _Impl0::_S_set(__v[_Is], __i - _Is * _S_chunk_size<_Tp>, static_cast<_Up&&>(__x))
              : 0), ...);
          }
      };

    template <typename _Abi0, size_t... _Is>
      struct _MaskImplArray
      {

      };

    template <_SimdSizeType _Np, typename _Tag, typename = experimental::__detail::__odr_helper>
      struct _MaskImplAbiCombine;

    template <_SimdSizeType _Np, typename _Tag>
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
                       _SimdSizeType(8 * experimental::simd_size_v<_Tp, _NativeAbi<_Tp>>));

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

        using _MaskImpl = _MaskImplAbiCombine<_Np, _Tag>;

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

            using _MaskImpl = _MaskImplAbiCombine<_Np, _Tag>;

            using _SimdMember = __fixed_size_storage_t<_Tp, _Np>;

            using _MaskMember = _SanitizedBitMask<_Np>;

            static constexpr size_t _S_simd_align = alignof(_SimdMember);

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

    template <_SimdSizeType _Np, typename _Tag, typename>
      struct _MaskImplAbiCombine
      {
        static_assert(
          sizeof(uint64_t) * __CHAR_BIT__ >= _Np,
          "The fixed_size implementation relies on one uint64_t being able to store "
          "all boolean elements."); // required in load & store

        using _Abi = _AbiCombine<_Np, _Tag>;

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
          // TODO: uint8_t is not necessarily the best type to use here. For smaller
          // _Np uint16_t, uint32_t, uint64_t, float, and double can be more efficient.
          uint64_t __r = 0;
          using _Vs = __fixed_size_storage_t<uint8_t, _Np>;
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
        { _FirstAbi<uint8_t>::_CommonImpl::_S_store_bool_array(__bitmask, __mem); }

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

    // next try _AbiArray of _NativeAbi
    template <typename _Tp, _SimdSizeType _Np>
      requires (not _AllNativeAbis::template _S_has_valid_abi<_Tp, _Np>
                  and _Np % _NativeAbi<_Tp>::template _S_size<_Tp> == 0)
      struct _DeduceAbi<_Tp, _Np>
      { using type = _AbiArray<_NativeAbi<_Tp>, _Np / _NativeAbi<_Tp>::template _S_size<_Tp>>; };

    // fall back to _AbiCombine of inhomogenous ABI tags
    template <typename _Tp, _SimdSizeType _Np>
      requires (not _AllNativeAbis::template _S_has_valid_abi<_Tp, _Np>
                  and _Np % _NativeAbi<_Tp>::template _S_size<_Tp> != 0
                  and _AbiCombine<_Np, _NativeAbi<_Tp>>::template _S_is_valid_v<_Tp>)
      struct _DeduceAbi<_Tp, _Np>
      { using type = _AbiCombine<_Np, _NativeAbi<_Tp>>; };

    template <typename _Tp, _SimdSizeType _Np>
      using __deduce_t = typename _DeduceAbi<_Tp, _Np>::type;
  }
}

#endif  // PROTOTYPE_SIMD_ABI_H_

// vim: et
