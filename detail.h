/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_DETAIL_H_
#define PROTOTYPE_DETAIL_H_

#include <ranges>
#include <concepts>

namespace detail
{
  template <std::size_t Bytes>
    struct make_unsigned;

  template <>
    struct make_unsigned<sizeof(unsigned int)>
    { using type = unsigned int; };

  template <>
    struct make_unsigned<sizeof(unsigned long)
                           + (sizeof(unsigned long) == sizeof(unsigned int))>
    { using type = unsigned long; };

  template <>
    struct make_unsigned<sizeof(unsigned long long)
                           + (sizeof(unsigned long long) == sizeof(unsigned long))>
    { using type = unsigned long long; };

  template <>
    struct make_unsigned<sizeof(unsigned short)>
    { using type = unsigned short; };

  template <>
    struct make_unsigned<sizeof(unsigned char)>
    { using type = unsigned char; };

  template <typename T>
    using make_unsigned_t = typename make_unsigned<sizeof(T)>::type;
  template <std::ranges::sized_range T>
    constexpr inline std::size_t
    static_range_size = []() {
      if constexpr (requires { {T::size()} -> std::integral; })
        return T::size();
      else if constexpr (requires { {T::extent} -> std::integral; })
        return T::extent;
      else if constexpr (requires { {std::tuple_size_v<T>} -> std::integral; })
        return std::tuple_size_v<T>;
      else
        return std::dynamic_extent;
    }();

  template <auto Value>
    using cnst_t = std::integral_constant<decltype(Value), Value>;

  template <auto Value>
    inline constexpr cnst_t<Value> cnst{};
}

#endif  // PROTOTYPE_DETAIL_H_
