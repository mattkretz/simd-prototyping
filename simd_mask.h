/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_MASK2_H_
#define PROTOTYPE_SIMD_MASK2_H_

#include "detail.h"
#include "simd_abi.h"

#include <concepts>
#include <climits>

namespace std
{
  template <typename _Tp, typename _Abi>
    class simd_mask : public __detail::simd_mask<_Tp, _Abi>
    {
      using _Base = std::experimental::simd_mask<_Tp, _Abi>;

    public:
      using value_type = bool;
      using simd_type = std::simd<_Tp, _Abi>;

      using _Base::simd_mask;

      constexpr simd_type
      operator-() const
      {
        if constexpr (sizeof(simd_mask) == sizeof(simd_type))
          {
            if constexpr (std::integral<_Tp>)
              return std::bit_cast<simd_type>(*this);

            using U = std::experimental::rebind_simd_t<__detail::__make_unsigned_t<_Tp>, simd_type>;
            const auto bits = std::bit_cast<U>(*this);
            return std::bit_cast<simd_type>(bits & std::bit_cast<U>(simd_type(_Tp(-1))));
          }
        else
          {
            simd_type r = {};
            where(*this, r) = _Tp(-1);
            return r;
          }
      }

      constexpr simd_type
      operator+() const
      { return operator simd_type(); }

      constexpr
      operator simd_type() const
      {
        if constexpr (sizeof(simd_mask) == sizeof(simd_type))
          {
            using U = std::experimental::rebind_simd_t<__detail::__make_unsigned_t<_Tp>, simd_type>;
            const auto bits = std::bit_cast<U>(*this);
            if constexpr (std::integral<_Tp>)
              return std::bit_cast<simd_type>(bits >> (sizeof(_Tp) * CHAR_BIT - 1));
            else
              return std::bit_cast<simd_type>(bits & std::bit_cast<U>(simd_type(_Tp(1))));
          }
        else
          {
            simd_type r = {};
            where(*this, r) = 1;
            return r;
          }
      }
    };

  template <typename _Tp, typename _Abi>
    struct is_simd_mask<simd_mask<_Tp, _Abi>> : is_default_constructible<simd_mask<_Tp, _Abi>>
    {};
}

#endif  // PROTOTYPE_SIMD_MASK2_H_
