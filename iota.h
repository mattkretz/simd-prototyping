/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright © 2023-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_IOTA_H_
#define PROTOTYPE_IOTA_H_

#include "simd.h"
#include <array>

#if not SIMD_IS_A_RANGE
#error "iota_v is implemented such that it requires simd to be a range"
#endif

namespace std
{
  namespace __detail
  {
    template <typename _Tp, auto _First, auto _Step, typename>
      struct __iota_array;

    template <typename _Tp, auto _First, auto _Step, std::size_t... _Is>
      struct __iota_array<_Tp, _First, _Step, std::index_sequence<_Is...>>
      {
        static constexpr _Tp __data[sizeof...(_Is)] = {
          static_cast<_Tp>(_First + _Step * static_cast<_Tp>(_Is))...
        };
      };

    template <typename _Rg, auto _First, auto _Step, std::size_t... _Is>
      concept __init_list_constructible = requires {
        _Rg{static_cast<std::ranges::range_value_t<_Rg>>(
              _First + _Step * static_cast<std::ranges::range_value_t<_Rg>>(_Is))...};
      };

    template <typename _Rg, auto _First, auto _Step, std::size_t... _Is>
      requires __init_list_constructible<_Rg, _First, _Step, _Is...>
      constexpr _Rg
      __iota_init_range(std::index_sequence<_Is...>)
      {
        using _Tp = std::ranges::range_value_t<_Rg>;
        return _Rg{static_cast<_Tp>(_First + _Step * static_cast<_Tp>(_Is))...};
      }

    template <typename _Rg, auto _First, auto _Step, std::size_t... _Is>
      requires (not __init_list_constructible<_Rg, _First, _Step, _Is...>)
      and requires {
        _Rg(__iota_array<std::ranges::range_value_t<_Rg>, _First, _Step,
                         std::index_sequence<_Is...>>::__data);
      }
      constexpr _Rg
      __iota_init_range(std::index_sequence<_Is...>)
      {
        return _Rg(__iota_array<std::ranges::range_value_t<_Rg>, _First, _Step,
                                std::index_sequence<_Is...>>::__data);
      }

    template <typename _Rg, auto _First, auto _Step>
      constexpr _Rg
      __iota_init()
      {
        if constexpr (__static_range_size<_Rg> != std::dynamic_extent)
          return __iota_init_range<_Rg, _First, _Step>(std::make_index_sequence<__static_range_size<_Rg>>());
        else
          return _Rg{_First};
      }

    template <typename _Tp, auto _First, auto _Step>
      concept __iota_constructible
        = (std::is_array_v<_Tp> and std::constructible_from<std::remove_extent_t<_Tp>, size_t>)
            or (__static_range_size<_Tp> != std::dynamic_extent
                  and requires { __iota_init_range<_Tp, _First, _Step>(
                                   std::make_index_sequence<__static_range_size<_Tp>>()); })
            or (std::regular<_Tp> and requires { _Tp{_First}; } and (_Tp {_First} == _First));
  }

  template <typename _Tp, auto _First = 0, auto _Step = 1>
    requires __detail::__iota_constructible<_Tp, _First, _Step>
    inline constexpr _Tp
    iota_v = __detail::__iota_init<_Tp, _First, _Step>();

  template <typename _Tp, std::size_t _Np, auto _First, auto _Step>
    requires std::constructible_from<_Tp, size_t>
    inline constexpr auto&
    iota_v<_Tp[_Np], _First, _Step>
      = __detail::__iota_array<_Tp, _First, _Step, decltype(std::make_index_sequence<_Np>())>::__data;
}

#endif  // PROTOTYPE_IOTA_H_
