/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_MASK2_H_
#define PROTOTYPE_SIMD_MASK2_H_

#include "detail.h"
#include "stdx.h"

#include <concepts>
#include <climits>

template <typename SimdType>
  class simd_mask2 : public SimdType::mask_type
  {
    using T = typename SimdType::value_type;
    using base = typename SimdType::mask_type;

  public:
    using simd_type = SimdType;

    using base::simd_mask;

    constexpr simd_type
    operator-() const
    {
      if constexpr (sizeof(simd_mask2) == sizeof(simd_type)
        {
          if constexpr (std::integral<T>)
            return std::bit_cast<simd_type>(*this);

          using U = std::experimental::rebind_simd_t<detail::make_unsigned_t<T>, simd_type>;
          const auto bits = std::bit_cast<U>(*this);
          return std::bit_cast<simd_type>(bits & std::bit_cast<U>(simd_type(T(-1))));
        }
      else
        {
          simd_type r = {};
          where(*this, r) = T(-1);
          return r;
        }
    }

    constexpr simd_type
    operator+() const
    { return operator simd_type(); }

    constexpr
    operator simd_type() const
    {
      if constexpr (sizeof(simd_mask2) == sizeof(simd_type))
        {
          using U = std::experimental::rebind_simd_t<detail::make_unsigned_t<T>, simd_type>;
          const auto bits = std::bit_cast<U>(*this);
          if constexpr (std::integral<T>)
            return std::bit_cast<simd_type>(bits >> (sizeof(T) * CHAR_BIT - 1));
          else
            return std::bit_cast<simd_type>(bits & std::bit_cast<U>(simd_type(T(1))));
        }
      else
        {
          simd_type r = {};
          where(*this, r) = 1;
          return r;
        }
    }
  };

#endif  // PROTOTYPE_SIMD_MASK2_H_
