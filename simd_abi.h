/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_ABI_H_
#define PROTOTYPE_SIMD_ABI_H_

#include <cstdint>
#include "detail.h"
#include "constexpr_wrapper.h"

namespace std
{
  namespace __detail
  {
    using namespace vir::literals;

    template <typename _Abi0, size_t... _Is>
      struct _SimdImplArray;

    template <typename _Abi0, size_t... _Is>
      struct _MaskImplArray;

    template <typename _Tp>
      struct __is_vectorizable
      : bool_constant<__vectorizable<_Tp>>
      {};

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
          : __pv2::_InvalidTraits
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

        template <__vectorizable _Tp>
          using _SimdMember = typename abi_type::template __traits<_Tp>::_SimdMember;

        template <typename _Tp>
          using _ValueTypeOf = typename __pv2::_VectorTraits<typename _Tp::value_type>::value_type;

        template <typename _Tp>
          struct _MaskMemberImpl
          { using type = typename abi_type::template __traits<_ValueTypeOf<_Tp>>::_MaskMember; };

        template <__vectorizable _Tp>
          struct _MaskMemberImpl<_Tp>
          { using type = typename abi_type::template __traits<_Tp>::_MaskMember; };

        template <typename _Tp>
          using _MaskMember = typename _MaskMemberImpl<_Tp>::type;

        template <__vectorizable _Tp>
          static constexpr _SimdSizeType _S_size = abi_type::template _S_size<_Tp>;

        template <__vectorizable _Tp>
          static constexpr _SimdSizeType _S_chunk_size = _Abi0::template _S_size<_Tp>;

        using _Impl0 = typename _Abi0::_SimdImpl;

        using _MaskImpl = typename abi_type::_MaskImpl;

        template <__vectorizable _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdMember<_Tp>
          _S_broadcast(_Tp __x) noexcept
          { return {((void)_Is, _Impl0::_S_broadcast(__x))...}; }

        template <typename _Fp, __vectorizable _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdMember<_Tp>
          _S_generator(_Fp&& __gen, _TypeTag<_Tp> __tag)
          {
            return {_Impl0::_S_generator([&](auto __i) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                      return invoke(__gen, __ic<__i + _Is * _S_chunk_size<_Tp>>);
                    }, __tag)...};
          }

        template <__vectorizable _Tp, typename _Up>
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

        template <__vectorizable _Tp, typename _Up>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_store(_SimdMember<_Tp> const& __v, _Up* __mem, _TypeTag<_Tp> __tag) noexcept
          { (_Impl0::_S_store(__v[_Is], __mem + _Is * _S_chunk_size<_Tp>, __tag), ...); }

        template <typename _Tp, typename _Up>
          static constexpr inline void
          _S_masked_store(_Tp const& __v, _Up* __mem, const _MaskMember<_Tp> __k) noexcept
          { (_Impl0::_S_masked_store(__v[_Is], __mem + _Is * _S_chunk_size<_Tp>, __k[_Is]), ...); }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_complement(_Tp const& __x) noexcept
          { return {_Impl0::_S_complement(__x[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_unary_minus(_Tp const& __x) noexcept
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
          _S_bit_shift_left(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_bit_shift_left(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_bit_shift_right(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_bit_shift_right(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_bit_shift_left(_Tp const& __x, int __y) noexcept
          { return {_Impl0::_S_bit_shift_left(__x[_Is], __y)...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _Tp
          _S_bit_shift_right(_Tp const& __x, int __y) noexcept
          { return {_Impl0::_S_bit_shift_right(__x[_Is], __y)...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_equal_to(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_equal_to(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_not_equal_to(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_not_equal_to(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_less(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_less(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_less_equal(_Tp const& __x, _Tp const& __y) noexcept
          { return {_Impl0::_S_less_equal(__x[_Is], __y[_Is])...}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_negate(_Tp const& __x) noexcept
          { return {_Impl0::_S_negate(__x[_Is])...}; }

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

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_masked_assign(const _MaskMember<_Tp> __k, _Tp& __lhs,
                           const __type_identity_t<_Tp>& __rhs)
          { (_Impl0::_S_masked_assign(__k[_Is], __lhs[_Is], __rhs[_Is]), ...); }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_masked_assign(const _MaskMember<_Tp> __k, _Tp& __lhs, _ValueTypeOf<_Tp> __rhs)
          { (_Impl0::_S_masked_assign(__k[_Is], __lhs[_Is], __rhs), ...); }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr bool
          _S_is_constprop(const _Tp& __x)
          { return (__x[_Is]._M_is_constprop() and ...); }
      };

    template <typename _Abi0, size_t... _Is>
      struct _MaskImplArray
      {
        static constexpr _SimdSizeType _Np = sizeof...(_Is);

        using abi_type = _AbiArray<_Abi0, _Np>;

        template <typename _Ts>
          using mask_type = std::basic_simd_mask<sizeof(_Ts), _Abi0>;

        template <__vectorizable _Tp>
          using _MaskMember0 = typename _Abi0::template __traits<_Tp>::_MaskMember;

        template <__vectorizable _Tp>
          using _MaskMember = std::array<_MaskMember0<_Tp>, _Np>;

        template <__vectorizable _Tp>
          using _SimdMember = typename abi_type::template __traits<_Tp>::_SimdMember;

        template <typename _Tp>
          using _ValueTypeOf = typename __pv2::_VectorTraits<typename _Tp::value_type>::value_type;

        template <__vectorizable _Tp>
          static constexpr _SimdSizeType _S_size = abi_type::template _S_size<_Tp>;

        template <__vectorizable _Tp>
          static constexpr _SimdSizeType _S_chunk_size = _Abi0::template _S_size<_Tp>;

        using _Impl0 = typename _Abi0::_MaskImpl;

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC
          static constexpr mask_type<_Tp>
          _S_to_mask(const _MaskMember0<_Tp>& __k)
          { return {__pv2::__private_init, __k}; }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC
          static constexpr bool
          _S_any_of(std::array<_MaskMember0<_Tp>, _Np> const& __masks)
          { return (std::any_of(_S_to_mask<_Tp>(__masks[_Is])) or ...); }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC
          static constexpr bool
          _S_all_of(std::array<_MaskMember0<_Tp>, _Np> const& __masks)
          { return (std::all_of(_S_to_mask<_Tp>(__masks[_Is])) and ...); }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC
          static constexpr bool
          _S_none_of(std::array<_MaskMember0<_Tp>, _Np> const& __masks)
          { return (std::none_of(_S_to_mask<_Tp>(__masks[_Is])) and ...); }

        template <typename _Tp>
          static constexpr _SimdSizeType
          _S_find_first_set(std::array<_MaskMember0<_Tp>, _Np> const& __masks)
          {
            if (std::any_of(_S_to_mask<_Tp>(__masks[0])))
              return std::reduce_min_index(_S_to_mask<_Tp>(__masks[0]));

            for (int __i = 1; __i < _Np - 1; ++__i)
              {
                if (std::any_of(mask_type<_Tp>(__pv2::__private_init, __masks[__i])))
                  return __i * __masks[0]._S_size
                           + std::reduce_min_index(_S_to_mask<_Tp>(__masks[__i]));
              }
            return (_Np - 1) * __masks[0]._S_size
                           + std::reduce_min_index(_S_to_mask<_Tp>(__masks[_Np - 1]));
          }

        template <__vectorizable _Tp, typename _Fp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember<_Tp>
          _S_generator(_Fp&& __gen)
          {
            if constexpr (requires {_Impl0::template _S_generator<_Tp>(__gen);})
              return {_Impl0::template _S_generator<_Tp>([&](auto __i) {
                        return invoke(__gen, __ic<_Is * _S_chunk_size<_Tp> + __i>);
                      })...};
            else
              return {[&]<size_t... _Js>(vir::constexpr_value<int> auto __i,
                                         std::index_sequence<_Js...>) {
                return _MaskMember0<_Tp>{ -invoke(__gen, __ic<__i * _S_chunk_size<_Tp> + _Js>)... };
              }(vir::cw<_Is>, std::make_index_sequence<_S_chunk_size<_Tp>>())...};
          }

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr bool
          _S_is_constprop(const _Tp& __x)
          { return (__x[_Is]._M_is_constprop() and ...); }
      };

    template <typename _Tp, typename _A0, _SimdSizeType __offset>
      struct _SimdTupleMeta : public _A0::_SimdImpl
      {
        using _Abi = _A0;
        using _Traits = __pv2::_SimdTraits<_Tp, _Abi>;
        using _MaskImpl = typename _Abi::_MaskImpl;
        using _MaskMember = typename _Traits::_MaskMember;
        static constexpr auto _S_offset = vir::cw<__offset>;
        static constexpr auto _S_size = vir::cw<_SimdSizeType(simd_size_v<_Tp, _Abi>)>;
        static constexpr _MaskImpl _S_mask_impl = {};

        template <size_t _Np, bool _Sanitized>
          _GLIBCXX_SIMD_INTRINSIC static constexpr auto
          _S_submask(__pv2::_BitMask<_Np, _Sanitized> __bits)
          { return __bits.template _M_extract<_S_offset, _S_size>(); }

        template <size_t _Np, bool _Sanitized>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
          _S_make_mask(__pv2::_BitMask<_Np, _Sanitized> __bits)
          { return _MaskImpl::template _S_convert<_Tp>(_S_submask(__bits)._M_sanitized()); }

        _GLIBCXX_SIMD_INTRINSIC static constexpr unsigned long long
        _S_mask_to_shifted_ullong(_MaskMember __k)
        { return _MaskImpl::_S_to_bits(__k).to_ullong() << _S_offset; }

        _GLIBCXX_SIMD_INTRINSIC static constexpr basic_simd<_Tp, _A0>
        _S_to_simd(const _Traits::_SimdMember& __x0)
        { return basic_simd<_Tp, _A0>(__pv2::__private_init, __x0); }
      };

    template <typename _Tp, typename... _As>
      struct _SimdTuple
      {
        static_assert(sizeof...(_As) == 0);

        static constexpr auto _S_size = 0_cw;

        static constexpr auto _S_tuple_size = 0_cw;

        _GLIBCXX_SIMD_INTRINSIC constexpr bool
        _M_is_constprop() const
        { return true; }
      };

    template <typename _Tp, typename _A0, typename... _As>
      struct _SimdTupleData
      {
        static_assert(sizeof...(_As) != 0);
        typename _A0::__traits<_Tp>::_SimdMember _M_x;
        _SimdTuple<_Tp, _As...> _M_tail;
      };

    template <typename _Tp, typename _A0>
      struct _SimdTupleData<_Tp, _A0>
      {
        typename _A0::__traits<_Tp>::_SimdMember _M_x;
        static constexpr _SimdTuple<_Tp> _M_tail = {};
      };

    template <typename _Tp, typename _A0, typename... _As>
      struct _SimdTuple<_Tp, _A0, _As...> : _SimdTupleData<_Tp, _A0, _As...>
      {
        using _Base = _SimdTupleData<_Tp, _A0, _As...>;

        using _Base::_M_x;
        using _Base::_M_tail;

        using _SimdType = basic_simd<_Tp, _A0>;

        using _Simpl0 = _A0::_SimdImpl;

        using _Mimpl0 = _A0::_MaskImpl;

        static constexpr bool _S_recurse = sizeof...(_As) != 0;

        static constexpr auto _S_size = vir::cw<(simd_size_v<_Tp, _A0>)>;

        static constexpr auto _S_total_size
          = vir::cw<(simd_size_v<_Tp, _A0> + ... + simd_size_v<_Tp, _As>)>;

        static constexpr auto _S_tail_size = _SimdTuple<_Tp, _As...>::_S_size;

        static constexpr auto _S_tuple_size = vir::cw<_SimdSizeType(sizeof...(_As) + 1)>;

        _GLIBCXX_SIMD_INTRINSIC constexpr bool
        _M_is_constprop() const
        {
          bool __r;
          if constexpr (requires {_A0::_S_abiarray_size;})
            __r = _Simpl0::_S_is_constprop(_M_x);
          else
            __r = _M_x._M_is_constprop;
          return __r and _M_tail._M_is_constprop;
        }

        _GLIBCXX_SIMD_INTRINSIC constexpr _Tp
        operator[](_SimdSizeType __i) const
        {
          if (not __builtin_is_constant_evaluated())
            {
              const auto* __ptr = reinterpret_cast<const std::byte*>(this);
              _Tp __r;
              __builtin_memcpy(&__r, __ptr + __i * sizeof(_Tp), sizeof(_Tp));
              return __r;
            }
          else if (__i < _S_size)
            {
              if constexpr (_S_size == 1)
                return _M_x;
              else if constexpr (requires {_A0::_S_abiarray_size;})
                {
                  constexpr __detail::_SimdSizeType __n = _S_size / _A0::_S_abiarray_size;
                  return _M_x[__i / __n][__i % __n];
                }
              else
                return _M_x[__i];
            }
          else if constexpr (_S_recurse)
            return _M_tail[__i - _S_size];
          else
            __pv2::__invoke_ub("out-of-bounds subscript");
        }

        _GLIBCXX_SIMD_INTRINSIC constexpr auto
        _M_simd_at(vir::constexpr_value<int> auto __i) const
        {
          if constexpr (__i == 0)
            return _SimdType(__pv2::__private_init, _M_x);
          else if constexpr (_S_recurse)
            return _M_tail._M_simd_at(__i - 1_cw);
        }

        template <vir::constexpr_value<int> _Cv = decltype(0_cw)>
          _GLIBCXX_SIMD_INTRINSIC constexpr _SimdTuple&
          _M_forall(auto&& __fun, _Cv __total_offset = {})
          {
            __fun(_SimdTupleMeta<_Tp, _A0, __total_offset>(), _M_x);
            if constexpr (_S_recurse)
              _M_tail._M_forall(__fun, __total_offset + _S_size);
            return *this;
          }

        template <vir::constexpr_value<int> _Cv = decltype(0_cw)>
          _GLIBCXX_SIMD_INTRINSIC constexpr _SimdTuple&
          _M_forall(const _SimdTuple& __a, auto&& __fun, _Cv __total_offset = {})
          {
            __fun(_SimdTupleMeta<_Tp, _A0, __total_offset>(), _M_x, __a._M_x);
            if constexpr (_S_recurse)
              _M_tail._M_forall(_M_tail, __fun, __total_offset + _S_size);
            return *this;
          }

        template <vir::constexpr_value<int> _Cv = decltype(0_cw)>
          _GLIBCXX_SIMD_INTRINSIC constexpr const _SimdTuple&
          _M_forall(auto&& __fun, _Cv __total_offset = {}) const
          {
            __fun(_SimdTupleMeta<_Tp, _A0, __total_offset>(), _M_x);
            if constexpr (_S_recurse)
              _M_tail._M_forall(__fun, __total_offset + _S_size);
            return *this;
          }

        template <int __total_offset = 0>
          _GLIBCXX_SIMD_INTRINSIC constexpr __pv2::_SanitizedBitMask<_S_total_size>
          _M_test(auto&& __fun, auto&&... __more) const
          {
            const __pv2::_SanitizedBitMask<_S_size> __first
              = _A0::_MaskImpl::_S_to_bits(__fun(_SimdTupleMeta<_Tp, _A0, __total_offset>(),
                                                 _M_x, __more._M_x...));
            if constexpr (_S_recurse)
              return _M_tail.template _M_test<__total_offset + _S_size>(__fun, __more._M_tail...)
                       ._M_prepend(__first);
            else
              return __first;
          }

        constexpr
        _SimdTuple() = default;

        _GLIBCXX_SIMD_INTRINSIC constexpr
        _SimdTuple(auto &&__fun)
        { _M_forall(__fun); }
      };

    template <typename _Tp, _SimdSizeType _Np>
      struct _NextAbiTuple
      {
        using _Native = __pv2::_AllNativeAbis::_BestAbi<_Tp, _Np>;
        static constexpr int _S_native_size = simd_size_v<_Tp, _Native>;
        static constexpr int _S_array_size = _Np / _S_native_size;
        using type
          = std::conditional_t<_S_array_size >= 2, _AbiArray<_Native, _S_array_size>, _Native>;
      };

    template <typename _Tp, int _Np, typename _Tuple,
              typename _Next = typename _NextAbiTuple<_Tp, _Np>::type,
              int _Remain = _Np - int(simd_size_v<_Tp, _Next>)>
      struct __fixed_size_storage_builder;

    template <typename _Tp, int _Np, typename... _As, typename _Next>
      struct __fixed_size_storage_builder<_Tp, _Np, _SimdTuple<_Tp, _As...>, _Next, 0>
      { using type = _SimdTuple<_Tp, _As..., _Next>; };

    template <typename _Tp, int _Np, typename... _As, typename _Next, int _Remain>
      struct __fixed_size_storage_builder<_Tp, _Np, _SimdTuple<_Tp, _As...>, _Next, _Remain>
      {
        using type = typename __fixed_size_storage_builder<
                       _Tp, _Remain, _SimdTuple<_Tp, _As..., _Next>>::type;
      };

    template <typename _Tp, int _Np>
      using __fixed_size_storage_t
        = typename __fixed_size_storage_builder<_Tp, _Np, _SimdTuple<_Tp>>::type;

    template <_SimdSizeType _Np, typename _Tag, typename = experimental::__detail::__odr_helper>
      struct _SimdImplAbiCombine;

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

        _GLIBCXX_SIMD_INTRINSIC static constexpr __pv2::_SanitizedBitMask<_Np>
        _S_masked(__pv2::_BitMask<_Np> __x)
        { return __x._M_sanitized(); }

        _GLIBCXX_SIMD_INTRINSIC static constexpr __pv2::_SanitizedBitMask<_Np>
        _S_masked(__pv2::_SanitizedBitMask<_Np> __x)
        { return __x; }

        using _CommonImpl = __pv2::_CommonImplFixedSize;

        using _SimdImpl = _SimdImplAbiCombine<_Np, _Tag>;

        using _MaskImpl = _MaskImplAbiCombine<_Np, _Tag>;

        template <typename _Tp>
          struct __traits
          : __pv2::_InvalidTraits
          {};

        template <typename _Tp>
          requires _S_is_valid_v<_Tp>
          struct __traits<_Tp>
          {
            using _IsValid = true_type;

            using _SimdImpl = _SimdImplAbiCombine<_Np, _Tag>;

            using _MaskImpl = _MaskImplAbiCombine<_Np, _Tag>;

            using _SimdMember = __fixed_size_storage_t<_Tp, _Np>;

            using _MaskMember = __pv2::_SanitizedBitMask<_Np>;

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
      struct _SimdImplAbiCombine
      {
        using _MaskMember = __pv2::_SanitizedBitMask<_Np>;

        template <typename _Tp>
          using _SimdMember = __fixed_size_storage_t<_Tp, _Np>;

        using _Abi = _AbiCombine<_Np, _Tag>;

        template <typename _Tp>
          using _Simd = basic_simd<_Tp, _Abi>;

        template <typename _Tp>
          using _TypeTag = _Tp*;

        template <typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdMember<_Tp>
          _S_broadcast(_Tp __x) noexcept
          {
            return _SimdMember<_Tp>([&__x](auto __meta, auto& __chunk)
                   _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                     __chunk = __meta._S_broadcast(__x);
                   });
          }

        template <typename _Fp, typename _Tp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdMember<_Tp>
          _S_generator(_Fp&& __gen, _TypeTag<_Tp>)
          {
            return _SimdMember<_Tp>([&__gen](auto __meta, auto& __chunk)
                   _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                     __chunk = __meta._S_generator([&](vir::constexpr_value<int> auto __i)
                               _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                                 return invoke(__gen, __i + __meta._S_offset);
                               }, _TypeTag<_Tp>());
                   });
          }

        template <typename _Tp, typename _Up>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdMember<_Tp>
          _S_load(const _Up* __mem, _TypeTag<_Tp>)
          {
            return _SimdMember<_Tp>([&](auto __meta, auto& __chunk)
                   _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                     __chunk = __meta._S_load(__mem + __meta._S_offset, _TypeTag<_Tp>());
                   });
          }

        template <typename _Tp, typename... _As, typename _Up>
          _GLIBCXX_SIMD_INTRINSIC static _SimdTuple<_Tp, _As...>
          _S_masked_load(const _SimdTuple<_Tp, _As...>& __old,
                         const _MaskMember __bits, const _Up* __mem)
          {
            auto __merge = __old;
            __merge._M_forall([&](auto __meta, auto& __chunk) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
              if (__meta._S_submask(__bits).any())
#pragma GCC diagnostic push
                // Dereferencing __mem + __meta._S_offset could be UB ([expr.add]/4.3).
                // It is the responsibility of the caller of the masked load (via the mask's value)
                // to avoid UB. Consequently, the compiler may assume this branch is unreachable, if
                // the pointer arithmetic is UB.
#pragma GCC diagnostic ignored "-Warray-bounds"
                __chunk = __meta._S_masked_load(__chunk, __meta._S_make_mask(__bits),
                                                __mem + __meta._S_offset);
#pragma GCC diagnostic pop
            });
            return __merge;
          }

        template <typename _Tp, typename _Up>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_store(const _SimdMember<_Tp>& __v, _Up* __mem, _TypeTag<_Tp>)
          {
            __v._M_forall([&](auto __meta, auto __chunk) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
              __meta._S_store(__chunk, __mem + __meta._S_offset, _TypeTag<_Tp>());
            });
          }

        template <typename _Tp, typename... _As, typename _Up>
          _GLIBCXX_SIMD_INTRINSIC static void
          _S_masked_store(const _SimdTuple<_Tp, _As...>& __v, _Up* __mem, const _MaskMember __bits)
          {
            __v._M_forall([&](auto __meta, auto __chunk) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
              if (__meta._S_submask(__bits).any())
#pragma GCC diagnostic push
                // __mem + __mem._S_offset could be UB ([expr.add]/4.3, but it punts
                // the responsibility for avoiding UB to the caller of the masked
                // store via the mask. Consequently, the compiler may assume this
                // branch is unreachable, if the pointer arithmetic is UB.
#pragma GCC diagnostic ignored "-Warray-bounds"
                __meta._S_masked_store(__chunk, __mem + __meta._S_offset,
                                       __meta._S_make_mask(__bits));
#pragma GCC diagnostic pop
            });
          }

        template <typename _Tp, typename... _As>
          static constexpr inline _MaskMember
          _S_negate(const _SimdTuple<_Tp, _As...>& __x)
          {
            _MaskMember __bits = 0;
            __x._M_forall([&__bits](auto __meta, auto __chunk) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                __bits |= __meta._S_mask_to_shifted_ullong(__meta._S_negate(__chunk));
              });
            return __bits;
          }

        template <typename _Tp, typename _BinaryOperation, typename _A0, typename _A1,
                  typename... _Abis>
          static constexpr _Tp
          _S_reduce(const std::basic_simd<_Tp, _A0>& __x0,
                    const std::basic_simd<_Tp, _A1>& __x1,
                    const std::basic_simd<_Tp, _Abis>&... __tail,
                    const _BinaryOperation& __binary_op)
          {
            using _X0 = basic_simd<_Tp, _A0>;
            using _X1 = basic_simd<_Tp, _A1>;
            if constexpr (_X0::size.value == _X1::size.value)
              return _S_reduce(__binary_op(__x0, __x1), __tail..., __binary_op);
            else if constexpr (_X0::size.value > _X1::size.value)
              {
                using _X2 = std::resize_simd_t<__bit_ceil(_X0::size.value) / 2, _X0>;
                auto __x02 = std::simd_split<_X2>(__x0);
                if constexpr (_X1::size.value == _X2::size.value)
                  {
                    std::get<0>(__x02) = __binary_op(std::get<0>(__x02), __x1);
                    return std::apply([&](const auto&... __ys) {
                             return _S_reduce(__ys..., __tail..., __binary_op);
                           }, __x02);
                  }
                else if constexpr (_X1::size.value > _X2::size.value)
                  {
                    auto __x12 = std::simd_split<_X2>(__x1);
                    auto __x2 = __binary_op(std::get<0>(__x02), std::get<0>(__x12));
                    return std::apply([&](const auto&, const auto&... __x0s) {
                             return std::apply([&](const auto&, const auto&... __x1s) {
                                      return _S_reduce(__x2, __x0s..., __x1s..., __binary_op);
                             }, __x12);
                    }, __x02);
                  }
              }
            else
              {
                using _X2 = std::resize_simd_t<__bit_ceil(_X1::size.value) / 2, _X1>;
                auto __x12 = std::simd_split<_X2>(__x1);
                if constexpr (_X0::size.value == _X2::size.value)
                  {
                    std::get<0>(__x12) = __binary_op(std::get<0>(__x12), __x0);
                    return std::apply([&](const auto&... __ys) {
                             return _S_reduce(__ys..., __tail..., __binary_op);
                           }, __x12);
                  }
                else if constexpr (_X1::size.value > _X2::size.value)
                  {
                    auto __x02 = std::simd_split<_X2>(__x0);
                    auto __x2 = __binary_op(std::get<0>(__x02), std::get<0>(__x12));
                    return std::apply([&](const auto&, const auto&... __x0s) {
                             return std::apply([&](const auto&, const auto&... __x1s) {
                                      return _S_reduce(__x2, __x0s..., __x1s..., __binary_op);
                             }, __x12);
                    }, __x02);
                  }
              }
          }

        template <typename _Tp, typename _BinaryOperation>
          static constexpr _Tp
          _S_reduce(const _Simd<_Tp>& __x, const _BinaryOperation& __binary_op)
          {
            using _Tup = _SimdMember<_Tp>;
            const _Tup& __tup = __data(__x);
            if constexpr (_Tup::_S_tuple_size == 1)
              return std::reduce(__tup._M_simd_at(0_cw), __binary_op);
            else
              return _S_reduce(__tup._M_simd_at(0_cw), __tup._M_simd_at(1_cw), __binary_op);
          }

        template <typename _Tp, typename... _As>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdTuple<_Tp, _As...>
          _S_min(_SimdTuple<_Tp, _As...> __a, const _SimdTuple<_Tp, _As...>& __b)
          {
            return __a._M_forall(__b, [](auto __meta, auto& __aa, auto __bb)
               _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                 __aa = __meta._S_min(__aa, __bb);
               });
          }

        template <typename _Tp, typename... _As>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdTuple<_Tp, _As...>
          _S_max(_SimdTuple<_Tp, _As...> __a, const _SimdTuple<_Tp, _As...>& __b)
          {
            return __a._M_forall(__b, [](auto __meta, auto& __aa, auto __bb)
               _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                 __aa = __meta._S_max(__aa, __bb);
               });
          }

        template <typename _Tp, typename... _As>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdTuple<_Tp, _As...>
          _S_complement(_SimdTuple<_Tp, _As...> __x) noexcept
          {
            return __x._M_forall([](auto __meta, auto& __xx) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                     __xx = __meta._S_complement(__xx);
                   });
          }

        template <typename _Tp, typename... _As>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdTuple<_Tp, _As...>
          _S_unary_minus(_SimdTuple<_Tp, _As...> __x) noexcept
          {
            return __x._M_forall([](auto __meta, auto& __xx) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                     __xx = __meta._S_unary_minus(__xx);
                   });
          }

#define _GLIBCXX_SIMD_FIXED_OP(name_)                                                             \
        template <typename _Tp, typename... _As>                                                  \
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdTuple<_Tp, _As...>                        \
          name_(_SimdTuple<_Tp, _As...> __x, const _SimdTuple<_Tp, _As...>& __y)                  \
          {                                                                                       \
            return __x._M_forall(__y, [](auto __meta, auto& __xx, auto __yy)                      \
                   _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {                                           \
                     __xx = __meta.name_(__xx, __yy);                                             \
                   });                                                                            \
          }

        _GLIBCXX_SIMD_FIXED_OP(_S_plus)
        _GLIBCXX_SIMD_FIXED_OP(_S_minus)
        _GLIBCXX_SIMD_FIXED_OP(_S_multiplies)
        _GLIBCXX_SIMD_FIXED_OP(_S_divides)
        _GLIBCXX_SIMD_FIXED_OP(_S_modulus)
        _GLIBCXX_SIMD_FIXED_OP(_S_bit_and)
        _GLIBCXX_SIMD_FIXED_OP(_S_bit_or)
        _GLIBCXX_SIMD_FIXED_OP(_S_bit_xor)
        _GLIBCXX_SIMD_FIXED_OP(_S_bit_shift_left)
        _GLIBCXX_SIMD_FIXED_OP(_S_bit_shift_right)

#undef _GLIBCXX_SIMD_FIXED_OP

        template <typename _Tp, typename... _As>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdTuple<_Tp, _As...>
          _S_bit_shift_left(_SimdTuple<_Tp, _As...> __x, int __y)
          {
            return __x._M_forall([__y](auto __impl, auto& __xx)
                   _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                     __xx = __impl._S_bit_shift_left(__xx, __y);
                   });
          }

        template <typename _Tp, typename... _As>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _SimdTuple<_Tp, _As...>
          _S_bit_shift_right(_SimdTuple<_Tp, _As...> __x, int __y)
          {
            return __x._M_forall([__y](auto __impl, auto& __xx)
                   _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                     __xx = __impl._S_bit_shift_right(__xx, __y);
                   });
          }

#if 0
#define _GLIBCXX_SIMD_APPLY_ON_TUPLE(_RetTp, __name)                                               \
        template <typename _Tp, typename... _As, typename... _More>                                \
          static inline __fixed_size_storage_t<_RetTp, _Np>                                        \
          _S_##__name(const _SimdTuple<_Tp, _As...>& __x, const _More&... __more)                  \
          {                                                                                        \
            if constexpr (sizeof...(_More) == 0)                                                   \
              {                                                                                    \
                if constexpr (is_same_v<_Tp, _RetTp>)                                              \
                  {                                                                                \
                    auto __r = __x;                                                                \
                    return __r._M_forall([](auto __meta, auto& __xx)                               \
                         _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {                                      \
                             __xx = __data(__name(__meta._S_to_simd(__xx)));                       \
                         });                                                                       \
                  }                                                                                \
                else                                                                               \
                  return __optimize_simd_tuple(                                    \
                           __x.template _M_apply_r<_RetTp>(                        \
                             [](auto __impl, auto __xx)                            \
                         _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA                  \
                { return __impl._S_##__name(__xx); }));               \
              }                                                                    \
            else if constexpr (                                                    \
                   is_same_v<                                                           \
                     _Tp,                                                               \
                     _RetTp> && (... && is_same_v<_SimdTuple<_Tp, _As...>, _More>) )    \
              return __x._M_apply_per_chunk(                                       \
                       [](auto __impl, auto __xx, auto... __pack)                  \
                     constexpr _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA              \
            {                                                           \
                                 using _V = typename decltype(__impl)::simd_type;          \
                                 return __data(__name(_V(__private_init, __xx),            \
                                                      _V(__private_init, __pack)...));     \
                               }, __more...);                                              \
            else if constexpr (is_same_v<_Tp, _RetTp>)                             \
              return __x._M_apply_per_chunk(                                       \
                       [](auto __impl, auto __xx, auto... __pack)                  \
                     constexpr _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA              \
            {                                                           \
                                 using _V = typename decltype(__impl)::simd_type;          \
                                 return __data(__name(_V(__private_init, __xx),            \
                                                      __autocvt_to_simd(__pack)...));      \
                               }, __more...);                                              \
            else                                                                   \
              __assert_unreachable<_Tp>();                                         \
          }

        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, acos)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, asin)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, atan)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, atan2)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, cos)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, sin)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, tan)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, acosh)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, asinh)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, atanh)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, cosh)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, sinh)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, tanh)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, exp)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, exp2)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, expm1)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(int, ilogb)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, log)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, log10)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, log1p)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, log2)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, logb)
        // modf implemented in simd_math.h
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, scalbn) // double scalbn(double x, int exp);
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, scalbln)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, cbrt)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, abs)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, fabs)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, pow)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, sqrt)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, erf)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, erfc)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, lgamma)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, tgamma)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, trunc)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, ceil)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, floor)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, nearbyint)

        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, rint)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(long, lrint)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(long long, llrint)

        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, round)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(long, lround)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(long long, llround)

        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, ldexp)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, fmod)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, remainder)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, copysign)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, nextafter)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, fdim)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, fmax)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, fmin)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(_Tp, fma)
        _GLIBCXX_SIMD_APPLY_ON_TUPLE(int, fpclassify)
#undef _GLIBCXX_SIMD_APPLY_ON_TUPLE

        template <typename _Tp, typename... _Abis>
          static inline _SimdTuple<_Tp, _Abis...>
          _S_remquo(_SimdTuple<_Tp, _Abis...> __x, const _SimdTuple<_Tp, _Abis...>& __y,
                    __fixed_size_storage_t<int, _SimdTuple<_Tp, _Abis...>::_S_size()>* __z)
          {
            return __x._M_forall(__y, [](auto __impl, auto& __xx, const auto __yy, auto& __zz)
                   _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA
            { return __impl._S_remquo(__xx, __yy, &__zz); },
                     __y, *__z);
          }

        template <typename _Tp, typename... _As>
          static inline _SimdTuple<_Tp, _As...>
          _S_frexp(const _SimdTuple<_Tp, _As...>& __x,
                   __fixed_size_storage_t<int, _Np>& __exp) noexcept
          {
            return __x._M_apply_per_chunk(
                     [](auto __impl, const auto& __a, auto& __b) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                       return __data(frexp(typename decltype(__impl)::simd_type(__private_init, __a),
                                           __autocvt_to_simd(__b)));
                     }, __exp);
          }

#define _GLIBCXX_SIMD_TEST_ON_TUPLE_(name_)                                              \
        template <typename _Tp, typename... _As>                                             \
          static constexpr _MaskMember                                                          \
          _S_##name_(const _SimdTuple<_Tp, _As...>& __x) noexcept                            \
          {                                                                                  \
            return _M_test([] (auto __impl, auto __xx) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA  { \
                     return __impl._S_##name_(__xx);                                         \
                   }, __x);                                                                  \
          }

        _GLIBCXX_SIMD_TEST_ON_TUPLE_(isinf)
        _GLIBCXX_SIMD_TEST_ON_TUPLE_(isfinite)
        _GLIBCXX_SIMD_TEST_ON_TUPLE_(isnan)
        _GLIBCXX_SIMD_TEST_ON_TUPLE_(isnormal)
        _GLIBCXX_SIMD_TEST_ON_TUPLE_(signbit)
#undef _GLIBCXX_SIMD_TEST_ON_TUPLE_
#endif

        template <typename... _Ts>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_increment(_SimdTuple<_Ts...>& __x)
          {
            __x._M_forall([](auto __meta, auto& __chunk) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
              __chunk = __meta._S_increment(__chunk);
            });
          }

        template <typename... _Ts>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_decrement(_SimdTuple<_Ts...>& __x)
          {
            __x._M_forall([](auto __meta, auto& __chunk) _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
              __meta._S_decrement(__chunk);
            });
          }

#define _GLIBCXX_SIMD_CMP_OPERATIONS(__cmp)                                                        \
        template <typename _Tp, typename... _As>                                                   \
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember                                     \
          __cmp(const _SimdTuple<_Tp, _As...>& __x, const _SimdTuple<_Tp, _As...>& __y)            \
          {                                                                                        \
            return __x._M_test([](auto __impl, auto __xx, auto __yy)                               \
                   _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {                                            \
                     return __impl.__cmp(__xx, __yy);                                              \
                   }, __y);                                                                        \
          }

        _GLIBCXX_SIMD_CMP_OPERATIONS(_S_equal_to)
        _GLIBCXX_SIMD_CMP_OPERATIONS(_S_not_equal_to)
        _GLIBCXX_SIMD_CMP_OPERATIONS(_S_less)
        _GLIBCXX_SIMD_CMP_OPERATIONS(_S_less_equal)
        _GLIBCXX_SIMD_CMP_OPERATIONS(_S_isless)
        _GLIBCXX_SIMD_CMP_OPERATIONS(_S_islessequal)
        _GLIBCXX_SIMD_CMP_OPERATIONS(_S_isgreater)
        _GLIBCXX_SIMD_CMP_OPERATIONS(_S_isgreaterequal)
        _GLIBCXX_SIMD_CMP_OPERATIONS(_S_islessgreater)
        _GLIBCXX_SIMD_CMP_OPERATIONS(_S_isunordered)
#undef _GLIBCXX_SIMD_CMP_OPERATIONS

        template <typename _Tp, typename... _As, typename _Up>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_set(_SimdTuple<_Tp, _As...>& __v, int __i, _Up&& __x) noexcept
          {
            if (not __builtin_is_constant_evaluated())
              {
                auto* __ptr = reinterpret_cast<std::byte*>(&__v);
                const _Tp __y = static_cast<_Up&&>(__x);
                __builtin_memcpy(__ptr + __i * sizeof(_Tp), &__y, sizeof(_Tp));
              }
            else
              __v._M_forall([&](auto __meta, auto& __chunk) {
                if (__i >= __meta._S_offset and __i < __meta._S_offset + __meta._S_size)
                  __chunk[__i - __meta._S_offset] = static_cast<_Up&&>(__x);
              });
          }

        template <typename _Tp, typename... _As>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_masked_assign(const _MaskMember __bits, _SimdTuple<_Tp, _As...>& __lhs,
                           const __type_identity_t<_SimdTuple<_Tp, _As...>>& __rhs)
          {
            __lhs._M_forall(__rhs, [&](auto __meta, auto& __native_lhs, auto __native_rhs)
                 _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                   __meta._S_masked_assign(__meta._S_make_mask(__bits), __native_lhs, __native_rhs);
                 });
          }

        // Optimization for the case where the RHS is a scalar. No need to broadcast the scalar to a
        // simd first.
        template <typename _Tp, typename... _As>
          _GLIBCXX_SIMD_INTRINSIC static constexpr void
          _S_masked_assign(const _MaskMember __bits, _SimdTuple<_Tp, _As...>& __lhs,
                           const __type_identity_t<_Tp> __rhs)
          {
            __lhs._M_forall([&](auto __meta, auto& __native_lhs)
                 _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                   __meta._S_masked_assign(__meta._S_make_mask(__bits), __native_lhs, __rhs);
                 });
          }

        template <typename _Op, typename _Tp, typename... _As>
          static constexpr inline void
          _S_masked_cassign(const _MaskMember __bits, _SimdTuple<_Tp, _As...>& __lhs,
                            const _SimdTuple<_Tp, _As...>& __rhs, _Op __op)
          {
            __lhs._M_forall(__rhs, [&](auto __meta, auto& __native_lhs, auto __native_rhs)
                 _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                   __meta._S_masked_cassign(__meta._S_make_mask(__bits),
                                            __native_lhs, __native_rhs, __op);
                 });
          }

        // Optimization for the case where the RHS is a scalar. No need to broadcast
        // the scalar to a simd first.
        template <typename _Op, typename _Tp, typename... _As>
          static constexpr inline void
          _S_masked_cassign(const _MaskMember __bits, _SimdTuple<_Tp, _As...>& __lhs,
                            const _Tp& __rhs, _Op __op)
          {
            __lhs._M_forall([&](auto __meta, auto& __native_lhs)
                 _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                   __meta._S_masked_cassign(__meta._S_make_mask(__bits), __native_lhs, __rhs, __op);
                 });
          }

        template <template <typename> class _Op, typename _Tp, typename... _As>
          static constexpr inline _SimdTuple<_Tp, _As...>
          _S_masked_unary(const _MaskMember __bits, _SimdTuple<_Tp, _As...> __v)
          {
            return __v._M_forall([&__bits](auto __meta, auto& __native)
                   _GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA {
                     __native = __meta.template _S_masked_unary<_Op>(__meta._S_make_mask(__bits),
                                                                     __native);
                   });
          }
      };

    template <_SimdSizeType _Np, typename _Tag, typename>
      struct _MaskImplAbiCombine
      {
        static_assert(
          sizeof(uint64_t) * __CHAR_BIT__ >= _Np,
          "The fixed_size implementation relies on one uint64_t being able to store "
          "all boolean elements."); // required in load & store

        using _Abi = _AbiCombine<_Np, _Tag>;

        using _MaskMember = __pv2::_SanitizedBitMask<_Np>;

        template <typename _Tp>
          using _FirstAbi = typename __fixed_size_storage_t<_Tp, _Np>::_FirstAbi;

        template <typename _Tp>
          using _TypeTag = _Tp*;

        template <typename>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
          _S_broadcast(bool __x)
          { return __x ? ~_MaskMember() : _MaskMember(); }

        template <typename _Tp, typename _Fp>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
          _S_generator(_Fp&& __gen)
          {
            using _Up = unsigned long long;
            static_assert(_Np <= sizeof(_Up) * __CHAR_BIT__);
            return [&]<size_t... _Is>(std::index_sequence<_Is...>) {
              return ((_Up(__gen(__ic<_Is>)) << _Is) | ...);
            }(std::make_index_sequence<_Np>());
          }

        template <typename>
          _GLIBCXX_SIMD_INTRINSIC static constexpr _MaskMember
          _S_load(const bool* __mem)
          {
            using _Ip = __pv2::__int_for_sizeof_t<bool>;
            // the following load uses element_aligned and relies on __mem already
            // carrying alignment information from when this load function was
            // called.
            const std::basic_simd<_Ip, _Abi> __bools(
                    reinterpret_cast<const __pv2::__may_alias<_Ip>*>(__mem));
            return __data(__bools != 0);
          }

        template <bool _Sanitized>
          _GLIBCXX_SIMD_INTRINSIC static constexpr __pv2::_SanitizedBitMask<_Np>
          _S_to_bits(__pv2::_BitMask<_Np, _Sanitized> __x)
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
                                                            __ic<__meta._S_size()>));
                     });
          return __r;
        }

        static constexpr _MaskMember
        _S_masked_load(_MaskMember __merge, _MaskMember __mask, const bool* __mem) noexcept
        {
          __pv2::_BitOps::_S_bit_iteration(__mask.to_ullong(),
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
          __pv2::_BitOps::_S_bit_iteration(
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

    // try all native ABIs (including scalar) first
    template <typename _Tp, _SimdSizeType _Np>
      requires (__pv2::_AllNativeAbis::template _S_has_valid_abi<_Tp, _Np>)
      struct _DeduceAbi<_Tp, _Np>
      { using type = __pv2::_AllNativeAbis::_FirstValidAbi<_Tp, _Np>; };

    // next try _AbiArray of _NativeAbi
    template <typename _Tp, _SimdSizeType _Np>
      requires (not __pv2::_AllNativeAbis::template _S_has_valid_abi<_Tp, _Np>
                  and _Np % _NativeAbi<_Tp>::template _S_size<_Tp> == 0)
      struct _DeduceAbi<_Tp, _Np>
      { using type = _AbiArray<_NativeAbi<_Tp>, _Np / _NativeAbi<_Tp>::template _S_size<_Tp>>; };

    // fall back to _AbiCombine of inhomogenous ABI tags
    template <typename _Tp, _SimdSizeType _Np>
      requires (not __pv2::_AllNativeAbis::template _S_has_valid_abi<_Tp, _Np>
                  and _Np % _NativeAbi<_Tp>::template _S_size<_Tp> != 0
                  and _AbiCombine<_Np, _NativeAbi<_Tp>>::template _S_is_valid_v<_Tp>)
      struct _DeduceAbi<_Tp, _Np>
      { using type = _AbiCombine<_Np, _NativeAbi<_Tp>>; };
  }
}

#endif  // PROTOTYPE_SIMD_ABI_H_

// vim: et
