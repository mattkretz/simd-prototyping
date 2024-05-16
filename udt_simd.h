/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef UDT_SIMD_H_
#define UDT_SIMD_H_

#include <bit>
#include <concepts>
#include "simd_meta.h"

namespace std
{
  template <typename T>
    requires requires { typename T::representation_type; }
    struct representation_type<T>
    { using type = typename T::representation_type; };

  namespace __representation_impl
  {
    template <typename>
      void from_representation(auto&&, auto&&) = delete;

    template <typename T>
      requires requires { typename representation_type<T>::type; }
      struct __from_rep
      {
        // 1. ADL
        // 2. member
        // 3. bit_cast
        constexpr T
        operator()(representation_type_t<T> x) const
        {
          T* tag = nullptr;
          if constexpr (requires (representation_type_t<T> x) {
            { from_representation(tag, x) } -> std::same_as<T>;
          })
            return from_representation(tag, x);
          else if constexpr (requires (representation_type_t<T> x) {
            { T::from_representation(x) } -> std::same_as<T>;
          })
            return T::from_representation(x);
          else
            return std::bit_cast<T>(x);
        }
      };

    void to_representation(auto&&) = delete;

    struct __to_rep
    {
      // 1. ADL
      // 2. member
      // 3. bit_cast
      template <typename T>
        constexpr typename representation_type<T>::type
        operator()(const T& x) const
        {
          if constexpr (requires (const T& x) {
            { to_representation(x) } -> std::same_as<representation_type_t<T>>;
          })
            return to_representation(x);
          else if constexpr (requires (const T& x) {
            { x.to_representation() } -> std::same_as<representation_type_t<T>>;
          })
            return x.to_representation();
          else
            return std::bit_cast<representation_type_t<T>>(x);
        }
    };

    void construct_representation_from(auto&&, auto&&) = delete;

    template <typename _Tp>
      struct __construct_representation
      {
        static_assert(not __detail::__simd_type<_Tp>);

        using _Rp = std::representation_type_t<_Tp>;

        template <typename _Up>
          constexpr auto
          operator()(const _Up& __x) const
          {
            _Tp* __tag = nullptr;
            if constexpr (std::constructible_from<_Tp, const _Up&>)
              return __to_rep()(_Tp(__x));
            else if constexpr (requires { construct_representation_from(__tag, __x); })
              return construct_representation_from(__tag, __x);
            else if constexpr (requires { _Tp::construct_representation_from(__x); })
              return _Tp::construct_representation_from(__x);
            else if constexpr (__detail::__simd_type<_Up>)
              return std::rebind_simd_t<_Rp, _Up>([&](int __i) {
                       return __to_rep()(_Tp(__x[__i]));
                     });
          }
      };
  }

  template <typename _Tp>
    inline constexpr __representation_impl::__from_rep<_Tp> from_representation = {};

  inline constexpr __representation_impl::__to_rep to_representation = {};

  template <typename _Tp>
    inline constexpr __representation_impl::__construct_representation<_Tp>
    construct_representation_from = {};
}

namespace std::__detail
{
  // do we want to require std::regular<_Tp>? Default constructible might be too restrictive.
  template <typename _Tp>
    concept __indirectly_vectorizable
      = requires { typename std::representation_type<_Tp>::type; }
          and __vectorizable<std::representation_type_t<_Tp>>
          and requires(const std::representation_type_t<_Tp>& __r, const _Tp& __t)
      {
        { std::from_representation<_Tp>(__r) } -> std::same_as<_Tp>;
        { std::to_representation(__t) } -> std::same_as<std::representation_type_t<_Tp>>;
      };

  template <typename _Tp>
    concept __any_vectorizable = __vectorizable<_Tp> or __indirectly_vectorizable<_Tp>;
}

#endif  // UDT_SIMD_H_
