/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_CONVERTER_H_
#define PROTOTYPE_SIMD_CONVERTER_H_

#include "detail.h"

namespace std::__detail
{
  template <__vectorizable _Tp, typename _TAbi, __vectorizable _Up, typename _UAbi>
    struct _SimdConverter;

  template <__vectorizable _Tp, __simd_abi_tag _TAbi>
    struct _SimdConverter<_Tp, _TAbi, _Tp, _TAbi>
    {
      using _TV = typename _TAbi::template _SimdMember<_Tp>;

      _GLIBCXX_SIMD_INTRINSIC constexpr _TV
      operator()(_TV __x)
      { return __x; }
    };

  template <__vectorizable _From, __simd_abi_tag _FromAbi, __vectorizable _To, __simd_abi_tag _ToAbi>
    struct _SimdConverter<_From, _FromAbi, _To, _ToAbi>
    {
      using _Impl = typename _ToAbi::_SimdImpl;
      using _FromImpl = typename _FromAbi::_SimdImpl;

      _GLIBCXX_SIMD_INTRINSIC constexpr auto
      operator()(typename _FromAbi::template _SimdMember<_From> const& __x)
      {
        constexpr auto _NFrom = _FromAbi::_S_size;
        constexpr auto _NTo = _ToAbi::_S_size;
        static_assert(_NFrom >= _NTo);
        return _Impl::template _S_generator<_To>([&] [[__gnu__::__always_inline__]] (auto __i) {
                 return _FromImpl::_S_get(__x, __i);
               });
      }
    };
}

#endif // PROTOTYPE_SIMD_CONVERTER_H_
